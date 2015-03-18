#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include "producer.h"
#include "consumer.h"
#include "common.h"

void initialize_counts(int semkey);
void cleanup(int status);

//Max number of "charms" in "bin"
const int BUF_SIZE = 3;
//Array indices of the three semaphores to be used
enum SEMAPHORES {MUTEX = 0, EMPTY, FULL, NUM_SEM}; 

// Keys for Shared memory and semaphores
int shmid = -1;
int semkey = -1;
pid_t producer_id = -1;
pid_t consumer_id = -1;

int main() {

    // Create shared memory segment
    // With size shmsize = "bin size" * "charm size"
    // Create if necessary (IPC_CREAT) with r/w permissions (0666)
    size_t shmsize = BUF_SIZE*sizeof(struct charm);
    shmid = shmget(IPC_PRIVATE, shmsize, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("Error getting shared memory");
        cleanup(EXIT_FAILURE);
    }

    // Get key to private semaphore group (IPC_PRIVATE)
    // With NUM_SEM semaphores in it
    // Create if necessary (IPC_CREAT) with r/w permissions (0666)
    semkey = semget(IPC_PRIVATE, NUM_SEM, IPC_CREAT | 0666);
    if (semkey < 0) {
        perror("Error getting semaphores");
        cleanup(EXIT_FAILURE);
    }
    initialize_counts(semkey); //Initializes semaphores. See below.

    // Setup data to be shared between parent (this) process and
    // child (producer and consumer) processes.
    struct shared_data_info shared = {
        BUF_SIZE, 
        shmid,
        semkey,
        MUTEX,
        EMPTY,
        FULL
    };

    // Fork producer
    producer_id = fork();
    if (producer_id < 0) {
        perror("Error forking producer");
        cleanup(EXIT_FAILURE);
    } else if (!producer_id) {
	// Instead of writing child process logic here,
	// it is much cleaner to put it in its own function.
	// If that function is sufficiently complex, it should
	// have its own source file (like in this case).
        producer(shared);
    }

    // Fork consumer
    consumer_id = fork();
    if (consumer_id < 0) {
        perror("Error forking consumer");
        cleanup(EXIT_FAILURE);
    } else if (!consumer_id) {
	// Ditto above "producer(shared)" comment.
        consumer(shared);
    }

    // Wait for children
    int status1, status2;
    if (wait(&status1) < 0) {
	perror("wait(&status1)");
	cleanup(EXIT_FAILURE);
    }
    if (wait(&status2) < 0) {
	perror("wait(&status2)");
	cleanup(EXIT_FAILURE);
    }
    int status = WEXITSTATUS(status1) || WEXITSTATUS(status2);

    // Mark children as finished
    consumer_id = -1;
    producer_id = -1;

    cleanup(status);

    // Should never reach this statement, but...
    return status;
}

// System V semaphore groups are located sequentially in memory.
// Therefore each group is accessed via an array (hence defining
// the above named indices (via the enum). Even if only one semaphore
// is needed, it wil be accessed via a single item array.
void initialize_counts(int semkey) {
    // Create union structure for counts
    union semun sem_union;
    unsigned short counters[3];
    //This initializes the semaphores counts.
    counters[MUTEX] = 1;
    counters[EMPTY] = BUF_SIZE;
    counters[FULL ] = 0;
    sem_union.array = counters;

    // Call semctl to set all counts
    // (second argument is ignored)
    int semset = semctl(semkey, 0, SETALL, sem_union);
    if (semset < 0) {
        perror("Error setting semaphore counts");
        exit(EXIT_FAILURE);
    }
}

void cleanup(int status) {
    // Kill children if they're running
    if (producer_id > 0) {
        if(kill(producer_id, SIGKILL) < 0) {
	    perror("kill(producer_id, SIGKILL)");
	    exit(EXIT_FAILURE);
	} 
        wait(NULL);
    }
    if (consumer_id > 0) {
        if(kill(consumer_id, SIGKILL) < 0) {
	    perror("kill(consumer_id, SIGKILL)");
	    exit(EXIT_FAILURE);
	}
        wait(NULL);
    }

    // For semaphore group with semkey
    // (Second argument ignored)
    // remove it (IPC_RMID)
    if (semkey >= 0) {
        if (semctl(semkey, 0, IPC_RMID) < 0) {
            perror("Error removing semaphores");
            status = EXIT_FAILURE;
        }
    }

    // For shared memory segment with id shmid
    // remove it (IPC_RMID)
    // (last argument ignored, use NULL pointer)
    if (shmid >= 0) {
        if (shmctl(shmid, IPC_RMID, (struct shmid_ds *) NULL)) {
            perror("Error removing shared memory");
            status = EXIT_FAILURE;
        }
    }

    exit(status);
}
