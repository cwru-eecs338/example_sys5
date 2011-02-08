#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>

#include "common.h"

void producer(struct shared_data shared) {
    // Define useful semaphore variables
    struct sembuf wait_mutex = {shared.mutex, WAIT, 0};
    struct sembuf signal_mutex = {shared.mutex, SIGNAL, 0};
    struct sembuf wait_empty = {shared.empty, WAIT, 0};
    struct sembuf signal_full = {shared.full, SIGNAL, 0};

    // Attach to shared memory
    // As a trick, since 'shmat' returns
    // a pointer to the data, we can treat
    // it however we like, so let's pretend
    // it's an array of lucky charms...
    struct charm *charm_buf = shmat(shared.shmid, (void *) 0, 0);

    int nextp = 0;

    // Produce the charms
    int c;
    for(c = 0; c < CHARMS; c++) {

        // wait(empty)
        if(semop(shared.semkey, &wait_empty, 1) < 0) {
            perror("wait(empty)");
            _exit(EXIT_FAILURE);
        }
        
        // wait(mutex)
        if(semop(shared.semkey, &wait_mutex, 1) < 0) {
            perror("wait(mutex)");
            _exit(EXIT_FAILURE);
        }

        /********************CRITICAL SECTION BEGIN********************/
        
        struct charm next_charm = LUCKY_CHARMS[c];
        printf("Producing: ");
        print_charm(&next_charm);
        printf("\n");
        charm_buf[nextp] = next_charm; // Shared memory access
        nextp = (nextp + 1) % shared.buf_size;

        /*********************CRITICAL SECTION END*********************/

        // signal(mutex)
        if(semop(shared.semkey, &signal_mutex, 1) < 0) {
            perror("signal(mutex)");
            _exit(EXIT_FAILURE);
        }

        // signal(full)
        if(semop(shared.semkey, &signal_full, 1) < 0) {
            perror("signal(full)");
            _exit(EXIT_FAILURE);
        }

    }

    _exit(EXIT_SUCCESS);
}
