#ifndef SEMAPHORE_COMMON
#define SEMAPHORE_COMMON

/*
 * Create 'semun' structure
 * (copied from semctl man page)
 */
union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};

// Wait and signal operations
// given to 'semop'
#define WAIT -1
#define SIGNAL 1

// Holds information about shared data
// needed by producer and consumer
struct shared_data_info {
    int buf_size; // Size of shared buffer (in charms)
    int shmid;    // ID for shared memory
    int semkey;   // ID for semaphore group
    int mutex;    // Index for 'mutex' semaphore
    int empty;    // Index for 'empty' semaphore
    int full;     // Index for 'full' semaphore
};

// The data we'll be passing
// between processes
// (and part of a
//  balanced breakfast)
struct charm {
    enum COLOR
    {
        PINK = 0,
        YELLOW,
        ORANGE,
        GREEN,
        BLUE,
        PURPLE,
        RED
    } color;
    char shape[32];
};

#define CHARMS 7
extern struct charm LUCKY_CHARMS[];

void print_charm(struct charm *);
#endif
