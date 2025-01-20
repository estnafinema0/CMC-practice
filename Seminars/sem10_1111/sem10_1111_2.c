
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
/*В командной строке заданы имя файла и число N.
Написать программу, выводящую на экран содержимое файла порциями по N строк. 
Каждая последующая порция выдаётся после нажатия Ctrl-С.
Когда весь файл будет выведен на экран, процесс создает потомка, а сам завершает работу.
Потомок переходит в фоновый режим, с помощью функции system запускает команду «ps j», 
а затем в фоновом режиме создает новый файл — вставляет после каждой строки исходного N пустых строк.
*/

FILE* file = NULL;
volatile int N = 0;
volatile int check = 0;

void handler2(int signum){
    char c;
    int count = 0;

    while (fscanf(file, "%c", &c) == 1) {
        putchar(c);
        if (c == '\n')
            count++;
        if (count == N) 
            return;
    }
    check = 1; 
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <filename> <N>\n", argv[0]);
        return 1;
    }

    file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("[Error] opening file");
        return 1;
    }

    N = atoi(argv[2]);

    signal(SIGINT, handler2);

    while (!check) {
        pause();
    }

    signal(SIGINT, SIG_DFL);

    pid_t pid = fork();
    if (pid < 0) {
        perror("[ERROR] fork failed");
        fclose(file);
        return 1;
    } else if (pid == 0) {
        setpgid(0, 0);

        system("ps j");

        FILE *new_file = fopen("new_file.txt", "w");
        if (new_file == NULL) {
            perror("Error opening new_file.txt");
            return 1;
        }
    char c;
        fseek(file, 0, SEEK_SET);
        while (fscanf(file, "%c", &c) == 1) {
            fputc(c, new_file);
            if (c == '\n')
                for (int i = 0; i < N; i++)
                    fputc('\n', new_file); 

        }

        fclose(new_file); 
        return 0;
    } 
    fclose(file); 
    return 0;
}