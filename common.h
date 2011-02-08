
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
