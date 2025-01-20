#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
/*Возникает ли процесс «зомби» при завершении порожденного процесса, 
если сигнал SIGCHLD родительским процессом
а) перехватывается, б) не перехватывается, в) игнорируется.
*/
/*Данная проверка позволяет узнать. 
В тестируемой системе ответ:
при перехвате и игнорировании сигнала зомби-процесс не возникает, 
при не перехвате сигнала, то возможно, пока не зваершится родитель.
Проверка в отдельном терминале на зомби процесс.
*/

void handler1_5_perechvat(int signum) {
    printf("[HANDLER] SIGCHLD перехвачен.\n");
    wait(NULL);
}

int main() {

    //signal(SIGCHLD, handler1_5_perechvat);
    //signal(SIGCHLD, SIGIGN); // для игнорирования
    pid_t pid = fork();

    if (pid == 0) {
        printf("сын завершился.\n");
        _exit(0);
    } else {
        sleep(45);
        printf("ОТЕЦ завершился.\n");
    }

    return 0;
}