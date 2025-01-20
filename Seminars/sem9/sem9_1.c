#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <dirent.h> 
#include <sys/stat.h> 
#include <sys/wait.h>
 
enum {BUFFER_SIZE = 256};

int main(int argc, char** argv) {
    if (argc != 2) { 
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]); 
        return 1;
    } 
    int fdS[2], fdF[2];
    char buffer[BUFFER_SIZE];
    char upd_buffer[BUFFER_SIZE];
    const char *filename = argv[1];

    if ((pipe(fdS) == -1) || (pipe(fdF) == -1)) {
        perror("[ERROR]: Pipe failed.");
        return 1;
    }
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("[ERROR]: Open given file is failed");
        return 1;
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("[ERROR]: Fork failed");
        return 1;
    } else if (pid == 0) { 
        close(fdF[1]);
        close(fdS[0]);
        while (1) {
            ssize_t n = read(fdF[0], buffer, BUFFER_SIZE);
            if (n <= 0) break;
            if (strcmp(buffer, "exit") == 0) break;
            for (int i = 0; i < n; i++) 
                upd_buffer[i] = (buffer[i] == ' ') ? '*' : buffer[i];
            write(fd, upd_buffer, n);
            write(fd, "\n", 1); 
            
            fflush(NULL);
            write(fdS[1], buffer, BUFFER_SIZE);
            
        }
        close(fdF[0]);
        close(fdS[1]);
        close(fd); 
        exit(0);
    } else { 
        close(fdF[0]);
        close(fdS[1]);       
        printf("Enter lines to execute (to quit type 'exit'):\n");
        while (1) {
            fgets(buffer, BUFFER_SIZE, stdin);
            buffer[strcspn(buffer, "\n")] = 0;
            write(fd, buffer, strlen(buffer));
            write(fd, "\n", 1); 
            write(fdF[1], buffer, strlen(buffer) + 1);
            if (strcmp(buffer, "exit") == 0) break;
            
            fflush(NULL);
            read(fdS[0], buffer, BUFFER_SIZE);
        }
        close(fdF[1]);
        close(fdS[0]);
        close(fd); 
        wait(NULL);
    }
    return 0;
}
