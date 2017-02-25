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

// maximum characters in a path
#define PLEN 100

// debug-mode indicator
int debug_mode;

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

// returns the absolute path of current working directory
char* pres_working_dir(){

	char *pwd = (char *)malloc(sizeof(char)*PLEN);
	getcwd(pwd, sizeof(char)*PLEN);
	return pwd;

}

// prints prefix string for my_shell
void print_prefix(){

	printf("user@myshell:%s$ ", pres_working_dir());

}

int main(int argc, char const *argv[]) {

	while(1){

		print_prefix();
		
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
					execvp(args[0], args);

					// exit with status code of execvp() call
					exit(errno);

				}

				// inside parent process
				else{

					int status;
					// receives status code from child process here
					wait(&status);

				}

			}else{

				perror("fork");

			}
		}
	}

	return 0;

}