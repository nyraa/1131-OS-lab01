#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include "struct.h"


void receive(message_t* message_ptr, mailbox_t* mailbox_ptr);