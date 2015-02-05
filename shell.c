#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"tlpi_hdr.h"

/* defines ename */
#include"ename.c.inc"

#define MAX_ARG 100
#define MAX_LENGTH 500

int getLine(char *line, int ml);
void stripToArg(char *line, char *ap[]);

extern char **environ;

int main(int argc, char *argv[])
{
	pid_t childPid;
	char *argVec[MAX_ARG];
	char line[MAX_LENGTH];

	if(argc > 1 || (argc > 1 && strcmp(argv[1],"--help") == 0))
		usageErr("%s - a simple SHELL interpreter\nNo arguments required\n",argv[0]);

	while(getLine(line, MAX_LENGTH) != EOF){

//printf("\nlen = %ld\n",strlen(line));

		stripToArg(line, argVec);
/*
char **ap = argVec;
while(*ap)
	printf("\n[%s]\n",*ap++);
*/

		if(argVec[0] != NULL)
			switch(childPid = fork()){
				case -1:/* error */
					errExit("fork");

				case 0 :/* child  */
					execvp(argVec[0], argVec);
					if(errno == ENOENT){ /* file not found */
						fprintf(stderr,"%s: command not found\n",argVec[0]);
						exit(EXIT_FAILURE);
					}
					else
						errExit("execve");

				default:/* parent */
					if(wait(NULL) == -1)
						errExit("wait");
					break;
			}
	}
	putchar('\n');

	exit(EXIT_SUCCESS);
}


/* takes a line and returns its length on success or EOF if error */
int getLine(char *line, int ml)
{
	int len;

	printf(">>> ");

	if(fgets(line, ml, stdin) == NULL)	/* error or end-of-file */
		return EOF;

	len = strlen(line);
	line[len-1] = '\0';			/* removing the extra '\n' */

	return len-1;
}


/* extract each argument and insert it into an array */
void stripToArg(char *line, char *ap[])
{
	char *wp,*lp;
	char word[MAX_LENGTH];

	lp = line;
	wp = word;
	while(*lp != '\0'){
		if(*lp == ' '){
			*wp = '\0';
			if(strlen(word))
				*ap++ = strdup(word);
			wp = word;
		}
		else
			*wp++ = *lp;
		lp++;
	}

	*wp = '\0';
	if(strlen(word))
		*ap++ = strdup(word);

	*ap = NULL;
}
