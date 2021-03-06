#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>

#include "common.h"

void producer(struct shared_data_info shared) {
    // Define useful semaphore variables
    struct sembuf wait_mutex = {shared.mutex, WAIT, 0}; //for wait(mutex)
    struct sembuf signal_mutex = {shared.mutex, SIGNAL, 0}; //for signal(mutex)
    struct sembuf wait_empty = {shared.empty, WAIT, 0}; //for wait(empty)
    struct sembuf signal_full = {shared.full, SIGNAL, 0}; //for signal(full)

    // Attach to shared memory
    // As a trick, since 'shmat' returns
    // a pointer to the data, we can treat
    // it however we like, so let's pretend
    // it's an array of lucky charms...
    // (this implicitly casts the blob of shared
    // memory to the shared data structure,
    // which is the "charm" struct)
    struct charm *charm_buf = shmat(shared.shmid, (void *) 0, 0);
    if(charm_buf < 0) {
		perror("shmat(shared.shmid, (void *) 0, 0)");
		_exit(EXIT_FAILURE);
    }

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
        
        struct charm *produce_charm = &LUCKY_CHARMS[c];
        printf("Producing: ");
        print_charm(produce_charm);
        printf("\n");
		fflush(0);
        charm_buf[nextp] = *produce_charm; // Shared memory access
        nextp = (nextp + 1) % shared.BUF_SIZE;

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

    // Detach from the shared memory
    if(shmdt(charm_buf) < 0) {
    	perror("shmdt(charm_buf)");
		_exit(EXIT_FAILURE);
    }

    _exit(EXIT_SUCCESS);
}
