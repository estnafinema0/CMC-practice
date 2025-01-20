#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>
// процесс 3
union semun
{
    int val;
};

void Decrement(int semid, int semnum);
void Increment(int semid, int semnum);

int main(int argc, char *argv[])
{
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <filename_key> a <shared_file> <N>\n", argv[0]);
        exit(1);
    }

    int N = atoi(argv[4]);

    key_t key = ftok( argv[1], argv[2][0]);
    if (key == -1) {
        perror("ftok");
        exit(1);
    }
    int semid = semget(key, 3, 0666);

    for (int i = 1; i <= N; ++i) {
        Decrement(semid, 2);

        FILE *file = fopen(argv[3], "r");
        if (!file) {
            perror("fopen");
            exit(1);
        }
           char buffer[1024], new[1024]; 
    char tmp[1024] = ""; 
    char previous[1024] = ""; 

    while (fgets(buffer, sizeof(buffer), file)) {
        strcpy(previous, tmp); 
        strcpy(tmp, buffer); 
    }

        fclose(file);

        previous[strcspn(previous, "\n")] = 0;
        snprintf(new, 1000024, "%s %d\n", previous, getpid());

        file = fopen(argv[3], "a");
        if (!file) {
            perror("fopen");
            exit(1);
        }
        fputs(new, file);
        fclose(file);

        Increment(semid, 0);
    }
    
    Increment(semid, 3); // для удаления семафоров
    int finished_count = semctl(semid, 3, GETVAL);
    if (finished_count == 3) {
        if (semctl(semid, 0, IPC_RMID) == -1)
            perror("semctl IPC_RMID");
    }
    
    return 0;
}

void Decrement(int semid, int semnum)
{
    struct sembuf op = {semnum, -1, 0};
    if (semop(semid, &op, 1) == -1){
        perror("semop P");
        exit(1);
    }
}

void Increment(int semid, int semnum)
{
    struct sembuf op = {semnum, +1, 0};
    if (semop(semid, &op, 1) == -1){
        perror("semop V");
        exit(1);
    }
}
