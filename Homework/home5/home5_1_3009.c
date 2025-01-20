#include <stdio.h>
#include <termios.h>
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

enum {BLOCK_SIZE = 4};

char* getString(int fd, int * cur_length);
void deleteNStrings(int fd, int N);
int main(int argc, char** argv) 
{   
    if (argc < 3) {
        fprintf(stderr, "Error: no filename or parametr N is given.\n");
        exit(1);
    }
    
    int fd = open(argv[1], O_RDWR);
    
    if (fd == -1) {
        printf("Error: can not open the given file: %s.\n", argv[1]);
        perror("[ERROR]: Failed to open file.");
        exit(1);
    }
    char * endptr;
    errno = 0;
    int N = (int)strtol(argv[2], &endptr, 10);
    if (errno != 0) 
        perror("[ERROR]: Entered not a valid integer.");
    
    printf("Executing given file: %s\n", argv[1]);
    deleteNStrings(fd, N);
    puts("THe process has done successfully.");
    
    close(fd);
    
    return 0;
}

char* getString(int fd, int * cur_length)
{
    char ch, *buffer;
    buffer = (char *)malloc(sizeof(char) * BLOCK_SIZE); 
    if (buffer == NULL) {
        perror("[ERROR]: memory allocation failed.\n");
        exit(1);
    } 
    *cur_length = 0;
    int buffer_size = BLOCK_SIZE;
    int read_result;
    
    while(((read_result = read(fd, &ch, 1)) == 1) && ch != '\n'){ // read returns 0 at EOF
        if (*cur_length >= buffer_size - 1) {
            buffer_size += BLOCK_SIZE;
            buffer = (char *)realloc(buffer, sizeof(char) * buffer_size);
            if (buffer == NULL){
                perror("Error: memory allocation for buffer failed!\n");
                free(buffer); 
                exit(1); 
            }
        }
        buffer[*cur_length] = ch;
        (*cur_length)++;
    }
	if (read_result == 0 && *cur_length == 0) {
        free(buffer);
        return NULL; 
    }
    if (*cur_length == 0 && read_result == 1 && ch == '\n') {
        buffer[0] = '\0';
        return buffer;
    }

    buffer[*cur_length] = '\0';

    return buffer;
}


void deleteNStrings(int fd, int N)
{
	int cur_length = 0;
	char *buffer;
	int fd_tmp = open("/tmp", O_TMPFILE | O_RDWR, 0x666);
	if (fd_tmp == -1) {
		perror("Error opening temporary file");
		exit(1);
	}
	
    lseek(fd, 0, SEEK_SET);
    
	while((buffer = getString(fd, &cur_length)) != NULL){ //file descripter of stdin is STDIN_FILENO
		if (cur_length <= N){
			write(fd_tmp, buffer, strlen(buffer));	
			write(fd_tmp, "\n", 1); 
		}
		free(buffer);		
	}
	
    lseek(fd_tmp, 0, SEEK_SET);
    ftruncate(fd, 0);
    if (ftruncate(fd, 0) == -1) {
        perror("[EROOR]: error truncating file");
        close(fd_tmp);
	    close(fd);
        exit(1);
    }
    lseek(fd, 0, SEEK_SET);
    
	while ((buffer = getString(fd_tmp, &cur_length)) != NULL){
		write(fd, buffer, strlen(buffer));	
		write(fd, "\n", 1); 
		free(buffer);
	}
	close(fd_tmp);
	
}

//compiled with gcc -g -Wall -std=c99 -D_GNU_SOURCE ...
//valgrind --leak-version=full ...
