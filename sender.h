#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <mqueue.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "struct.h"

void send(message_t message, mailbox_t* mailbox_ptr);