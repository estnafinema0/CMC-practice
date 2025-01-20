
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*Написать программу, которая в командной строке получает целое число N — начальное значение счетчика. 
После запуска процесс находится в режиме ожидания поступления сигнала. 
При получении сигнала SIGTSTP (CtrI-Z) счетчик увеличивается на 3, 
при получении сигнала SIGINT (Ctrl-C) счетчик уменьшается на 4. 
При получении каждого сигнала на экран выводится номер и название сигнала, 
а также значение счетчика после его изменения. 
Работа программы прекращается, когда счетчик станет меньше нуля*/
volatile int counter;

void hSIGINT(int signum)
{
    counter -= 4;
    printf("\nCurrent state: SIGINT %d, counter: %d\n", signum, counter);
    if (counter < 0) exit(0);
}

void hSIGTSTP(int signum)
{
    counter += 3;
    printf("\nCurrent state: SIGTSTP %d, counter: %d\n", signum,  counter);
    if (counter<0) exit(0);
}

int main(int argc, char** argv)
{
    if (argc != 2){
        fprintf(stderr, "Usage: %s <N>\n", argv[0]);
        return 1;
    }
    counter = atoi(argv[1]);
    if (counter < 0){
        fprintf(stderr, "The counter is intially set as negative number.");
        return 0;
    }

    signal(SIGINT, hSIGINT);
    signal(SIGTSTP, hSIGTSTP);
    while (1) pause();

    return 0;
}
