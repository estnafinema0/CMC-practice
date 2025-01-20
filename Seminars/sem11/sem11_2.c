
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

volatile int switch_son = 1, write_flag = 1;
unsigned char bit = 0;

void handler_sigusr1(int s);
void handler_sigusr2(int s);
void handler_alarm(int s);
void handler_sigio(int s);

int main(int argc, char** argv){
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    signal(SIGUSR1, handler_sigusr1);
    signal(SIGUSR2, handler_sigusr2);
    signal(SIGALRM, handler_alarm);
    signal(SIGIO, handler_sigio);

    FILE* fd = fopen(argv[1], "r+");
    pid_t pid_1, pid_2;
    pid_1 = fork();

    if (pid_1 == -1){
            perror("[Error]: creating fork.");
            fclose(fd);
            exit(EXIT_FAILURE);
    } else if (pid_1 > 0){ 
        pid_2 = fork();
        if (pid_1 == -1){
                perror("[Error]: creating fork.");
                fclose(fd);
                exit(EXIT_FAILURE);
        }
        if (pid_2 == 0){ 
            char byte = 0;
            while (fread(&byte, sizeof(char), 1, fd) > 0)
                for (int i = 0; i < 8; i++){
                    unsigned char modified_byte = byte;               
                    modified_byte <<= i;
                    modified_byte >>= 7;
                    if (modified_byte == 0) kill(pid_1, SIGUSR1);
                    else kill(pid_1, SIGUSR2);
                    pause();
                }
        
            kill(pid_1, SIGIO);
        }
        else // Waiting for child 1 and 2 to end 
            for (int i = 0; i < 2; i++) wait(0);
    }

    if (pid_1 == 0){ 
        unsigned char value = 0;
        while (write_flag){
            value = 0;
            for (int i = 0; i < 8; i++)
            {
                if (switch_son  == 1) pause();
                if (write_flag == 0) break;
                value <<= 1;
                value += bit;
                switch_son  = 1;
                kill(0, SIGALRM);
            }
            if (write_flag == 0) break;
            putchar(value);
        }
    }
    fclose(fd);
    return 0;
}

void handler_alarm(int s)
{
    signal(SIGALRM, handler_alarm);
}

void handler_sigio(int s)
{
    signal(SIGIO, handler_sigio);
    write_flag = 0;
}
void handler_sigusr1(int s)
{
    signal(SIGUSR1, handler_sigusr1);
    switch_son = 0;
    bit = 0;
}

void handler_sigusr2(int s)
{
    signal(SIGUSR2, handler_sigusr2);
    switch_son  = 0;
    bit = 1;
    
}