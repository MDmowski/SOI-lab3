#include <string.h>
#include <errno.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>
#include "cbuf.c"
#include "const.h"
#include "randString.h"

void specialProducer(cbuf_handle_t Acbuf, sem_t *Aempty, sem_t *Amutex, sem_t *Afull, cbuf_handle_t Bcbuf, sem_t *Bempty, sem_t *Bmutex, sem_t *Bfull, cbuf_handle_t Ccbuf, sem_t *Cempty, sem_t *Cmutex, sem_t *Cfull, int key)
{
    time_t rawTime;
    struct tm *timeInfo;
    cbuf_handle_t cbuf;
    sem_t *empty, *mutex, *full;
    for (int k = 0; k < SPECIAL_PRODUCER_CYCLES; k++)
    {
        char letter = 65 + rand() % 3;
        if (letter == 'A')
        {
            cbuf = Acbuf;
            empty = Aempty;
            mutex = Amutex;
            full = Afull;
        }
        else if (letter == 'B')
        {
            cbuf = Bcbuf;
            empty = Bempty;
            mutex = Bmutex;
            full = Bfull;
        }
        else if (letter == 'C')
        {
            cbuf = Ccbuf;
            empty = Cempty;
            mutex = Cmutex;
            full = Cfull;
        }

        int id = shmget(key + k, 4 * sizeof(char), IPC_CREAT | 0666);
        char *s = (char *)shmat(id, NULL, 0);
        char *msg = rand_string(3, s);

        time(&rawTime);
        timeInfo = localtime(&rawTime);
        printf("[%02d:%02d:%02d][..][S_PRODUCER] New item: [%s] waiting for buf [%c]\n", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec, msg, letter);
        if (sem_wait(empty) == -1)
            perror("A sem_wait: empty"); // Semaphore down operation
        if (sem_wait(mutex) == -1)
            perror("A sem_wait: mutex");
        if (circular_buf_put_tail(cbuf, id) == -1)
            perror("Cbuf put");
        time(&rawTime);
        timeInfo = localtime(&rawTime);
        printf("[%02d:%02d:%02d][!!][S_PRODUCER] Put item [%s]. [Buffer %c] After input: ", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec, msg, letter);
        circular_buf_print(cbuf);
        if (sem_post(mutex) == -1)
            perror("sem_post: mutex");
        if (sem_post(full) == -1)
            perror("sem_post: full"); // Semaphore up operation

        sleep(SPECIAL_PRODUCER_SLEEP_SEC);
    }
    return;
}
