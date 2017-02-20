#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define CLEN 50 // max command length

int main(int argc, char const *argv[]) {
	while(1){
		printf("user@my_shell> ");
		char cmd[CLEN];
		fgets(cmd, CLEN, stdin);
		strtok(cmd, "\n");

		if(strcmp(cmd, "exit") == 0){
			return 0;
		}
		else{
			int pid = fork();
			if(pid >= 0){
				if(pid == 0){
					// printf("child process: %d\n", getpid());
					char **args = (char **)malloc(100*sizeof(char *));
					char *str2, *subtoken;
					int j;
					for (j = 0, str2 = cmd; ; str2 = NULL) {
						subtoken = strtok(str2, " ");
						if(subtoken == NULL){
							break;
						}
						args[j] = malloc(50*sizeof(char));
						strcpy(args[j++],subtoken);
						// printf("**--> %s\n", subtoken);
					}
					
					return execvp(args[0], args);
				}
				else{
					int status;
					wait(&status);
					perror("status code");
				}
			}
			else{
				perror("fork");
			}
		}
	}
	return 0;
}