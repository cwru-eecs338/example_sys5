#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>

#include "common.h"

void consumer(struct shared_data shared) {
    // Define useful semaphore variables
    struct sembuf wait_mutex = {shared.mutex, WAIT, 0};
    struct sembuf signal_mutex = {shared.mutex, SIGNAL, 0};

    // Attach to shared memory
    struct charm *charm_buf = shmat(shared.shmid, (void *) 0, 0);

    if(semop(shared.semkey, &wait_mutex, 1) < 0) {
        perror("Error using \"mutex:wait\"");
        _exit(EXIT_FAILURE);
    }
    puts("Consumer in critical section.");
    if(semop(shared.semkey, &signal_mutex, 1) < 0) {
        perror("Error using \"mutex:signal\"");
        _exit(EXIT_FAILURE);
    }
    while(1){}

    _exit(EXIT_SUCCESS);
}
