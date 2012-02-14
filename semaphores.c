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

const int BUF_SIZE = 3;
enum SEMAPHORES {MUTEX = 0, EMPTY, FULL, NUM_SEM};

// Keys for Shared memory and semaphores
int shmid = -1;
int semkey = -1;
pid_t producer_id = -1;
pid_t consumer_id = -1;

int main() {

    // Create shared memory segment
    // With size shmsize
    // Create if necessary (IPC_CREAT) with r/w permissions (0666)
    size_t shmsize = BUF_SIZE*sizeof(struct charm);
    shmid = shmget(IPC_PRIVATE, shmsize, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("Error getting semaphores");
        cleanup(EXIT_FAILURE);
    }

    // Get private semaphore group (IPC_PRIVATE)
    // With NUM_SEM semaphores in it
    // Create if necessary (IPC_CREAT) with r/w permissions (0666)
    semkey = semget(IPC_PRIVATE, NUM_SEM, IPC_CREAT | 0666);
    if (semkey < 0) {
        perror("Error getting semaphores");
        cleanup(EXIT_FAILURE);
    }
    initialize_counts(semkey);

    // Setup shared data
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
        producer(shared);
    }

    // Fork consumer
    consumer_id = fork();
    if (consumer_id < 0) {
        perror("Error forking consumer");
        cleanup(EXIT_FAILURE);
    } else if (!consumer_id) {
        consumer(shared);
    }

    // Wait for children
    // TODO: Error checking
    int status1, status2;
    wait(&status1);
    wait(&status2);
    int status = WEXITSTATUS(status1) || WEXITSTATUS(status2);

    // Mark children as finished
    consumer_id = -1;
    producer_id = -1;

    cleanup(status);

    // Should never reach this statement, but...
    return status;
}

void initialize_counts(int semkey) {
    // Create union structure for counts
    union semun sem_union;
    unsigned short counters[3];
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
        // TODO: Error checking
        kill(producer_id, SIGKILL);
        wait(NULL);
    }
    if (consumer_id > 0) {
        // TODO: Error checking
        kill(consumer_id, SIGKILL);
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
