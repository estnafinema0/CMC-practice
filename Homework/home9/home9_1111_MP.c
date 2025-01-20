#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

/*The task is realised with many pipes (MP) acceces between processes.*/

/*Code cound correctly process any parametrs!*/
/*The main process generates (N-1) "childs" and begins data transmission. 
The receiver increases the number (<start>) by 1 and passes it on, 
and so on in a circle until the number exceeds the specified maximum (<end>).
Each process prints the current number and its own pid.
*/

volatile int N = 0;
int start = 0, end = 0;

void createPipes(int pipes[][2], int N);
void closePipes(int pipes[][2], int N);
void analizeParametrs(int argc, char *argv[]);

int main(int argc, char **argv)
{
    analizeParametrs(argc, argv);
    
    int pipes[N][2];
    createPipes(pipes, N);
    
    for (int i = 0; i < N; i++){
        pid_t pid = fork();
        if (pid == -1){
            perror("[Error]: creating fork.");
            break;
        } else if (pid == 0) {
            int value;
            for (int j = 0; j < N; j++){
                if (j != i) close(pipes[j][0]);
                if (j != (i + 1) % N) close(pipes[j][1]);
            }
            while(1) {
                if (read(pipes[i][0], &value, sizeof(value)) <= 0) break;
                if (value > end) break;
                printf("Process: %d, pid: %d, received value: %d\n", i+1, getpid(), value);
                value++;
                write(pipes[(i+1)%N][1], &value, sizeof(value));
                if (value > end) break;
            }
            close(pipes[i][0]);
            close(pipes[(i + 1) % N][1]);
            exit(0);
        }
    }
    write(pipes[0][1], &start, sizeof(start));
    closePipes(pipes, N);
    for (int i = 0; i < N; i++) wait(NULL);
    return 0; 
}

void createPipes(int pipes[][2], int N)
{
    for (int i = 0; i < N; i++)
        if (pipe(pipes[i]) == -1){
            perror("[Error]: pipe creating.");
            exit(1);
        }
}

void closePipes(int pipes[][2], int N)
{
    for (int i = 0; i < N; i++){
        close(pipes[i][0]);
        close(pipes[i][1]);
    }
}

void analizeParametrs(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <N> <start> <end>\n", argv[0]);
        exit(0);
    }
    char* endptr = NULL;
    N = (int)strtol(argv[1], &endptr, 10);
    if (endptr == argv[1] || *endptr != '\0') {
        fprintf(stderr, "Invalid number <N> in parametrs\n");
        exit(1);
    }
    start = (int)strtol(argv[2], &endptr, 10);
    if (endptr == argv[2] || *endptr != '\0') {
        fprintf(stderr, "Invalid number <start> in parametrs\n");
        exit(1);
    }
    end = (int)strtol(argv[3], &endptr, 10); 
    if (endptr == argv[3] || *endptr != '\0') {
        fprintf(stderr, "Invalid number <end> in parametrs\n");
        exit(1);
    }
	if (N < 1 || start > end) {
		fprintf(stderr, "Incorrect parametrs:\n1) Number of processes must be > 0\n2) The \"start\" must be >= than \"end\"\n3) All parametrs are integers.\n");
		exit(1);
	}
}