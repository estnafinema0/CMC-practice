#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <unistd.h>


int work = 1;
int flag=1;
unsigned char ch=0;

void fio(int s){
    signal(SIGIO, fio);
    work = 0;
}
void usr1(int s){
    signal(SIGUSR1, usr1);
    ch = 0;
    flag = 0;
}

void usr2(int s){
    signal(SIGUSR2, usr2);
    ch = 1;
    flag = 0;
    
}
void alrm(int s){
    signal(SIGALRM, alrm);
    //printf("ARRM\n");
}


int main(int argc, char** argv){
    if (argc != 2){
        perror("Error\n");
        return 0;
    }
    signal(SIGUSR1, usr1);
    signal(SIGUSR2, usr2);
    signal(SIGALRM, alrm);
    signal(SIGIO, fio);
    pid_t pid1, pid2;
    pid1 = fork();
    if (pid1 < 0){
        perror("Error\n"); return 0;
    }
    FILE* f = fopen(argv[1], "r+");
    if (pid1 != 0){ //father
        pid2 = fork();
        if (pid2 < 0){perror("Error\n"); return 0;}
        if (pid2 == 0){ //son2
            char s = 0;
            while (fread(&s, sizeof(char), 1, f) > 0){
                for (int i = 0; i < 8; i++){
                    unsigned char y = s;               
                    y <<= i;
                    y >>= 7;
                    if (y == 0) kill(pid1, SIGUSR1);
                    else kill(pid1, SIGUSR2);
                    pause();
                }
        }
        kill(pid1, SIGIO);
    }
        else{ // father
            wait(0);
            wait(0);
        }
    }

    if (pid1 == 0){ // son1
        unsigned char data = 0;
        while (work){
            data = 0;
            for (int i = 0; i < 8; i++)
            {
                if (flag == 1) pause();
                if (work == 0) {
                    break;
                    }
                data <<= 1;
                data += ch;
                flag = 1;
                kill(0, SIGALRM);
            }
            if (work == 0){
                break;
            }
            putchar(data);
        }
    }

    fclose(f);
    return 0;
}