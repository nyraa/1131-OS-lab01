#include "receiver.h"

void receive(message_t* message_ptr, mailbox_t* mailbox_ptr)
{
    if(mailbox_ptr->flag == MESSAGE_PASSING)
    {
        if(mq_receive(mailbox_ptr->storage.mqd, (char*)message_ptr, sizeof(message_t), NULL) == -1)
        {
            perror("mq_receive");
            exit(1);
        }
    }
    else if(mailbox_ptr->flag == SHARED_MEMORY)
    {
        memcpy(message_ptr, mailbox_ptr->storage.shm_addr, sizeof(message_t));
    }
}

int main(int argc, char* argv[])
{
    int method = atoi(argv[1]);

    // create semaphore
    sem_t* sem_sender = sem_open("sem_sender", O_RDWR);
    sem_t* sem_receiver = sem_open("sem_receiver", O_RDWR);
    if (sem_sender == SEM_FAILED || sem_receiver == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }

    // create mailbox
    message_t message;
    mailbox_t mailbox;
    mailbox.flag = method;

    if(mailbox.flag == MESSAGE_PASSING)
    {
        printf("Message passing\n");
        mailbox.storage.mqd = mq_open("/mailbox", O_RDONLY);
        if(mailbox.storage.mqd == (mqd_t) -1)
        {
            perror("mq_open");
            exit(1);
        }
    }
    else if(mailbox.flag == SHARED_MEMORY)
    {
        printf("Shared memory\n");
        int shm_fd = shm_open("/mailbox", O_RDWR, 0644);
        if(shm_fd == -1)
        {
            perror("shm_open");
            exit(1);
        }
        mailbox.storage.shm_addr = mmap(NULL, sizeof(message_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if(mailbox.storage.shm_addr == MAP_FAILED)
        {
            perror("mmap");
            exit(1);
        }
        close(shm_fd);
    }

    struct timespec start, end;
    double time_taken = 0;

    while(1)
    {
        sem_wait(sem_receiver);
        clock_gettime(CLOCK_MONOTONIC, &start);
        receive(&message, &mailbox);
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_taken += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
        sem_post(sem_sender);
        if(message.flag == 1)
        {
            printf("\nTermination message received\n");
            printf("Total time taken in receiving messages: %f\n", time_taken);
            break;
        }
        printf("Message received: %s", message.message);
    }

    // close mailbox
    if(mailbox.flag == MESSAGE_PASSING)
    {
        mq_close(mailbox.storage.mqd);
    }
    else if(mailbox.flag == SHARED_MEMORY)
    {
        munmap(mailbox.storage.shm_addr, sizeof(message_t));
    }

    sem_close(sem_sender);
    sem_close(sem_receiver);
    return 0;
}