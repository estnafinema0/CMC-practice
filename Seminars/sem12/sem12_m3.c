#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>  
#include <string.h>
#include <errno.h>
//процесс 3
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


    int msqid = msgget(key, 0666);
    if (msqid == -1) {
        perror("msgget");
        exit(1);
    }

    struct msgbuf sbuf;
    sbuf.mtype = 1; 
    struct msgbuf rbuf;
    for (int i = 1; i <= N; i++) {
        if (msgrcv(msqid, &rbuf, 0, 3, 0) == -1) {
            perror("msgrcv from process 2");
            exit(1);
        }

        FILE *file = fopen(shared_filename, "r");
        if (file == NULL) {
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

        file = fopen(shared_filename, "a");
        if (file == NULL) {
            perror("fopen");
            exit(1);
        }
        fprintf(file, "%s", new);
        fclose(file);

        if (msgsnd(msqid, &sbuf, 0, 0) == -1) {
            perror("msgsnd to process 1");
            exit(1);
        }
    }

    return 0;
}
