#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// maximum characters in a command
#define CLEN 50

// maximum tokens in a command
#define CTOKS 100

// debug-mode indicator
bool debug-mode;

void tokenize_string(char *cmd, char **tokens){

	char *str2, *subtoken;
	int j;

	// strtok implementation from man page
	for (j = 0, str2 = cmd; ; str2 = NULL) {

		subtoken = strtok(str2, " ");
		if(subtoken == NULL){
			break;
		}
		tokens[j] = malloc(50*sizeof(char));
		strcpy(tokens[j++], subtoken);
		// printf("**--> %s\n", subtoken);

	}
}

int main(int argc, char const *argv[]) {

	while(1){

		printf("user@my_shell> ");
		
		// command string
		char cmd[CLEN];
		fgets(cmd, CLEN, stdin);
		// remove newline character
		strtok(cmd, "\n");

		if(strcmp(cmd, "exit") == 0){

			return 0;

		}else{

			int pid = fork();
			
			// fork successful
			if(pid >= 0){

				// inside child process
				if(pid == 0){

					// printf("child process: %d\n", getpid());

					// tokenize cmd into command and options
					char **args = (char **)malloc(CTOKS*sizeof(char *));
					tokenize_string(cmd, args);
					
					// execute command with options
					return execvp(args[0], args);

				}

				// inside parent process
				else{

					int status;
					wait(&status);
					perror("status code");

				}

			}else{

				perror("fork");

			}
		}
	}

	return 0;

}