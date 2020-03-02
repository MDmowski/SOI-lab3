#include "producer.c"
#include "consumer.c"
#include "specialProducer.c"
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "const.h"
// #include "cbuf.c"

sem_t *Amutex, *Bmutex, *Cmutex, *Aempty, *Bempty, *Cempty, *Afull, *Bfull, *Cfull;
circular_buf_t *A_cbufferHandle, *B_cbufferHandle, *C_cbufferHandle;
int A_bufferId, A_cBufferId, B_bufferId, B_cBufferId, C_bufferId, C_cBufferId;
int *A_buffer, *B_buffer, *C_buffer;

int offset;

void semaphoresInit()
{
    sem_unlink(A_MUTEX_SEM_NAME);
    if ((Amutex = sem_open(A_MUTEX_SEM_NAME, O_CREAT | O_EXCL, 666, 1)) == SEM_FAILED)
        printf("A Mutex open fail\n");
    sem_unlink(B_MUTEX_SEM_NAME);
    if ((Bmutex = sem_open(B_MUTEX_SEM_NAME, O_CREAT | O_EXCL, 666, 1)) == SEM_FAILED)
        printf("B Mutex open fail\n");
    sem_unlink(C_MUTEX_SEM_NAME);
    if ((Cmutex = sem_open(C_MUTEX_SEM_NAME, O_CREAT | O_EXCL, 666, 1)) == SEM_FAILED)
        printf("C Mutex open fail\n");
    sem_unlink(A_EMPTY_SEM_NAME);
    if ((Aempty = sem_open(A_EMPTY_SEM_NAME, O_CREAT | O_EXCL, 666, A_SIZE)) == SEM_FAILED)
        printf("A Empty open fail\n");
    sem_unlink(B_EMPTY_SEM_NAME);
    if ((Bempty = sem_open(B_EMPTY_SEM_NAME, O_CREAT | O_EXCL, 666, B_SIZE)) == SEM_FAILED)
        printf("B Empty open fail\n");
    sem_unlink(C_EMPTY_SEM_NAME);
    if ((Cempty = sem_open(C_EMPTY_SEM_NAME, O_CREAT | O_EXCL, 666, C_SIZE)) == SEM_FAILED)
        printf("A Empty open fail\n");
    sem_unlink(A_FULL_SEM_NAME);
    if ((Afull = sem_open(A_FULL_SEM_NAME, O_CREAT | O_EXCL, 666, 0)) == SEM_FAILED)
        printf("A Full open fail\n");
    sem_unlink(B_FULL_SEM_NAME);
    if ((Bfull = sem_open(B_FULL_SEM_NAME, O_CREAT | O_EXCL, 666, 0)) == SEM_FAILED)
        printf("B Full open fail\n");
    sem_unlink(C_FULL_SEM_NAME);
    if ((Cfull = sem_open(C_FULL_SEM_NAME, O_CREAT | O_EXCL, 666, 0)) == SEM_FAILED)
        printf("C Full open fail\n");
}

int main()
{

    semaphoresInit();

    //Initializing buffer A
    if ((A_bufferId = shm_open(A_BUFFER_SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU)) == -1)
        perror("shm_open");
    if ((A_cBufferId = shm_open(A_CBUFFER_SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU)) == -1)
        perror("shm_open");
    if (ftruncate(A_cBufferId, sizeof(struct circular_buf_t)) == -1)
        perror("ftruncate");
    if (ftruncate(A_bufferId, A_SIZE * sizeof(int)) == -1)
        perror("ftruncate");
    if ((A_cbufferHandle = mmap(
             NULL,
             sizeof(struct circular_buf_t),
             PROT_READ | PROT_WRITE,
             MAP_SHARED,
             A_cBufferId,
             0)) == MAP_FAILED)
        perror("mmap");
    if ((A_buffer = mmap(
             NULL,
             A_SIZE * sizeof(int),
             PROT_READ | PROT_WRITE,
             MAP_SHARED,
             A_bufferId,
             0)) == MAP_FAILED)
        perror("mmap");
    // A_bufferId = shmget(BUF_A_KEY, A_SIZE * sizeof(int), IPC_CREAT);
    // A_buffer = (int *)shmat(A_bufferId, NULL, 0);
    // A_cBufferId = shmget(CBUF_A_KEY, sizeof(circular_buf_t), IPC_CREAT);
    // A_cbufferHandle = shmat(A_cBufferId, NULL, 0);
    A_cbufferHandle = circular_buf_init(A_cbufferHandle, A_buffer, A_SIZE);

    //Initializing buffer B
    if ((B_bufferId = shm_open(B_BUFFER_SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU)) == -1)
        perror("shm_open");
    if ((B_cBufferId = shm_open(B_CBUFFER_SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU)) == -1)
        perror("shm_open");
    if (ftruncate(B_cBufferId, sizeof(struct circular_buf_t)) == -1)
        perror("ftruncate");
    if (ftruncate(B_bufferId, B_SIZE * sizeof(int)) == -1)
        perror("ftruncate");
    if ((B_cbufferHandle = mmap(
             NULL,
             sizeof(struct circular_buf_t),
             PROT_READ | PROT_WRITE,
             MAP_SHARED,
             B_cBufferId,
             0)) == MAP_FAILED)
        perror("mmap");
    if ((B_buffer = mmap(
             NULL,
             B_SIZE * sizeof(int),
             PROT_READ | PROT_WRITE,
             MAP_SHARED,
             B_bufferId,
             0)) == MAP_FAILED)

        perror("mmap");
    B_cbufferHandle = circular_buf_init(B_cbufferHandle, B_buffer, B_SIZE);


    //Initializing buffer C
    if ((C_bufferId = shm_open(C_BUFFER_SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU)) == -1)
        perror("shm_open");
    if ((C_cBufferId = shm_open(C_CBUFFER_SHM_NAME, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU)) == -1)
        perror("shcm_open");
    if (ftruncate(C_cBufferId, sizeof(struct circular_buf_t)) == -1)
        perror("ftruncate");
    if (ftruncate(C_bufferId, C_SIZE * sizeof(int)) == -1)
        perror("ftruncate");
    if ((C_cbufferHandle = mmap(
             NULL,
             sizeof(struct circular_buf_t),
             PROT_READ | PROT_WRITE,
             MAP_SHARED,
             C_cBufferId,
             0)) == MAP_FAILED)
        perror("mmap");
    if ((C_buffer = mmap(
             NULL,
             C_SIZE * sizeof(int),
             PROT_READ | PROT_WRITE,
             MAP_SHARED,
             C_bufferId,
             0)) == MAP_FAILED)
        perror("mmap");
    C_cbufferHandle = circular_buf_init(C_cbufferHandle, C_buffer, C_SIZE);

    if (fork() == 0)
    {
        srand(time(NULL) ^ (getpid() << 16));
        producer(A_cbufferHandle, Aempty, Amutex, Afull, "A", 200);
        exit(0);
    }
    if (fork() == 0)
    {
        srand(time(NULL) ^ (getpid() << 16));
        producer(B_cbufferHandle, Bempty, Bmutex, Bfull, "B", 500);

        exit(0);
    }
    if (fork() == 0)
    {
        srand(time(NULL) ^ (getpid() << 16));
        producer(C_cbufferHandle, Cempty, Cmutex, Cfull, "C", 1000);
        exit(0);
    }
    if (fork() == 0)
    {
        sleep(2);
        srand(time(NULL) ^ (getpid() << 16));
        consumer(A_cbufferHandle, Aempty, Amutex, Afull, B_cbufferHandle, Bempty, Bmutex, Bfull, C_cbufferHandle, Cempty, Cmutex, Cfull, 'A', 1500, 50);
        exit(0);
    }
    if (fork() == 0)
    {
        sleep(2);
        srand(time(NULL) ^ (getpid() << 16));
        consumer(A_cbufferHandle, Aempty, Amutex, Afull, B_cbufferHandle, Bempty, Bmutex, Bfull, C_cbufferHandle, Cempty, Cmutex, Cfull, 'B', 2000, 10);
        exit(0);
    }
    if (fork() == 0)
    {
        sleep(2);
        srand(time(NULL) ^ (getpid() << 16));
        consumer(A_cbufferHandle, Aempty, Amutex, Afull, B_cbufferHandle, Bempty, Bmutex, Bfull, C_cbufferHandle, Cempty, Cmutex, Cfull, 'C', 2500, 5);
        exit(0);
    }
    if (fork() == 0)
    {
        srand(time(NULL) ^ (getpid() << 16));
        specialProducer(A_cbufferHandle, Aempty, Amutex, Afull, B_cbufferHandle, Bempty, Bmutex, Bfull, C_cbufferHandle, Cempty, Cmutex, Cfull, 3000);
        exit(0);
    }

    pid_t wpid;
    int status;

    while ((wpid = wait(&status)) > 0)
        ;
    printf("All child processes have finished. Returning to parent process.\n");
    puts("BUFFER A");
    circular_buf_print(A_cbufferHandle);
    puts("BUFFER B");
    circular_buf_print(B_cbufferHandle);
    puts("BUFFER C");
    circular_buf_print(C_cbufferHandle);

    circular_buf_clear(A_cbufferHandle);
    circular_buf_clear(B_cbufferHandle);
    circular_buf_clear(C_cbufferHandle);

    // Cleanup memory
    if (munmap(A_cbufferHandle, sizeof(struct circular_buf_t)) == -1)
        perror("munmap");
    if (munmap(A_buffer, A_SIZE * sizeof(int)) == -1)
        perror("munmap");
    if (close(A_bufferId) == -1)
        perror("close");
    if (close(A_cBufferId) == -1)
        perror("close");
    if (shm_unlink(A_BUFFER_SHM_NAME) == -1)
        perror("shm_unlink");
    if (shm_unlink(A_CBUFFER_SHM_NAME) == -1)
        perror("shm_unlink");
    puts("Buffer A unlinked.");

    if (munmap(B_cbufferHandle, sizeof(struct circular_buf_t)) == -1)
        perror("munmap");
    if (munmap(B_buffer, A_SIZE * sizeof(int)) == -1)
        perror("munmap");
    if (close(B_bufferId) == -1)
        perror("close");
    if (close(B_cBufferId) == -1)
        perror("close");
    if (shm_unlink(B_BUFFER_SHM_NAME) == -1)
        perror("shm_unlink");
    if (shm_unlink(B_CBUFFER_SHM_NAME) == -1)
        perror("shm_unlink");
    puts("Buffer B unlinked.");

    if (munmap(C_cbufferHandle, sizeof(struct circular_buf_t)) == -1)
        perror("munmap");
    if (munmap(C_buffer, A_SIZE * sizeof(int)) == -1)
        perror("munmap");
    if (close(C_bufferId) == -1)
        perror("close");
    if (close(C_cBufferId) == -1)
        perror("close");
    if (shm_unlink(C_BUFFER_SHM_NAME) == -1)
        perror("shm_unlink");
    if (shm_unlink(C_CBUFFER_SHM_NAME) == -1)
        perror("shm_unlink");
    puts("Buffer C unlinked.");

    if (sem_close(Aempty) == -1)
        perror("sem_unlink");
    if (sem_close(Bempty) == -1)
        perror("sem_unlink");
    if (sem_close(Cempty) == -1)
        perror("sem_unlink");
    if (sem_close(Amutex) == -1)
        perror("sem_unlink");
    if (sem_close(Bmutex) == -1)
        perror("sem_unlink");
    if (sem_close(Cmutex) == -1)
        perror("sem_unlink");
    if (sem_close(Afull) == -1)
        perror("sem_unlink");
    if (sem_close(Bfull) == -1)
        perror("sem_unlink");
    if (sem_close(Cfull) == -1)
        perror("sem_unlink");
    puts("Semaphores closed.");

    if (sem_unlink(A_MUTEX_SEM_NAME) == -1)
        perror("sem_unlink");
    if (sem_unlink(A_EMPTY_SEM_NAME) == -1)
        perror("sem_unlink");
    if (sem_unlink(A_FULL_SEM_NAME) == -1)
        perror("sem_unlink");
    if (sem_unlink(B_MUTEX_SEM_NAME) == -1)
        perror("sem_unlink");
    if (sem_unlink(B_EMPTY_SEM_NAME) == -1)
        perror("sem_unlink");
    if (sem_unlink(B_FULL_SEM_NAME) == -1)
        perror("sem_unlink");
    if (sem_unlink(C_MUTEX_SEM_NAME) == -1)
        perror("sem_unlink");
    if (sem_unlink(C_EMPTY_SEM_NAME) == -1)
        perror("sem_unlink");
    if (sem_unlink(C_FULL_SEM_NAME) == -1)
        perror("sem_unlink");
    puts("Semaphores unlinked.");

    puts("Everything cleared.");

    return 0;
}
