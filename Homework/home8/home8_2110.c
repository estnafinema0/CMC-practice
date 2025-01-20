#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

enum { BLOCK_SIZE = 4 };

/* Lines of any legnth could be readed, because of using 
getString that accepts fd as an argument.

Code works correctly on empty files.
Code modifies correctly empty lines in file.
*/

void reverseWords(char *line);
char* getString(int fd);

int main(int argc, char **argv)
{
    if (argc != 2)
    {     
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1; 
    }

    int pipeFtC[2]; 
    int pipeCtF[2]; 
    if (pipe(pipeFtC) == -1 || pipe(pipeCtF) == -1)     
    {
        perror("[ERROR]: Pipe failed");
        return 2;
    }  
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("[ERROR]: Fork failed");
        return 1;
    } else if (pid == 0) { 
        close(pipeFtC[1]);
        close(pipeCtF[0]);

        int modified_len = 0;	
        while (1) {
            read(pipeFtC[0], &modified_len, sizeof(int));
            if (modified_len == -1)
                break;
            
            char *modified_line = malloc(modified_len + 1);
            if (modified_line == NULL) {
                perror("[ERROR]: memory allocation failed.\n");
                exit(1);
            }

            read(pipeFtC[0], modified_line, modified_len + 1);
            reverseWords(modified_line);
            write(pipeCtF[1], modified_line, modified_len + 1);
            free(modified_line);
        }

        close(pipeFtC[0]);
        close(pipeCtF[1]);
        exit(0);
    } else {
        close(pipeFtC[0]);
        close(pipeCtF[1]);
        
        int fd = open(argv[1], O_RDWR);
		if (fd == -1) {
            perror("[ERROR]: Failed to open file");
			return 1;
		}   

		int ch = 0;
		lseek(fd, -1, SEEK_END);
		read(fd, &ch, 1);
		if (ch != '\n')
			write(fd, "\n", 1);

		lseek(fd, 0, SEEK_SET);
        int len;

        while (1) {
            char *line = getString(fd);
            if (line == NULL) {
                int flag = -1;
                write(pipeFtC[1], &flag, sizeof(int));
                break;
            }

            len = strlen(line);
            write(pipeFtC[1], &len, sizeof(int));
            write(pipeFtC[1], line, len + 1);
            read(pipeCtF[0], line, len + 1); 

            if (len > 0) {
				lseek(fd, -(len + 1), SEEK_CUR); 
				write(fd, line, len); 
				write(fd, "\n", 1); 
            }
            free(line);    
        }
        close(pipeFtC[1]);
        close(pipeCtF[0]);
        close(fd);
        wait(0);
    }
       
    return 0;
}

void reverseWords(char *str) 
{
    int len = strlen(str), index = 0, wordStart;
    char temp[len + 2];
    for (int i = len - 1; i >= 0; i--) {
        if (str[i] == ' ' || i == 0) {
			wordStart = i;
			if (str[i] == ' ') wordStart = i + 1;
            for (int j = wordStart; j < len && str[j] != ' '; j++) 
                temp[index++] = str[j];
            if (i != 0)  temp[index++] = ' ';
        }
    }
    if (str[0] == ' ') {
		temp[index] = ' ';
		temp[index + 1] = '\0';
	}
	else temp[index] = '\0'; 
    strcpy(str, temp); 
}

char* getString(int fd)
{
    unsigned bufferSize = BLOCK_SIZE, curLength = 0;
    char *buffer = malloc(bufferSize);
    
    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation error!\n");
        exit(EXIT_FAILURE);
    }

    int ch = 0; //Required initialization!
    ssize_t bytesRead;

    while (1) {
        bytesRead = read(fd, &ch, 1); 
        if (bytesRead < 0) {
            free(buffer);
            close(fd);
            fprintf(stderr, "Read error!\n");
            exit(EXIT_FAILURE);
        }
        if (bytesRead == 0 || ch == '\n' || ch == EOF)
            break;

        if (curLength >= bufferSize - 1) {
            bufferSize += BLOCK_SIZE;
            char *newBuffer = realloc(buffer, bufferSize);
            if (newBuffer == NULL) {
                free(buffer);
                close(fd);
                fprintf(stderr, "Memory allocation error!\n");
                exit(EXIT_FAILURE);
            }
            buffer = newBuffer;
        }
        buffer[curLength++] = (char)ch;
    }

    if (curLength == 0 && bytesRead == 0) {
        free(buffer);
        return NULL;
    }

    buffer[curLength] = '\0';
    return buffer;
}