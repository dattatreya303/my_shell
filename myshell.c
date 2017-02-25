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

// returns the tokens as a double pointer, cmd becomes null
char** tokenize_string(char *cmd){

	char **tokens = (char **)malloc(CTOKS*sizeof(char *));
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
		// printf("%s %s\n", str2, subtoken);

	}

	return tokens;
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

void change_dir(char *to_path){

	int status = chdir(to_path);

	// chdir() call fails
	if( status == -1 ){

		printf("No file or directory: %s\n", to_path);

	}

}

// checks if string full has the given prefix
int startswith(char* full, char* prefix){

	int i;
	for(i=0; prefix[i] != '\0' && full[i] != '\0'; i++){

		if( prefix[i] != full[i] ){

			return 0;

		}

	}

	if( prefix[i] != '\0' ){

		return 0;

	}

	return 1;
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

		}

		else if( startswith(cmd, "cd") ){

			// extract path string from input
			char **args = tokenize_string(cmd);

			if( args[1] != NULL ){

				change_dir(args[1]);

			}

		}

		else{

			int pid = fork();
			
			// fork successful
			if(pid >= 0){

				// inside child process
				if(pid == 0){

					// printf("child process: %d\n", getpid());

					// tokenize cmd into command and options
					char **args = tokenize_string(cmd);
					
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

			}

			else{

				perror("fork");

			}

		}

	}

	return 0;

}