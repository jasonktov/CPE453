#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int pipefd[2];

int main(){
	if(pipe(pipefd) == -1){
		printf("Pipe Failed\n");
		return 1;
	}

	pid_t pid1;
	pid1 = fork();
	if(pid1 == -1){
		printf("Child 1 Fork Failed\n");
		close(pipefd[0]);
		close(pipefd[1]);
		exit(1);

	}else if(pid1 == 0){
		//Child 1
		close(pipefd[0]);
		if(dup2(pipefd[1], STDOUT_FILENO) == -1){
			printf("Child 1 dup failed\n");
			exit(1); 
		}else{
			close(pipefd[1]);
			execlp("ls", "ls", NULL);

			printf("Child 1 exec failed\n");
			exit(2); 
		}
	}else{
		//Parent
		pid_t pid2;
		pid2 = fork();
		if(pid2 == -1){
			printf("Child 2 Fork Failed\n");
			close(pipefd[0]);
			close(pipefd[1]);

			int wstatus;
			waitpid(pid1, &wstatus, 0);
			if(WIFEXITED(wstatus)){
				printf("Child 1 exited with exit code %d\n", WEXITSTATUS(wstatus));
				pid1 = 0;
			}	
			exit(1);

		}else if(pid2 == 0){
			//Child 2
			close(pipefd[1]);
			int outfd = creat("outfile", 0777);
			if(outfd == -1){
				printf("Child 2 file creation failed\n");
				exit(3);
			}else{
				if(dup2(pipefd[0], STDIN_FILENO) == -1){
					printf("Child 2 dup failed\n");
					exit(1);
				}
				if(dup2(outfd, STDOUT_FILENO) == -1){
					printf("Child 2 dup failed\n");
					exit(1);
				}

				close(pipefd[0]);
				execlp("sort", "sort", "-r", NULL);

				printf("Child 2 exec failed\n");
				exit(2);
			}

		}else{
			//Parent
			close(pipefd[0]);
			close(pipefd[1]);
			
			int wstatus;
			while(pid1 | pid2){
				if(pid1 && waitpid(pid1, &wstatus, WNOHANG)){
					if(WIFEXITED(wstatus)){
						printf("Child 1 exited with exit code %d\n", WEXITSTATUS(wstatus));
						pid1 = 0;
					}	
				}
				if(pid2 && waitpid(pid2, &wstatus, WNOHANG)){
					if(WIFEXITED(wstatus)){
						printf("Child 2 exited with exit code %d\n", WEXITSTATUS(wstatus));
						pid2 = 0;
					}	
				}
			}			
		}
	}
	exit(0);
}

