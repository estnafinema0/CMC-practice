#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
/* Возобновляет ли работу функция wait(), 
если во время ожидания пришел сигнал, или она завершается с ошибкой?*/

/*Данная проверка позволяет узнать. 
Если wait() завершается успешно со значением отличным -1, то функция возобновляется.
Создается сыновний процесс. Sigint вызывает отцовский процесс.
в тестируемой системе ответ - нет.
*/

void handler1_4(int signum) {
    printf("[HANDLER]Получен сигнал %d.\n", signum);
}

int main(){
    pid_t pid = fork();
    if (pid == 0) {
        sleep(5);
        printf("Сыновний процесс завершён.\n");
        exit(0);
    } else {
        signal(SIGINT, handler1_4);
        printf("Отец ждет завершения сыновниго процесса. ВВедите Ctrl+C (SIGINT).\n");
        int status;
        int res = wait(&status);

        if (res != -1)  {
            printf("Сыновний процесс завершён.\n");
        }
    }
    return 0;

}