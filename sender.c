#include "sender.h"

void send(message_t message, mailbox_t* mailbox_ptr)
{
    if(mailbox_ptr->flag == MESSAGE_PASSING)
    {
        if(mq_send(mailbox_ptr->storage.mqd, (char*)&message, sizeof(message_t), 0) == -1)
        {
            perror("mq_send");
            exit(1);
        }
    }
    else if(mailbox_ptr->flag == SHARED_MEMORY)
    {
        memcpy(mailbox_ptr->storage.shm_addr, &message, sizeof(message_t));
    }
    
}

int main(int argc, char* argv[])
{
    int method = atoi(argv[1]);
    FILE* fp = fopen(argv[2], "r");

    // create semaphore
    sem_t* sem_sender = sem_open("sem_sender", O_CREAT, 0644, 1);
    sem_t* sem_receiver = sem_open("sem_receiver", O_CREAT, 0644, 0);
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
        struct mq_attr attr;
        attr.mq_maxmsg = 10;
        attr.mq_msgsize = sizeof(message_t);
        mailbox.storage.mqd = mq_open("/mailbox", O_WRONLY | O_CREAT, 0644, &attr);
        if(mailbox.storage.mqd == (mqd_t) -1)
        {
            perror("mq_open");
            exit(1);
        }
    }
    else if(mailbox.flag == SHARED_MEMORY)
    {
        printf("Shared memory\n");
        int shm_fd = shm_open("/mailbox", O_RDWR | O_CREAT, 0644);
        if(shm_fd == -1)
        {
            perror("shm_open");
            exit(1);
        }
        ftruncate(shm_fd, sizeof(message_t));
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

    while(fgets(message.message, MAX_MESSAGE_SIZE, fp) != NULL){
        message.size = strlen(message.message);

        // send message
        sem_wait(sem_sender);
        printf("Sending message: %s", message.message);
        clock_gettime(CLOCK_MONOTONIC, &start);
        send(message, &mailbox);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sem_post(sem_receiver);
        time_taken += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    }

    // send termination message
    message.flag = 1;
    sem_wait(sem_sender);
    send(message, &mailbox);
    sem_post(sem_receiver);
    
    // print terminate message
    printf("\nEOF reached\n");
    printf("Total time taken in sending msg: %f\n", time_taken);

    // close semaphore
    sem_close(sem_sender);
    sem_close(sem_receiver);
    sem_unlink("sem_sender");
    sem_unlink("sem_receiver");

    // close mailbox
    if(mailbox.flag == MESSAGE_PASSING)
    {
        mq_close(mailbox.storage.mqd);
        mq_unlink("/mailbox");
    }
    else if(mailbox.flag == SHARED_MEMORY)
    {
        munmap(mailbox.storage.shm_addr, sizeof(message_t));
        shm_unlink("/mailbox");
    }
    return 0;
}