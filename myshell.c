/*
	* TODO:-
	* Implement 'cd'				DONE
	* Implement 'history'			DONE
	* Implement 'clear'				DONE
	* Implement 'kill'				DONE
	* Implement 'pwd'				DONE
	* Handle erroneous commands		DONE
	* Implement redirection 		DONE (only one operator, only system commands supported)
	* Implement piping 				DONE (only one pipe, only system commands supported)
	* Handle CTRL-C signal 			DONE
	* Handle RETURN signal			PARTIAL

	NB: Piping and redirection operators are handled separately, as of now. So, using them together is not supported.
*/

/*
	* Name: Dattatreya Mohapatra
	* Roll No.: 2015021
*/

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Name of file keeping record of user commands issued
#define HIST_FILE "/home/dattatreya/.history"

// Maximum characters in a path
#define PLEN 100

// Maximum characters in a command
#define CLEN 50

// Maximum tokens in a command
#define CTOKS 100

// Debug mode indicator
int DEBUG_MODE;

// Return key pressed
int RET_SIG;


// Returns the arrray of tokens (separated by 'symbol') as a double pointer
char** tokenize_string( char *cmd, char *symbol ){

	char **tokens = ( char ** )malloc( CTOKS*sizeof(char*) );
	char *str2, *subtoken;
	int j;

	// strtok implementation from man page
	for ( j = 0, str2 = cmd; ; str2 = NULL ) {

		subtoken = strtok( str2, symbol );
		if ( subtoken == NULL ){
			break;
		}
		tokens[j] = malloc( 50*sizeof(char) );
		strcpy( tokens[j++], subtoken );
		// printf("%s %s\n", str2, subtoken);

	}

	return tokens;
}

// Checks if string 'full' has the given prefix
int startswith( char* full, char* prefix ){

	int i;
	for ( i = 0; prefix[i] != '\0' && full[i] != '\0'; i++ ){

		if( prefix[i] != full[i] ){

			return 0;

		}

	}

	if( prefix[i] != '\0' ){

		return 0;

	}

	return 1;
}

/*
	* Strips the string of the 'symbol' at the both ends
	* Works like strip() in python
*/
char* strip_string(char *string, char symbol){

	int i;
	while( string[0] == symbol ){

		string = string + 1;

	}

	while( string[ (int)(strlen(string)) - 1 ] == symbol ){

		string[ (int)(strlen(string)) - 1 ] = '\0';

	}

	return string;

}

// Displays information regarding all the in-built functions in my_shell
void display_help(){

	printf("my_shell, author: Dattatreya Mohapatra\n");
	printf("The following commands are implemented internally in my_shell:-\n");
	printf("cd [path...]\n");
	printf("history [-c]\n");

}

// Returns the absolute path of current working directory
char* pres_working_dir(){

	char *pwd = ( char * )malloc( sizeof(char)*PLEN );
	getcwd( pwd, sizeof(char)*PLEN );
	return pwd;

}

// Prints prefix string for my_shell
void print_prefix(){

	printf( "user@myshell:%s$ ", pres_working_dir() );


}

void change_dir( char *to_path ){

	int status = chdir( to_path );

	// chdir() call fails
	if( status == -1 ){

		printf( "No file or directory: %s\n", to_path );

	}

}

/*
	* Checks if '>' or '<' or '|' exits in the command string
	* 0 - no redir
	* 1 - '>' (output redirection)
	* 2 - '<' (input redirection)
	* 3 - '|' (piping)
	* Stores position of operator in index
*/
int check_redir( char *cmd, int *index ){

	int i, op = 0;
	for( i = 0; cmd[i] != '\0'; i++ ){

		*index = i;

		switch(cmd[i]){
			case '>':	return 1;
			case '<':	return 2;
			case '|':	return 3;
		}

	}

	return 0;

}

/*
	* Redirects STDIN or STDOUT to filename specified in cmd
	* The filename is extracted from cmd using the index of redirection operator
	* Streams are redirected according to redir_mode (1 - '>', 2 - '<')
	* The path to file is trimmed of whitespace at the ends using strip_string().
*/
void prep_redirection( char *cmd, int index, int redir_mode ){

	switch(redir_mode){
		
		case 1:{

			char *filepath = cmd + index + 1;
			filepath = strip_string( filepath, ' ' );
			int fd_out = open( filepath, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR );
			if( dup2( fd_out, STDOUT_FILENO ) == -1 ){

				printf("Failed to duplicate file descriptors!\n");

			}
			close(fd_out);
			cmd[index] = '\0';
			cmd = strip_string( cmd, ' ' );
			break;

		}

		case 2:{

			char *filepath = cmd + index + 1;
			filepath = strip_string( filepath, ' ' );
			int fd_in = open( filepath, O_RDONLY );
			if( dup2( fd_in, STDIN_FILENO ) == -1 ){

				printf("Failed to duplicate file descriptors!\n");
				
			}
			close(fd_in);
			cmd[index] = '\0';
			cmd = strip_string( cmd, ' ' );
			break;

		}

	}

}

char** prep_piping( char *cmd ){

	char **arr = tokenize_string( cmd, "|" );
	int i;
	for( i = 0; arr[i] != NULL; i++ ){
		arr[i] = strip_string( arr[i], ' ' );
	}

	return arr;

}

int exec_with_piping( char **commands ){

	int noComs;
	for( noComs = 0; commands[noComs] != NULL; noComs++ );
	
	int pipe_fds[2];
	int pipe_fl = pipe(pipe_fds);
	if( pipe_fl == -1 ){

		printf("Failed to create pipe!\n");
		return -1;

	}

	int pid_1 = fork();
	if( pid_1 >= 0){

		if( pid_1 == 0 ){

			// Inside child process
			
			if( dup2( pipe_fds[0], STDIN_FILENO ) == -1 ){

				printf("Failed to duplicate file descriptors\n");
				return -1;

			}

			close(pipe_fds[0]);
			close(pipe_fds[1]);

			// Tokenize commands[1] into command and options
			char **args = tokenize_string( commands[1], " " );

			// Execute command with options
			return execvp( args[0], args );

		}
	}

	else{

		printf("Failed to fork()!\n");
		return -1;

	}


	int pid_0 = fork();
	if( pid_0 >= 0){

		if( pid_0 == 0 ){

			// Inside child process 0
			
			if( dup2( pipe_fds[1], STDOUT_FILENO ) == -1 ){

				printf("Failed to duplicate file descriptors\n");
				return -1;

			}

			close(pipe_fds[0]);
			close(pipe_fds[1]);

			// Tokenize commands[0] into command and options
			char **args = tokenize_string( commands[0], " " );

			// Execute command with options
			return execvp( args[0], args );

		}
	}

	else{

		printf("Failed to fork()!\n");
		return -1;
		
	}

	//  Close pipe as parent process does not need it anymore
	close(pipe_fds[0]);
	close(pipe_fds[1]);

	// wait for all the children to terminate
	int flag = 0;
	int status;
	while( wait(&status) > 0 ){

		// Handle erroneous command
		if( status == 65280 ){
			
			flag = -1;

		}

	}

	return flag;
}

void print_cmd_history(){

	FILE *fp = fopen( HIST_FILE, "r" );
	char *buffer = (char *)malloc( sizeof(char)*(CLEN+1) );

	int i = 0;
	while( fgets( buffer, CLEN, fp ) != NULL ){

		printf( "%d %s", i++, buffer );

	}

	fclose(fp);

}

void clear_cmd_history(){

	FILE *fp = fopen( HIST_FILE, "w" );
	fputs( "", fp );
	fclose(fp);

}

void append_cmd_history( char* cmd ){

	FILE *fp = fopen( HIST_FILE, "a" );
	fprintf(fp, "%s\n", cmd );
	fclose(fp);

}

/*
	* SIGINT signal handler
	* Ignore when CTRL-C is pressed
	* Child process is terminated, but my_shell is not affected
*/
void SIGINT_handler( int signum ){

	// print_prefix();

	return ;

}

int main( int argc, char const *argv[] ){

	// Register SIGINT handler to ignore CTRL-C
	signal( SIGINT, SIGINT_handler );

	// RETURN signal indicator
	RET_SIG = 0;

	while(1){

		print_prefix();
		
		// Command string
		char cmd[CLEN];
		fgets( cmd, CLEN, stdin );
		// Remove newline character
		strtok( cmd, "\n" );

		// Handle RETURN signal
		if( cmd[0] == '\n' ){

			RET_SIG = 1;

		}
		
		if( !RET_SIG ){

			// Append command to history file
			append_cmd_history(cmd);
			

			/*

				* Now we execute the user command, or throw an error if it is invalid.
				* If the user issues a system command, then we use execvp().
				* Else, it is implemented in the code itself.
				* Supported shell commands: exit, cd, history(basic)

			*/

			// Exit from myshell
			if( strcmp( cmd, "exit" ) == 0 ){

				return 0;

			}


			// Display help for all in-built commands
			else if( strcmp( cmd, "help" ) == 0 ){

				display_help();

			}

			else if( startswith( cmd, "cd" ) ){

				// Extract path string from input
				char **args = tokenize_string(cmd, " " );

				if( args[1] != NULL ){

					change_dir(args[1]);

				}

			} // 'cd' handler

			/*
				* history: Prints the etire history of commands issued by user
				* history -c: Prints and clears history
			*/
			else if ( startswith( cmd, "history" ) ){

				char **args = tokenize_string( cmd, " " );

				if( args[1] == NULL ){

					print_cmd_history();

				}	

				else if ( strcmp( args[1], "-c" ) == 0 ){

					clear_cmd_history();

				}

			} // 'history' handler

			// Handle system commands
			else{

				/*
					* Check for redirection operators (<, >, |)
					* 0 - no redirection operators or pipes
					* 1 - >
					* 2 - <
					* 3 - |
				*/
				int index = -1;
				int redir_mode = check_redir(cmd, &index);

				/*
					Handle piped commands separately, as
				 	we need to fork() multiple times.
				 */
				if( redir_mode == 3 ){

					// Create array of commands to be piped
					char **commands = prep_piping(cmd);
					// printf("%s %s\n", commands[0], commands[1]);
					
					// Handle commands in array with separate forks. 
					int st = exec_with_piping(commands);

					if( st < 0 ){

						printf("Something bad happened...\n");

					}

				}

				// Handle '<', '>' or no redirection ( only one fork needed )
				else{

					int pid = fork();
					
					// fork() successful
					if( pid >= 0 ){

						// Inside child process
						if( pid == 0 ){

							// printf("child process: %d\n", getpid());

							// Redirect I/O streams appropriately
							switch(redir_mode){

								case 1:
								case 2:	prep_redirection( cmd, index, redir_mode ); break;

							}

							// Tokenize cmd into command and options
							char **args = tokenize_string( cmd, " " );

							// Execute command with options
							return execvp( args[0], args );

						}

						// Inside parent process
						else{

							int status;
							
							// Receives status code from child process here
							wait(&status);

							// Handle erroneous command
							if( status == 65280 ){
								
								printf("%s: command not found\n", cmd);

							}

						}

					}

					else{

						perror("fork");

					}
					
				} // Handle '<', '>' or no redirection ( only one fork needed )

			} // handle system commands

		} // if ( !RET_SIG )

		RET_SIG = 0;

	} // while-loop ends

	return 0;

}