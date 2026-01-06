#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int pipefd[2];


int main(){
	if(pipe(pipefd) == -1){
		perror("Pipe Failed\n");
		return 1;
	}

	pid_t pid1;
	pid1 = fork();
	if(pid1 == -1){
		perror("Child 1 Fork Failed\n");
		close(pipefd[0]);
		close(pipefd[1]);
		exit(1);

	}else if(pid1 == 0){
		//Child 1
		close(pipefd[0]);
		if(dup2(pipefd[1], STDOUT_FILENO) == -1){
			exit(1); //dup failed
		}else{
			close(pipefd[1]);
			execlp("ls", "ls", NULL);

			exit(2); //exec failed
		}
	}else{
		//Parent
		pid_t pid2;
		pid2 = fork();
		if(pid2 == -1){
			perror("Child 2 Fork Failed\n");
			close(pipefd[0]);
			close(pipefd[1]);
			wait(NULL);
			exit(1);

		}else if(pid2 == 0){
			//Child 2
			close(pipefd[1]);
			int outfd = creat("outfile", 0777);
			if(outfd == -1){
				exit(3); // file creation failed
			}else{
				if(dup2(pipefd[0], STDIN_FILENO) == -1){
					exit(1);
				}
				if(dup2(outfd, STDOUT_FILENO) == -1){
					exit(1);
				}

				close(pipefd[0]);
				execlp("sort", "sort", "-r", NULL);

				exit(2); //exec failed
			}

		}else{
			close(pipefd[0]);
			close(pipefd[1]);
			//Parent

			int wstatus;
			int waiting = 2;
			
			while(waiting){
				if(waitpid(pid1, &wstatus, WNOHANG)){
					if(WIFEXITED(wstatus)){
						switch(WEXITSTATUS(wstatus)){
							case 1:
								perror("Child 1 dup failed");
								break;
							case 2:
								perror("Child 1 exec failed");
								break;
						}
					}
					waiting--;		
				}
				if(waitpid(pid2, &wstatus, WNOHANG)){
					if(WIFEXITED(wstatus)){
						switch(WEXITSTATUS(wstatus)){
							case 1:
								perror("Child 2 dup failed");
								break;
							case 2:
								perror("Child 2 exec failed");
								break;
							case 3:
								perror("Child 2 file creation failed");
								break;
						}
					}
					waiting--;
				}
			}			
		}
	}
	exit(0);
}

