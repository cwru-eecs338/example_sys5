#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>

#include "common.h"

void consumer(struct shared_data_info shared) {
    // Define useful semaphore variables
    struct sembuf wait_mutex = {shared.mutex, WAIT, 0}; //for wait(mutex)
    struct sembuf signal_mutex = {shared.mutex, SIGNAL, 0}; //for signal(mutex)
    struct sembuf wait_full = {shared.full, WAIT, 0}; //for wait(full)
    struct sembuf signal_empty = {shared.empty, SIGNAL, 0}; //for signal(empty)

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

    int nextc = 0;

    // Consume every delicious
    // marshmallow
    int c;
    for(c = 0; c < CHARMS; c++) {

        // wait(full)
        if(semop(shared.semkey, &wait_full, 1) < 0) {
            perror("wait(full)");
            _exit(EXIT_FAILURE);
        }
        
        // wait(mutex)
        if(semop(shared.semkey, &wait_mutex, 1) < 0) {
            perror("wait(mutex)");
            _exit(EXIT_FAILURE);
        }

        /********************CRITICAL SECTION BEGIN********************/
        
        // Shared memory access
        struct charm consume_charm = charm_buf[nextc];
        nextc = (nextc + 1) % shared.BUF_SIZE;
        printf("    Consuming: ");
        print_charm(&consume_charm);
        printf("\n");
        fflush(0);

        /*********************CRITICAL SECTION END*********************/

        // signal(mutex)
        if(semop(shared.semkey, &signal_mutex, 1) < 0) {
            perror("signal(mutex)");
            _exit(EXIT_FAILURE);
        }

        // signal(empty)
        if(semop(shared.semkey, &signal_empty, 1) < 0) {
            perror("signal(empty)");
            _exit(EXIT_FAILURE);
        }

    }

    //Detach from the shared memory.
    if(shmdt(charm_buf) < 0) {
		perror("shmdt(charm_buf");
		_exit(EXIT_FAILURE);
    }
    
    _exit(EXIT_SUCCESS);
}
