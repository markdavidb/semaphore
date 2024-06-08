#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

// ANSI escape codes for colors
#define RED     "\x1B[31m"
#define GREEN   "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE    "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN    "\x1B[36m"
#define RESET   "\x1B[0m"

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

int semid;
union semun arg;

int createSemaphoreSet()
{

    semid = semget(IPC_PRIVATE, 5, 0600);

    if (semid == -1)
    {
        perror("semget");
        exit(1);
    }
}

int destroySemaphoreSet()
{
    if (semctl(semid, 0, IPC_RMID, 0) == -1)
    {
        perror("semctl");
        exit(-1);
    }
}

int initializeSemaphores()
{

    for (int i = 0; i < 5; i++)
    {
        arg.val = 4- i;
        if (semctl(semid, i, SETVAL, arg) == -1)
        {
            perror("semctl");
            exit(1);
        }
    }
}

int adjustSemaphore(int num, int changeVal)
{

    struct sembuf sem_operation;
    sem_operation.sem_num = num;
    sem_operation.sem_op = changeVal;
    sem_operation.sem_flg = 0;
    semop(semid, &sem_operation, 1);
}

int releaseSemaphore(int num)
{
    adjustSemaphore(num, 1);
}

int acquireSemaphore(int num)
{
    adjustSemaphore(num, -4);
}

int nextStep(int num)
{
    for (int i = 0; i < 5; i++)
    {
        if (i != num)
        {
            releaseSemaphore(i);
        }
    }
}

int process(int semnum)
{
    const char* colors[] = {RED, GREEN, YELLOW, BLUE, MAGENTA};
    int i = semnum;
    while (i < 101)
    {
        acquireSemaphore(semnum);
        printf("%sProcess %d: %d%s\n", colors[semnum], semnum, i, RESET);
        nextStep(semnum);
        i += 5;
    }
}

int main(int argc, char *argv[])
{
    createSemaphoreSet();
    initializeSemaphores();
    int pid;
    for (int i = 0; i < 5; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            process(i);
            exit(0);
        }
    }

    for (int i = 0; i < 5; i++)
    {
        wait(NULL);
    }

    destroySemaphoreSet();
    return 0;
}