#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <string.h>

struct sembuf sem_lock = {0, -1, 0}; 
struct sembuf sem_unlock = {0, 1, 0}; 

void process1(int semid1, int semid2, const char *file, int N) 
{
    int fd = open(file, O_WRONLY | O_APPEND);
    if (fd == -1) 
    {
		fprintf(stderr, "can not open the file!\n");
		exit(1);
	}

    for (int i = 1; i <= N; i++) 
    {
        semop(semid1, &sem_lock, 1); 
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%d %d\n", i, getpid());
        write(fd, buffer, strlen(buffer));
        semop(semid2, &sem_unlock, 1); 
    }

    close(fd);
}

void process2(int semid2, int semid3, const char *file, int N) 
{
    int fd = open(file, O_RDWR);
    if (fd == -1) 
    {
        fprintf(stderr, "can not open the file!\n");
        exit(1);
    }
	lseek(fd, 0, SEEK_SET);
    for (int i = 1; i <= N; i++) 
    {
        semop(semid2, &sem_lock, 1); 
        char buff[256];
        int dif = i > 10 ? 2 : 1;
        if (i > 1)
			lseek(fd, 13 + dif, SEEK_CUR);
        read(fd, buff, sizeof(buff) - 1);
        buff[strcspn(buff, "\n")] = 0;
        snprintf(buff + strlen(buff), sizeof(buff), " %d\n", getpid());
        write(fd, buff, strlen(buff));
        semop(semid3, &sem_unlock, 1); 
    }

    close(fd);
}

void process3(int semid3, int semid1, const char *filename, int N) 
{
    int fd = open(filename, O_RDWR);
    if (fd == -1) 
    {
        fprintf(stderr, "can not open the file!\n");
        exit(1);
    }

    for (int i = 1; i <= N; i++) 
    {
        semop(semid3, &sem_lock, 1); 
        char buff[256];
        read(fd, buff, sizeof(buff) - 1);
        buff[strcspn(buff, "\n")] = 0;
        snprintf(buff + strlen(buff), sizeof(buff), "% d\n", getpid());
        write(fd, buff, strlen(buff));
        semop(semid1, &sem_unlock, 1); 
    }

    close(fd);
}

int main(int argc, char **argv) 
{
    if (argc < 5) 
    {
        fprintf(stderr, "wrong number of arguments!\n");
        return 1;
    }

    int processNum = atoi(argv[1]);
    char *keyF = argv[2];
    char keyCh = argv[3][0];
    char *shared = argv[4];
    int N = atoi(argv[5]);

    key_t key = ftok(keyF, keyCh);
    if (key == -1) 
    {
        fprintf(stderr, "can not use ftok!\n");
        exit(1);
    }

    int semid1 = semget(key, 1, IPC_CREAT | 0666);
    int semid2 = semget(key + 1, 1, IPC_CREAT | 0666);
    int semid3 = semget(key + 2, 1, IPC_CREAT | 0666);

    if (semid1 == -1 || semid2 == -1 || semid3 == -1) 
    {
        fprintf(stderr, "can not use semget!\n");
        exit(1);
    }
    
    if (processNum == 0) 
    {
        semctl(semid1, 0, SETVAL, 1); // начинает
        semctl(semid2, 0, SETVAL, 0); 
        semctl(semid3, 0, SETVAL, 0); 
        int fd = open(shared, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd == -1) 
        {
            fprintf(stderr, "can not open the file!\n");
            exit(1);
        }
        close(fd);
        printf("run processes 1, 2, and 3.\n");
        exit(0);
    }


    if (processNum == 1) 
    {
        process1(semid1, semid2, shared, N);
    } 
    else if (processNum == 2) 
    {
        process2(semid2, semid3, shared, N);
    } 
    else if (processNum == 3) 
    {
        process3(semid3, semid1, shared, N);
    } 
    else 
    {
        fprintf(stderr, "cwrong number of process!\n");
        exit(1);
    }

    return 0;
}