#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

/*Сбрасывается ли обработка сигнала на значение по умолчанию после вызова обработчика, 
если нет его явной переустановки?
*/
/*в тестируемой системе ответ - да.
*/

void handler1_1(int s){
    printf("Сброс сигнала.\n");
}

int main(){
    signal(SIGINT, handler1_1);
    while (1){
        pause();
    }
    return 0;
}