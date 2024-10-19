#include <mqueue.h>
#define MAX_MESSAGE_SIZE 1024   // 1KB
#define MAX_MESSAGE_N_SIZE 1025    // 1KB + 1
#define MESSAGE_PASSING 1
#define SHARED_MEMORY 2

typedef struct {
    int flag;      // 1 for message passing, 2 for shared memory
    union{
        mqd_t mqd;
        void* shm_addr;
    }storage;
} mailbox_t;


typedef struct {
    /*  TODO: 
        Message structure for wrapper
    */
    char message[MAX_MESSAGE_N_SIZE];
    int size;
    int flag;       // 0 for normal message, 1 for termination message
} message_t;