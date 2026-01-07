#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int pipefd[2];

int main(){
	if(pipe(pipefd) == -1){ /* Initialize Pipe */
		printf("Pipe Failed\n");
		return 1;
	}

	pid_t pid1;
	pid1 = fork(); /* Fork Child 1 */
	if(pid1 == -1){
		//Fork Failed
		printf("Child 1 Fork Failed\n");
		close(pipefd[0]);
		close(pipefd[1]);
		exit(1);

	}else if(pid1 == 0){
		//Child 1
		close(pipefd[0]);
		if(dup2(pipefd[1], STDOUT_FILENO) == -1){ /* Redirect stdout to pipe */
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
		pid2 = fork(); /* Fork Child 2 */
		if(pid2 == -1){
			//Fork Failed
			printf("Child 2 Fork Failed\n");
			close(pipefd[0]);
			close(pipefd[1]);

			int wstatus;
			waitpid(pid1, &wstatus, 0); /* Wait for Child 1 */
			if(WIFEXITED(wstatus)){
				printf("Child 1 exited with exit code %d\n", WEXITSTATUS(wstatus));
			}	
			exit(1);

		}else if(pid2 == 0){
			//Child 2
			close(pipefd[1]);
			int outfd = creat("outfile", 0777); /* Create output file */
			if(outfd == -1){
				printf("Child 2 file creation failed\n");
				exit(3);
			}else{
				if(dup2(pipefd[0], STDIN_FILENO) == -1){ /* Redirect stdin to pipe */
					printf("Child 2 dup failed\n");
					exit(1);
				}
				if(dup2(outfd, STDOUT_FILENO) == -1){ /* Redirect stdout to output file */
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
			int exit_status = 0;
			while(pid1 | pid2){ /* Non-blocking wait for both children */
				if(pid1 && waitpid(pid1, &wstatus, WNOHANG)){
					if(WIFEXITED(wstatus)){
						printf("Child 1 exited with exit code %d\n", WEXITSTATUS(wstatus));
						pid1 = 0;
						if(wstatus != 0){ /* If either child had non-zero exit status, exit main with exit status 1*/
							exit_status = 1;
						}
					}	
				}
				if(pid2 && waitpid(pid2, &wstatus, WNOHANG)){
					if(WIFEXITED(wstatus)){
						printf("Child 2 exited with exit code %d\n", WEXITSTATUS(wstatus));
						pid2 = 0;
						if(wstatus != 0){
							exit_status = 1;
						}
					}	
				}
			}		
			exit(exit_status);	
		}
	}
	exit(1);
}

