#include <stdio.h>
#include <termios.h>
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/*Работа завершается нажатием клавиши 'q', продолжение вывода - нажатие ' '. 
При введении пользователем других символов программа выведет напоминание о функционале.
При введении пробела между выводимыми блоками не печатается пустая строка.
Вводимые пользователем клавили после запуска программы не выводятся на экран.
*/
/*
Добавлена обработка неверно переданных параметром в командной строке.
*/

enum {skip_activation = 0, default_lines = 10, default_linenum = 1};
enum {BLOCK_SIZE = 4};

int global_count = 1;

int analizeParametrs(int argc, char *argv[], int *skip_lines, int *num, int *linenum);
char* getString(FILE *file, int *cur_length);
int printLines(const int skip_lines, const int num, const int linenum, FILE *file);
void runTheScript(const int skip_lines, const int num, const int linenum, FILE *file);

int main(int argc, char** argv) 
{	
	int skip_lines = skip_activation, num = default_lines, linenum = default_linenum;
    int count = analizeParametrs(argc, argv, &skip_lines, &num, &linenum);
		
	if (argv[count] == NULL) {
		perror("Error: no filename is given.\n");
		exit(1);
	}
	
	FILE *file = fopen(argv[count], "r");
	if (file == NULL) {
		fprintf(stderr, "Error: can not open the given file: %s.\n", argv[count]);
		perror("fopen our_file");
		exit(1);
	}
	
	printf("Executing given file: %s\n", argv[count]);
	runTheScript(skip_lines, num, linenum, file);
	fclose(file);
    return 0;
}

int analizeParametrs(int argc, char *argv[], int * skip_lines, int * num, int * linenum)
{
	if (argc < 2){
		perror("[ERROR]: no required file to execute.\n");
		exit(1);
	}
	if (argc > 5){
		perror("[ERROR]: too much arguments to execute.\n");
		exit(1);
	}
	
	int count_parametrs = 1;
    char *endptr; 
    
	if (strcmp(argv[count_parametrs], "-s") == 0){
		*skip_lines = 1;
		count_parametrs++;
	}
	if (argv[count_parametrs][0] == '-'){
		errno = 0;
		*num = (int)strtol(argv[count_parametrs] + 1, &endptr, 10);
		if (endptr == (argv[count_parametrs] + 1) || (errno != 0)) {
			perror("[ERROR]: Failed to convert to int the parametr\n");
			exit(1);
		}
		if (*num == 0){
			fprintf(stderr, "[ERROR]: paramentr [num] can not be equal zero\n");
			exit(1);
		}
		count_parametrs++;
	}
	if (argv[count_parametrs][0] == '+'){
		errno = 0;
		*linenum = (int)strtol(argv[count_parametrs] + 1, &endptr, 10);
		if (endptr == (argv[count_parametrs] + 1) || (errno != 0 && *linenum == 0)) {
			perror("[ERROR]: Failed to convert to int the parametr\n");
			exit(1);
		}
		count_parametrs++;	
	}	
	return count_parametrs;
}

char* getString(FILE* file, int* cur_length) {
    char ch, *buffer;
    buffer = (char *)malloc(sizeof(char) * BLOCK_SIZE); 
    if (buffer == NULL) {
        perror("[ERROR]: memory allocation failed.\n");
        exit(1);
    } 
    *cur_length = 0;
    int buffer_size = BLOCK_SIZE;

    while ((ch = fgetc(file)) != EOF && ch != '\n') {
        if (*cur_length >= buffer_size - 1) {
            buffer_size += BLOCK_SIZE;
            buffer = (char *)realloc(buffer, sizeof(char) * buffer_size);
            if (buffer == NULL) {
                printf("Error: memory allocation for buffer failed!\n");
                free(buffer); 
                exit(1); 
            }
        }
        buffer[*cur_length] = ch;
        (*cur_length)++;
    }

    if (*cur_length == 0 && ch == EOF) {
        free(buffer);
        return NULL; 
    }

    buffer[*cur_length] = '\0';

    return buffer;
}

int printLines(const int skip_lines, const int num, const int linenum, FILE * file) 
{	
	int printed_lines = 0, is_prev_empty = 0, cur_length;
	char *buffer;

    while (global_count < linenum - 1 && (buffer = getString(file, &cur_length))){
		global_count++;
		free(buffer);
	}
	while (printed_lines < num && (buffer = getString(file, &cur_length))) {
		if (skip_lines && (cur_length == 0) && is_prev_empty) {
			free(buffer);
			continue;
		}
		printf("%s\n", buffer);
		free(buffer);
		printed_lines++;
		is_prev_empty = (cur_length == 0);
	}

	if (buffer == NULL){
        printf("Attention: End of file.\n");
        return 0;
    }
	return 7;
}

void runTheScript(const int skip_lines, const int num, const int linenum, FILE * file)
{
	static struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt); // сохранение текущего режима
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO); //ECHO: the characters typed by the user are not shown on the terminal 
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // изменение
    
    char c = ' ';
	while (c != 'q') {
		if (c == ' '){
			//putchar('\n'); // If you an empty line before each prints of line
			if (printLines(skip_lines, num, linenum, file) == 0) break;	
		} else puts("Enter space to continue or 'q' to quit.");
		c = getchar();
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // восстановление исходного режима
}
