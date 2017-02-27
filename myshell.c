/*
	* TODO:-
	* Implement 'cd'				DONE
	* Implement 'history'			DONE (have to place .history in HOME)
	* Implement 'clear'				DONE
	* Handle erroneous commands		DONE
	* Implement 'kill'
	* Handle RETURN signal
	* Handle CTRL-C signal
	* Implement piping
	* Implement redirection operators
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

// CTRL-C signal sent
int CTRL_C_SIG;


// Returns the tokens as a double pointer
char** tokenize_string( char *cmd ){

	char **tokens = ( char ** )malloc( CTOKS*sizeof(char*) );
	char *str2, *subtoken;
	int j;

	// strtok implementation from man page
	for ( j = 0, str2 = cmd; ; str2 = NULL ) {

		subtoken = strtok( str2, " " );
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

void prep_redirection( int redir_mode, int index, char *cmd ){

	switch(redir_mode){
		
		case 1:{

			char *filepath = cmd + index + 1;
			filepath = strip_string( filepath, ' ' );
			int fd_out = open( filepath, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR );
			dup2( fd_out, STDOUT_FILENO );
			cmd[index] = '\0';
			cmd = strip_string( cmd, ' ' );
			break;

		}

		case 2:{

			char *filepath = cmd + index + 1;
			filepath = strip_string( filepath, ' ' );
			int fd_in = open( filepath, O_RDONLY );
			dup2( fd_in, STDIN_FILENO );
			cmd[index] = '\0';
			cmd = strip_string( cmd, ' ' );
			break;

		}

	}

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

void append_cmd_history(char* cmd){

	FILE *fp = fopen( HIST_FILE, "a" );
	fprintf(fp, "%s\n", cmd );
	fclose(fp);

}

int main( int argc, char const *argv[] ){

	RET_SIG = 0;
	CTRL_C_SIG = 0;

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

			// Exit form myshell
			if( strcmp( cmd, "exit" ) == 0 ){

				return 0;

			}

			else if( startswith( cmd, "cd" ) ){

				// Extract path string from input
				char **args = tokenize_string(cmd);

				if( args[1] != NULL ){

					change_dir(args[1]);

				}

			} // 'cd' handler

			/*
				* history: Prints the etire history of commands issued by user
				* history -c: Prints and clears history
			*/
			else if ( startswith( cmd, "history" ) ){

				char **args = tokenize_string(cmd);

				if( args[1] == NULL ){

					print_cmd_history();

				}	

				else if ( strcmp( args[1], "-c" ) == 0 ){

					clear_cmd_history();

				}

			} // 'history' handler

			else{

				int pid = fork();
				
				// fork() successful
				if( pid >= 0 ){

					// Inside child process
					if( pid == 0 ){

						// printf("child process: %d\n", getpid());

						// Check for redirection operators (<, >, |)
						int index = -1;
						int redir_mode = check_redir(cmd, &index);

						if( redir_mode == 1 || redir_mode == 2 ){

							prep_redirection( redir_mode, index, cmd);

						}

						// Tokenize cmd into command and options
						char **args = tokenize_string(cmd);
						// printf("lalalala\n");
						// Execute command with options
						return execvp( args[0], args );

					}

					// Inside parent process
					else{

						int status;
						
						// Receives status code from child process here
						wait(&status);

						// Handle erroreous command
						if( status == 65280 ){
							
							printf("%s: command not found\n", cmd);

						}

					}

				}

				else{

					perror("fork");

				}

			} // system command handler

		} // if ( !RET_SIG )

		RET_SIG = 0;
		CTRL_C_SIG = 0;

	} // while-loop ends

	return 0;

}