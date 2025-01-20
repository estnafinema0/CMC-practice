#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>

/*Задачи 1, 2 сданы на семинаре.
РЕзультат задачи 3 сохраняется в новом созданном файле "results3".
Интерактивный интерфейс взаимодействия с программой в терминале.
*/
enum {BUFFER_SIZE = 10};

void task3( const char *filename_bin) {
    int fd = open(filename_bin, O_RDWR);
    if (fd == -1) {
        perror("Cannot open the given binary file");
        exit(1);
    }

    int32_t buffer[BUFFER_SIZE];
    int neg_count = 0, zero_count = 0, pos_count = 0;
    off_t f_length = lseek(fd, 0, SEEK_END);
    if (f_length == 0) {
        printf("File is empty.\n");
        close(fd);
        exit(0);
    }

    lseek(fd, 0, SEEK_SET);
    while (read(fd, buffer, BUFFER_SIZE * sizeof(int32_t))) {
        for (int i = 0; i < BUFFER_SIZE; i++) {
            if (buffer[i] < 0) neg_count++;
            else if (buffer[i] == 0) zero_count++;
            else pos_count++;
        }
    }
	
	
    int num_count = f_length / sizeof(int32_t);
    lseek(fd, 0, SEEK_SET);

	for (int i = 0; i < neg_count; i++) {
        lseek(fd, i * sizeof(int32_t), SEEK_SET);
        int32_t num;
        read(fd, &num, sizeof(int32_t));
        if (num > 0) {
            for (int j = num_count - 1; j >= neg_count; j--) {
                lseek(fd, j * sizeof(int32_t), SEEK_SET);
                int32_t num2;
                read(fd, &num2, sizeof(int32_t));
                if (num2 <= 0) {
                    lseek(fd, j * sizeof(int32_t), SEEK_SET);
                    write(fd, &num, sizeof(int32_t));

                    lseek(fd, i * sizeof(int32_t), SEEK_SET);
                    write(fd, &num2, sizeof(int32_t));
                    break;
                }
            }
        }
    }

    for (int i = 0; i < neg_count + zero_count; i++) {
        lseek(fd, i * sizeof(int32_t), SEEK_SET);
        int32_t num;
        read(fd, &num, sizeof(int32_t));
        if (num == 0) {
            for (int j = num_count - 1; j >= neg_count + zero_count; j--) {
                lseek(fd, j * sizeof(int32_t), SEEK_SET);
                int32_t num2;
                read(fd, &num2, sizeof(int32_t));
                if (num2 < 0) {
                    lseek(fd, j * sizeof(int32_t), SEEK_SET);
                    write(fd, &num, sizeof(int32_t));

                    lseek(fd, i * sizeof(int32_t), SEEK_SET);
                    write(fd, &num2, sizeof(int32_t));
                    break;
                }
            }
        }
    }
    
    close(fd);
}
void createBinaryFile(const char * filename_bin, int N) {
	
    int fd = open(filename_bin, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (fd == -1) {
        perror("Cannot open the given first file");
        exit(1);
    }
    srand(time(NULL)); 

    for (int i = 0; i < N; i++) {
        int num = rand() % 30 - 15; 

        if (write(fd, &num, sizeof(int32_t)) != sizeof(int32_t)) {
            perror("Error writing the binary file.");
            close(fd);
            exit(1);
        }
    }
    close(fd);
}

void convertToText(const char *filename_bin, const char *filename_txt) {
    int fd_binary = open(filename_bin, O_RDONLY);
    if (fd_binary == -1) {
        perror("Cannot open the given binary file");
        exit(1);
    }

    int fd_txt = open(filename_txt, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd_txt == -1) {
        printf("The file %s already exist. Do you still want to continue? (y/n): ", filename_txt);
        char answer = getchar();
        if (answer !='y') {
            close(fd_binary);
            exit(0);
        }
        
            while (getchar() != '\n');
        fd_txt = open(filename_txt, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    }

    int number;
    char buffer[12];

    while (read(fd_binary, &number, sizeof(int32_t)) == sizeof(int32_t)) {
        int is_num_written = snprintf(buffer, sizeof(buffer), "%d ", number);
        if (write(fd_txt, buffer, is_num_written) != is_num_written) {
            perror("Error writing the binary file.");
            close(fd_binary);
            close(fd_txt);
            exit(1);
        }
    }

    close(fd_binary);
    close(fd_txt);
}



int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "The correct input: %s <binary filename> <number N> <text filename>\n", argv[0]);
        return 1;
    }
    char *endptr;
	errno = 0;
	int N = (int)strtol(argv[2], &endptr, 10);
		if (endptr == argv[2] || (errno != 0)) {
			perror("[ERROR]: Failed to convert to int the parametr N.\n");
			exit(1);
		}

    createBinaryFile(argv[1], N);
    puts("Task 1 is done.");
    
	convertToText(argv[1], argv[3]);
    puts("Task 2 is done.");
    
    printf("Do you want to \"sort\" binary file %s? (y/n): ", argv[1]);
    char answer = getchar();
    if (answer !='y') 
		return 0;
    
    while (getchar() != '\n');
    task3(argv[1]);
    puts("The binary file is \"sorted\".\nTask 3 is done.");
    convertToText(argv[1], "result3");
    puts("The sorted binary file is converted to text.\nThe result in the file \"result3\"");
    return 0;
}