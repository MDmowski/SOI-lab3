#include <string.h>
#include <errno.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>
#include "cbuf.c"
#include "const.h"
#include "randString.h"

void consumer(cbuf_handle_t Acbuf, sem_t *Aempty, sem_t *Amutex, sem_t *Afull, cbuf_handle_t Bcbuf, sem_t *Bempty, sem_t *Bmutex, sem_t *Bfull, cbuf_handle_t Ccbuf, sem_t *Cempty, sem_t *Cmutex, sem_t *Cfull, char name, int key, int pr)
{
    time_t rawTime;
    struct tm *timeInfo;
    cbuf_handle_t cbuf;
    sem_t *empty, *mutex, *full;

    if (name == 'A')
    {
        cbuf = Acbuf;
        empty = Aempty;
        mutex = Amutex;
        full = Afull;
    }
    else if (name == 'B')
    {
        cbuf = Bcbuf;
        empty = Bempty;
        mutex = Bmutex;
        full = Bfull;
    }
    else if (name == 'C')
    {
        cbuf = Ccbuf;
        empty = Cempty;
        mutex = Cmutex;
        full = Cfull;
    }
    else
    {
        printf("\n\nCONSUMER - WRONG NAME\n\n");
        exit(-1);
    }

    for (int k = 0; k < CONSUMER_CYCLES; k++)
    {
        char *msg;
        time(&rawTime);
        timeInfo = localtime(&rawTime);
        printf("[%02d:%02d:%02d][..][CONSUMER %c] wait for buf [%c]\n", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec, name, name);

        if (sem_wait(full) == -1)
            perror("A sem_wait: empty"); // Semaphore down operation
        if (sem_wait(mutex) == -1)
            perror("A sem_wait: mutex");
        msg = circular_buf_get(cbuf);

        time(&rawTime);
        timeInfo = localtime(&rawTime);
        printf("[%02d:%02d:%02d][!!][CONSUMER %c] Taken item [%s]. [Buffer %c] After take: ", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec, name, msg, name);
        circular_buf_print(cbuf);

        if (sem_post(mutex) == -1)
            perror("sem_post: mutex");

        if (sem_post(empty) == -1)
            perror("sem_post: full"); // Semaphore up operation

        usleep(500000);

        char letter = msg[0];
        if (strlen(msg) != 0)
        {
            msg++;

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
            strcpy(s, msg);
            if (rand() % pr == 0)
            {
                char *add;
                if ((rand() % 3) == 0)
                    add = "A";
                if ((rand() % 3) == 1)
                    add = "B";
                if ((rand() % 3) == 2)
                    add = "C";
                s = strncat(s, add, 1);
            }
            printf("[%02d:%02d:%02d][..][CONSUMER %c] New item: [%s] wait for buf [%c]\n", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec, name, s, letter);
            if (sem_wait(empty) == -1)
                perror("A sem_wait: empty"); // Semaphore down operation
            if (sem_wait(mutex) == -1)
                perror("A sem_wait: mutex");
            if (circular_buf_put2(cbuf, id) == -1)
                perror("Cbuf put");
            time(&rawTime);
            timeInfo = localtime(&rawTime);
            printf("[%02d:%02d:%02d][!!][CONSUMER %c] Put item [%s]. [Buffer %c] After input: ", timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec, name, s, letter);
            circular_buf_print(cbuf);
            if (sem_post(mutex) == -1)
                perror("sem_post: mutex");
            if (sem_post(full) == -1)
                perror("sem_post: full"); // Semaphore up operation
        }

        usleep(CONSUMER_SLEEP_TIME);
    }
    return;
}
