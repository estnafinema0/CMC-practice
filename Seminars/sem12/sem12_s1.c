#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

/*Для запуска необходимо последовательно в 3х терминалах запустить программы: сначала s1, s2, s3.
*/
// gcc -g -Wall sem12_s1.c -o s1 && ./s1 keyfile a sharedfile 5
// gcc -g -Wall sem12_s2.c -o s2 && ./s2 keyfile a sharedfile 5
// gcc -g -Wall sem12_s3.c -o s3 && ./s3 keyfile a sharedfile 5

// ipcs -s | awk 'NR > 3 {print $2}' | xargs -I {} ipcrm -s {}
// удаление всех ipcs из терминала 

int semid;

union semun {
    int val;
};

void Decrement(int semid, int semnum);
void Increment(int semid, int semnum);

int main(int argc, char *argv[]) {
    if(argc != 5) {
        fprintf(stderr, "Usage: %s <filename_key> a <shared_file> <N>\n", argv[0]);
        exit(1);
    }
    
    char *key_filename = argv[1];
    char key_char = argv[2][0];                                                                                                                                                                                 
    char *shared_file = argv[3];
    int N = atoi(argv[4]);

    key_t key = ftok(key_filename, key_char);
    if(key == -1) {
        perror("ftok");
        exit(1);
    }
    
    semid = semget(key, 4, IPC_CREAT | IPC_EXCL | 0666); // 0, 1, 2 - for main programs and 3 for indicating og ending their job

    union semun sem_union;

    if(semid != -1) {
        sem_union.val = 1; semctl(semid, 0, SETVAL, sem_union);
        sem_union.val = 0; semctl(semid, 1, SETVAL, sem_union);
        sem_union.val = 0; semctl(semid, 2, SETVAL, sem_union);
        sem_union.val = 0; semctl(semid, 3, SETVAL, sem_union); //для синхронизации удаления семафоров
        FILE *file = fopen(shared_file, "w");
        if(!file) {
            perror("fopen");
            exit(1);
        }
        fclose(file);
    } else 
        semid = semget(key, 3, 0666);

    pid_t pid = getpid();

    for(int i = 1; i <= N; ++i) {
        Decrement(semid, 0);

        FILE *file = fopen(shared_file, "a");
        if(!file) {
            perror("fopen");
            exit(1);
        }
        fprintf(file, "%d %d\n", i, pid);
        fclose(file);

        Increment(semid, 1);
    }

    Increment(semid, 3); // для удаления семафоров
    int finished_count = semctl(semid, 3, GETVAL);
    if (finished_count == 3) {
        if (semctl(semid, 0, IPC_RMID) == -1)
            perror("semctl IPC_RMID");
    }
    return 0;
}

void Decrement(int semid, int semnum) {
    struct sembuf op = {semnum, -1, 0};
    if(semop(semid, &op, 1) == -1) {
        perror("semop P");
        exit(1);
    }
}

void Increment(int semid, int semnum) {
    struct sembuf op = {semnum, +1, 0};
    if(semop(semid, &op, 1) == -1) {
        perror("semop V");
        exit(1);
    }
}