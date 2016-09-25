// JOSH LIU ID:  260612384 

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <termios.h>

char **history;
int counter;
struct termios old_termios, new_termios;

// adds a command to the history buffer (at pos 0) and shifts other elements
void addhistory(char *line) {
		
	int i;
	for (i = 10; i > 0; i--) {
		strcpy(history[i], history[i - 1]);
		
	}
	strcpy(history[0], line);
}

// removes the most recent element from the history buffer (pos 0) and shifts other elements
void removehistory() {

	int i;
	for (i = 0; i < 10; i++) strcpy(history[i], history[i + 1]);
}

// takes in a string (line) and splits it into an array of tokens (adding a terminating NULL) returns 1 if &
int tokenize(char *args[], char *line) {
	
	int i = 0;
	int bg = 0;
	char *token;
	char *templine = malloc(sizeof(line));
	strtok(line, "\n");
	strcpy(templine, line);
	while ((token = strsep(&templine, " \t\n")) != NULL) {
		int j;		
		for (j = 0; j < strlen(token); j++) {
			if (token[j] <= 32) token[j] = '\0';
		}
		if (strlen(token) > 0) args[i++] = token;
		if (strcmp(token, "&") == 0) {
			args[i - 1] = NULL;
			bg = 1;
		}
		args[i] = NULL;
	}
	free(templine);
	return bg;
} 

// uses chdir to change the directory when cd is input
int changedir(char *args[], char *line) {

	if (strcmp(args[0], "cd") == 0) {
		if (args[1] == NULL) {
			printf("failed: you must provide an argument.");
			return 1;	
		} else if (args[2] != NULL) {
			printf("failed: too many arguments.");
			return 1;
		} else {
			if (chdir(args[1]) == -1) {
				char errmsg[64];
				snprintf(errmsg, sizeof(errmsg), "cd %s failed", args[1]);
				perror(errmsg);
				return 1;
			}
			addhistory(line);
			return 2;
		}
	}
	return 0;

}

// uses getcwd() to show the current directory
int getpwd(char *args[], char *line) {
	
	if (strcmp(args[0], "pwd") == 0) {
		if (args[1] != NULL) {
			printf("failed: pwd has no arguments.");
			return 1;
		} else {
			printf("%s\n", getcwd(NULL, 0));
			addhistory(line);
			return 2;
		}	
	}
	return 0;

}

// prints the job for one line given by the getjobs function
void printjobs(char *line, int count) {

	char *pid = "\0";
	char *name = "\0";
	char *defunct = "\0";
	char *token = "\0";
	int i = 0;
  
 	token = strtok(line, " ");
	pid = token;
	i++;

	// get desired tokens of the lines of ps -ppid
	while (token != NULL) {
		if (i == 4) name = token;
		if (i == 5) defunct = token;
    		token = strtok(NULL, " ");
		i++;
  	}
	
	// print out the modified lines
	if (strcmp(name, "sh\n") != 0  && strcmp(name, "sh") != 0 && strcmp(pid, "NULL") != 0) 
		printf("ID (for fg): %d    pid: %s    name: %s", (count + 1), pid, name); 
	if (strcmp(defunct, "\0") != 0) printf(" <defunct>\n");
}

// gets list of jobs 
int getjobs(char *args[], char *line) {
	
	if (strcmp(args[0], "jobs") == 0) {
		if (args[1] != NULL) {
			printf("failed: jobs has no arguments.");
			return 1;
		} else {
			// creating command
    			char str[50] = "ps --ppid ";
	  		char ppid [7];
    			sprintf(ppid, "%d", getpid());
    			strcat(str, ppid);
    			
			// getting output
  			FILE *fp;
			char path[1035];
 			fp = popen(str, "r");
  			if (fp == NULL) {
    				printf("Failed to run command\n" );
    				return 1;
  			}

			char inputline[50];

			printf("Running processes:\n");			
			int count = -1;
  			// reads the output of ps and prints out the formatted output
  			while (fgets(path, sizeof(path) - 1, fp) != NULL) {
				if (count < 0) {
					count++;
					continue;
				}
				if (count == 100) break;  
				strcpy(inputline, path);
				printjobs(inputline, count);
				count++;
 			}
			
 		 	pclose(fp);
			addhistory(line);
			return 2;
		}
	}
	return 0;
}

// returns string of request pid given an ID and returns null if none is found
char *getfgid(char *input) {

	char *output = "\0";
	char *token = "\0";
	int i = 0;
	token = strtok(input, " ");
	if (token != NULL) output = token;

	if (strcmp(output, "\0") == 0) return "\0";
	return output;
}

// does the fg command
int getfg(char *args[], char *line) {

		if (strcmp(args[0], "fg") == 0) {
		if (args[1] == NULL) {
			printf("failed: fg requires an argument (type \"jobs\" to find the ID of the process).");
			return 1;
		} else if (args[2] != NULL) {
			printf("failed: too many arguments.");
			return 1;
		} else {
			// have to get the output of the jobs command first 
    			char str[50] = "ps --ppid ";
	  		char ppid [7];
    			sprintf(ppid, "%d", getpid());
    			strcat(str, ppid);
    			
			// getting output
  			FILE *fp;
			char path[1035];
 			fp = popen(str, "r");
  			if (fp == NULL) {
    				printf("Failed to run command\n" );
    				return 1;
  			}

			// get input ID
			char inputline[50];
			char *ans = "\0";
			int ID;
			if ((ID = atoi(args[1])) == 0) {
				printf("failed: argument is not an integer.");
				return 1;
			}

			int count = 0;
  			// compare ID to jobs table and extract correct pid
  			while (fgets(path, sizeof(path) - 1, fp) != NULL) {
				if (count == 100) break;  
				if (ID == count) {	
					strcpy(inputline, path);			
					ans = getfgid(inputline);
					break;
				}
				count++;
 			}					
			if (strcmp(ans, "\0") == 0) {
				printf("failed: process with ID %s not found.", args[1]);
			} else {
				// bring process to foreground by waiting on it
				ID = atoi(ans);
				waitpid(ID, NULL, 0);
			}
 		 	pclose(fp);
			addhistory(line);
			return 2;
		}
	}
	return 0;

}


// prints out history buffer with command numbers
int gethistory(char *args[], char *line) {

	if (strcmp(args[0], "history") == 0) {
		if (args[1] != NULL) {
			printf("failed: too many arguments.");
			return 1;
		} else {
			printf("| ");
			int i;
			for (i = 0; i < 10; i++) {
				if ((counter - i) < 1) break;
				printf("%d: %s | ", (counter - i), history[i]);
			}
			printf("\n");
			addhistory(line);
			return 2;
		}
	}
	return 0;
}

// checks to see if any shell commands are selected by the "r" command
int choosehistory(char *args[], char *line, int flag) {
	
	int status = 0;

	// cd is treated differently			
	if (strcmp(args[0], "cd") == 0) status = changedir(args, line);
	// pwd			
	if (strcmp(args[0], "pwd") == 0) status = getpwd(args, line);
	// jobs
	if (strcmp(args[0], "jobs") == 0) status = getjobs(args, line);
	// fg 
	if (strcmp(args[0], "fg") == 0) status = getfg(args, line);
	// history
	if (strcmp(args[0], "history") == 0) status = gethistory(args, line);

	if (status == 2) {
		printf("\nExecuting command %d: %s\n", (counter - flag), history[flag + 1]);
		return 2;				
	}
	else if (status == 1) return 1;
	return 0;

}

// checks user input and configures the args[] array to be used in executing commands
int getcmd(char *prompt, char *args[], int *background) {
	
	int length;
	char *line;
	int status;
	size_t linecap = 0;
	
	if (strcmp(prompt, "") != 0) printf("%s", prompt);
	length = getline(&line, &linecap, stdin);	

	// checks for various inputs that could cause seg faults
	if (length <= 0) exit(-1);
	if (strcmp(line, "\n") == 0) return 1;

	// checks if Ctrl-D was entered and exits appropriately
	if (line[0] == 4 || strcmp(line, "<Control><D>\n") == 0) {
		printf("\nExiting...\n\n");
		tcsetattr(0, TCSANOW, &old_termios);
		exit(0);
	}

	// checks for other control characters
	if (line[0] >=0 && line[0] <= 31) return 1;

	// checks if the background modifier is present
	if (index(line, '&') != NULL) {
		*background = 1;
	} else {
		*background = 0;
	}
	
	// populates the args array with the correct tokens
	tokenize(args, line);

	// code for the exit command.
	if (strcmp(args[0], "exit") == 0) {
		if (args[1] != NULL) {
			printf("falied: exit has no arguments.");
			return 1;
		}
		printf("\nExiting...\n\n");
		tcsetattr(0, TCSANOW, &old_termios);
		exit(0);
	}

	// status check for the fg command. 0 no match, 1 error, 2 success
	status = getfg(args, line);
	if (status == 1) return 1;
	if (status == 2) return 2;

	// status check for the history command.
	status = gethistory(args, line);
	if (status == 1) return 1;
	if (status == 2) return 2;

	// status check for the jobs command.
	status = getjobs(args, line);
	if (status == 1) return 1;
	if (status == 2) return 2;

	// status check for the pwd command. 
	status = getpwd(args, line);
	if (status == 1) return 1;
	if (status == 2) return 2;	

	// status check for the change directory command.
	status = changedir(args, line);
	if (status == 1) return 1;
	if (status == 2) return 2;

	// code for the history commands
	if (strcmp(args[0], "r") == 0) {
		
		int match = 0;
		// only r is input
		if (args[1] == NULL) {
			
			strcpy(line, history[0]);
			// catches error if there is no history yet
			if (strcmp(history[0], "\0") == 0) {
				printf("No match found in history");			
				return 1;
			}
			
			// get the desired command ready for execution
			*background = tokenize(args, history[0]);
			
			// check to see if its a shell command
			status = choosehistory(args, line, 0);
			if (status == 2) return 2;
			else if (status == 1) return 1;

			printf("Executing command %d: %s\n", counter, history[0]);
		
		// r + x is input
		} else {
			
			int i;
			// comparing first letters of command + history
			for (i = 0; i < 10; i++) {
				char *s1 = history[i];
				char *s2 = args[1];
				if (s1[0] == s2[0]) {
					
					strcpy(line, history[i]);
					*background = tokenize(args, history[i]);
					
					// check to see if it is a shell command 
					status = choosehistory(args, line, i);
					if (status == 2) return 2;
					else if (status == 1) return 1;

					// the command is for exec()
					printf("Executing command %d: %s\n", (counter - i), history[i]);
					match = 1;
					break;
				}			
			}
			if (match == 0) {
				printf("No match found in history"); 
				return 1;			
			}
		}
	}
	
	addhistory(line);
	// freeing the space allocated by getline
	free(line);
	return 0;

}

// handles various signals that the process recieves (i.e Ctrl-D)
void sig_handler(int signo) {

	if (signo == SIGINT) {
		// checking for Ctrl-C
		if (signo == 2) {
			printf("\nShell closed.\n");
			tcsetattr(0, TCSANOW, &old_termios);
			exit(0);
		}
	}
}

int main(void) {
	
	// signal detection
	if (signal(SIGINT, sig_handler) == SIG_ERR) printf("\ncan't catch SIGINT\n");

	// setting Ctrl-D to not be EOF so it can be caught as a signal
	setvbuf(stdout, NULL, _IONBF, 0);
	tcgetattr(0, &old_termios);

	new_termios = old_termios;
	new_termios.c_cc[VEOF] = 15; // Ctrl-O
	new_termios.c_cc[VINTR] = 3; // Ctrl-C
	tcsetattr(0, TCSANOW, &new_termios);

	char *args[20];
	int bg;
	int childstatus = 0;

	// the current history position
	counter = 0;

	// creating space for history, commands are assumed to be less than 1000 chars
	// it has size 11 to hold the one extra command that might get deleted if exec fails
	history = malloc(11 * sizeof(char *));
	int i;
	for (i = 0; i < 11; i++) {
		history[i] = malloc(1001);
		int j;
		for (j = 0; j < 1001; j++) history[i][j] = '\0';
	}
	
	while(1) {

		bg = 0;
		
		// cleaning up zombie processes
		waitpid(-1, NULL, WNOHANG);

		// returns 1 if a shell command has failed in some way. 2 for shell command success.
		int status = getcmd("\nCOMP310SHELL >>  ", args, &bg);	
		if (status == 1) continue;
		else {
			counter++;
			if (status == 2) continue;
		}

		int cpid;
		// creating child process
		if ((cpid = fork()) == 0) {
			if (execvp(args[0], args) == -1) {
				char errmsg[64];
				snprintf(errmsg, sizeof(errmsg), "exec %s failed", args[0]);
				perror(errmsg); 
				exit(-1);
			} 
		} else {
			// waiting if & is not specified
			if (bg == 0) {
				// arbitrary value that tells me if parent is still waiting
				waitpid(cpid, &childstatus, 0);
				if (childstatus != 0) {
					// remove the most recent history entry
					removehistory();
					counter--;
				}
			}
		}
		
	}

}




