#include <string.h>
#include <errno.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include "cbuf.c"
#include "const.h"
#include "randString.h"

void producer(cbuf_handle_t cbuf, sem_t *empty, sem_t *mutex, sem_t *full, char *name, int key)
{
    time_t rawTime;
    struct tm *timeInfo;

    for (int k = 0; k < PRODUCER_CYCLES; k++)
    {
        int id = shmget(key + k, 4 * sizeof(char), IPC_CREAT | 0666);
        char *s = (char *)shmat(id, NULL, 0);
        char *msg = rand_string(3, s);
        time(&rawTime);
        timeInfo = localtime(&rawTime);
        printf("[%02d:%02d:%02d][..][PRODUCER %s] New item: [%s] wait for buf [%s]\n", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec, name, msg, name);
        if (sem_wait(empty) == -1)
            perror("A sem_wait: empty"); // Semaphore down operation
        if (sem_wait(mutex) == -1)
            perror("A sem_wait: mutex");
        if (circular_buf_put2(cbuf, id) == -1)
            perror("Cbuf put");
        time(&rawTime);
        timeInfo = localtime(&rawTime);
        printf("[%02d:%02d:%02d][!!][PRODUCER %s] Put item [%s]. [Buffer %s] After input: ", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec, name, msg, name);
        circular_buf_print(cbuf);
        if (sem_post(mutex) == -1)
            perror("sem_post: mutex");
        if (sem_post(full) == -1)
            perror("sem_post: full"); // Semaphore up operation
        sleep(PRODUCER_SLEEP_SEC);
    }
    return;
}
