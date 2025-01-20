#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

int pipefd[2]; // Канал для обмена данными
int current_pid = 0; // ID текущего процесса

void signal_handler(int sig) {
    // Пустая обработка сигнала для продолжения работы процесса
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <N> <start> <end>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);   // Количество процессов
    int start = atoi(argv[2]); // Начальное значение
    int end = atoi(argv[3]);  // Конечное значение

    if (N <= 1) {
        fprintf(stderr, "N должно быть больше 1\n");
        return 1;
    }

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    signal(SIGUSR1, signal_handler); // Обработчик сигналов для синхронизации

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {  // Дочерний процесс
        close(pipefd[1]);  // Закрыть записывающую сторону канала
        int number;
        read(pipefd[0], &number, sizeof(int));  // Чтение числа

        while (number <= end) {
            printf("Process %d: %d\n", getpid(), number);
            number++;

            if (number > end) break;  // Если число больше конечного, завершаем

            // Переход к следующему процессу
            kill(current_pid, SIGUSR1);  // Отправка сигнала родителю для продолжения
            pause();  // Ожидание сигнала от родителя

            write(pipefd[1], &number, sizeof(int));  // Запись числа в канал
        }
        close(pipefd[0]);
        exit(0);
    } else {  // Родительский процесс
        close(pipefd[0]);  // Закрыть читающую сторону канала
        int number = start;
        printf("Process %d: %d\n", getpid(), number);  // Первый вывод от главного процесса

        write(pipefd[1], &number, sizeof(int));  // Отправка первого числа дочерним процессам

        current_pid = pid; // Устанавливаем текущий pid дочернего процесса

        for (int i = 1; i < N; i++) {
            pause();  // Ожидание сигнала от дочернего процесса
        }

        // Ждем завершения всех дочерних процессов
        for (int i = 1; i < N; i++) {
            wait(NULL);
        }

        close(pipefd[1]);
    }

    return 0;
}
