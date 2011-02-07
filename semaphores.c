#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

const int BUF_SIZE = 3;
enum SEMAPHORES {MUTEX = 0, EMPTY, FULL, NUM_SEM};

int main() {

    // Get private semaphore group (IPC_PRIVATE)
    // With NUM_SEM semaphores in it
    // Create if necessary (IPC_CREAT) with r/w permissions (0666)
    int semkey = semget(IPC_PRIVATE, NUM_SEM, IPC_CREAT | 0666);
    if (semkey < 0) {
        perror("Error getting semaphores");
        exit(EXIT_FAILURE);
    }

    // For semaphore group with semkey
    // (Second argument ignored)
    // remove it (IPC_RMID)
    if (semctl(semkey, 0, IPC_RMID) < 0) {
        perror("Error removing semaphores");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
