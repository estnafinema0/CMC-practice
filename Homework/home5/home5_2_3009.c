#include <stdio.h>
#include <termios.h>
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>


/*Kоманла uniq будет рассматривать как 'неуникальные' - подряд идущие одинковые строки.
Программа реализована с помощью низкоуровневых функций работы с файлом.
Обрабатывает ошибки ввода параметров.
*/

enum {c_default = 0, d_default = 0, u_default = 0, s_default = 0, s_num_default = 0};
enum {BLOCK_SIZE = 4};

int global_count = 1;

int analizeParametrs(int argc, char *argv[], int *c, int *d, int *u, int *s, int *s_num);
char* getString(int fd, int *cur_length);
int compare_with_offset(const char *str1, const char *str2, int offset);
int myUniq(int fd, const int c, const int d, const int u, const int s, const int s_num);

int main(int argc, char** argv) 
{	
	int c = c_default, d = d_default, u = u_default, s = s_default, s_num = s_num_default;
    int count = analizeParametrs(argc, argv, &c, &d, &u, &s, &s_num);
		
	if (argv[count] == NULL) {
		perror("Error: no filename is given.\n");
		exit(1);
	}
	
	int fd = open(argv[count], O_RDONLY);
	if (fd == -1) {
		fprintf(stderr, "Error: can not open the given file: %s.\n", argv[count]);
		perror("Opening fd.");
		exit(1);
	}
	
	//printf("Executing given file: %s\n", argv[count]);
	myUniq(fd, c, d, u, s, s_num);
	close(fd);
    return 0;
}

int analizeParametrs(int argc, char *argv[], int *c, int *d, int *u, int *s, int *s_num)
{
	if (argc < 2){
		perror("[ERROR]: no required file to execute.\n");
		exit(1);
	}
	if (argc > 7){
		perror("[ERROR]: too much arguments to execute.\n");
		exit(1);
	}
	
	int count_parametrs = 1, is_file_name = 0;
    char *endptr; 
    while (!is_file_name && count_parametrs < 6) {
		if (argv[count_parametrs][0] == '-' && strlen(argv[count_parametrs]) == 2) {
			switch (argv[count_parametrs][1]) {
				case 'd':
					*d = 1;
					break;
				case 'u':
					*u = 1;
					break;
				case 'c':
					*c = 1; 
					break;
				case 's':
					*s = 1;
					if (count_parametrs + 1 >= argc) {
                        fprintf(stderr, "[ERROR]: No number provided after parameter \"-s\".\n");
                        exit(1);
                    }
					errno = 0;
					*s_num = (int)strtol(argv[++count_parametrs], &endptr, 10);
					if (*s_num < 0) {
						printf("[ERROR]: Invalid number %d of bytes to skip.\n", *s_num);
						exit(1);
					}
					if (*endptr != '\0' || errno != 0) {
						perror("[ERROR]: Invalid number after parametr \"-s\" id provided.\n");
						exit(1);
					}
					break;
				case '\0': 
					fprintf(stderr, "[ERROR]: No parametr after '-' is provided.\n");
					exit(1);
				default:
					fprintf(stderr, "[ERROR]: Invalid flag is provided.\n");
					exit(1);
			}
			count_parametrs++;
		} else is_file_name = 1;
	}
	return count_parametrs;
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

int compare_with_offset(const char *str1, const char *str2, int offset) 
{
    if (offset >= strlen(str1)) str1 = "";
	else str1 += offset;

    if (offset >= strlen(str2)) str2 = "";
	else str2 += offset;

    return strcmp(str1, str2);
}

int myUniq(int fd, const int c, const int d, int u, const int s, int s_num) 
{
    if (d && u) {
        fputs("Flags \"-u\" and \"-d\" provided.\nNo lines to print.\n", stdout);
        return 0;
    }
    
    int c_current = 1, length_string;
    char *buffer = NULL, *buffer_prev = getString(fd, &length_string);
    if (buffer_prev == NULL) {
        return 0; 
    }
    	//printf("Inputted paramets: c = %d, d = %d, u = %d, s = %d, s_num = %d.\n", c, d, u, s, s_num);

    while ((buffer = getString(fd, &length_string)) != NULL) {
        if (compare_with_offset(buffer_prev, buffer, s_num) == 0) {
            c_current++;
        } else {
            if ((!d && !u) || (c_current > 1 && d) || (c_current == 1 && u)) {
                if (c) {
                    printf("%7d ", c_current);
                }
                printf("%s\n", buffer_prev);
            }
            free(buffer_prev);
            buffer_prev = strdup(buffer);
            c_current = 1;
        }
        free(buffer);
    }
	
    if (buffer_prev != NULL) {
        if ((!d && !u) || (c_current > 1 && d) || (c_current == 1 && u)) {
            if (c) {
                printf("%7d ", c_current);
            }
            printf("%s\n", buffer_prev);
        } 
        free(buffer_prev);
    }

    return 1;
}
