#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>  
#include <string.h>
#include <errno.h>
//процесс 1

/*Для запуска необходимо последовательно в 3х терминалах запустить программы: сначала m1, m2, m3.
*/
// gcc -g -Wall sem12_m1.c -o m1 && ./m1 keyfile a sharedfile 5
// gcc -g -Wall sem12_m2.c -o m2 && ./m2 keyfile a sharedfile 5
// gcc -g -Wall sem12_m3.c -o m3 && ./m3 keyfile a sharedfile 5

struct msgbuf {
    long    mtype;
    char    mtext[1];
};

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s 5 args\n", argv[0]);
        exit(1);
    }

    char *shared_filename = argv[3];
    int N = atoi(argv[4]);

    key_t key = ftok(argv[1],  argv[2][0]);
    if (key == -1) {
        perror("ftok");
        exit(1);
    }


    int msqid = msgget(key, IPC_CREAT | 0666);
    if (msqid == -1) {
        perror("msgget");
        exit(1);
    }

    pid_t pid = getpid();

    struct msgbuf sbuf;
    sbuf.mtype = 2; 

    struct msgbuf rbuf;

    for (int i = 1; i <= N; i++) {
        FILE *file;

        if (i == 1) {
            file = fopen(shared_filename, "w");
            if (file == NULL) {
                perror("fopen");
                exit(1);
            }
            fclose(file);
        }

        file = fopen(shared_filename, "a");
        if (file == NULL) {
            perror("fopen");
            exit(1);
        }
        fprintf(file, "%d %d\n", i, pid);
        fclose(file);

        msgsnd(msqid, &sbuf, 0, 0);
        msgrcv(msqid, &rbuf, 0, 1, 0);
    
    }
    msgctl(msqid, IPC_RMID, NULL);

    return 0;
}
