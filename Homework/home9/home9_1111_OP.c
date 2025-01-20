#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>

/*The task is realised with one pipe (OP) accece between processes.
Synchronization of calls to the pipe is organized using signals.
*/
/*Code cound correctly process any parametrs!*/
/*The main process generates (N-1) "childs" and begins data transmission. 
The receiver increases the number (<start>) by 1 and passes it on, 
and so on in a circle until the number exceeds the specified maximum (<end>).
Each process prints the current number and its own pid.
*/

volatile int semaphor_up = true, semaphor_down = true;
int N = 0, start = 0, end = 0;

void handler(int signum);
void analizeParametrs(int argc, char *argv[]);

int main(int argc, char **argv)
{
    analizeParametrs(argc, argv);

    int pipefd[2];
    if (pipe(pipefd) == -1){
        perror("[Error]: pipe creating.");
        exit(1);
    }

    signal(SIGUSR1, handler);
    int pids[N], value = start;
	int process_num = -1;
	pids[0] = getpid();
    for (int i = 1; i < N; i++) 
    {
		pid_t pid = fork();
        if (pid == -1){
            perror("[Error]: creating fork.");
            break;
        } else if (pid == 0) { //Child process
			process_num = i;
			pids[i] = pid;
			if (i == N - 1) {
                int current_number = N - (process_num );
                printf("Process: %d, pid: %d, received value: %d\n", current_number , getpid(), value);
				value++;
				write(pipefd[1], &value, sizeof(int));
				usleep(100); 
				kill(pids[i - 1], SIGUSR1);
			}
            break;
        } 
        pids[i] = pid; 
    }
	if (process_num > -1)
    {
		usleep(50);
		while (1) 
		{
			pause();
			read(pipefd[0], &value, sizeof(int));
			if (value > end) {
				write(pipefd[1], &value, sizeof(int)); 
				kill(pids[0], SIGUSR1);
				close(pipefd[0]);
				close(pipefd[1]);
				exit(0);
			}

            int current_number = N - (process_num );
            printf("Process: %d, pid: %d, received value: %d\n", current_number, getpid(), value);
			value++;
			write(pipefd[1], &value, sizeof(int)); 
			kill(pids[process_num - 1], SIGUSR1); 
			usleep(10);
		}
    }
    else {
		while (1) {
			pause();
			read(pipefd[0], &value, sizeof(int));
			if (value > end) {
				for (int i = 1; i < N; i++) {
					write(pipefd[i], &value, sizeof(int));
					kill(pids[i], SIGUSR1);
				}
				break;
			}

            int current_number = N;
            printf("Process: %d, pid: %d, received value: %d\n", current_number, getpid(), value);
			value++;
			write(pipefd[1], &value, sizeof(int)); 
			usleep(50);
			kill(pids[N - 1], SIGUSR1); 
		}
		for (int i = 1; i < N; i++){ //Waiting for child processes to end
			wait(0);
			close(pipefd[0]);
            close(pipefd[1]);
            exit(0);			
		}
	}
    return 0;
}

void handler(int signum) {

}

void analizeParametrs(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <N> <start> <end>\n", argv[0]);
        exit(0);
    }
    char *endptr = NULL;
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
    if (N < 2 || start > end) {
        fprintf(stderr, "Incorrect parametrs:\n1) number of processes must be > 1\n2) The \"start\" must be >= than \"end\"\n3) All parametrs are integers.\n");
        exit(1);
    }
}