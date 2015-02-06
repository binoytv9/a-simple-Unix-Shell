#include<fcntl.h>
#include<ctype.h>
#include<stdbool.h>
#include"error_functions.c"

#define MAX_ARG 100
#define MAX_LENGTH 500

extern char **environ;

int getch(void);
void ungetch(int c);
void stripToArg(char *line, char *ap[]);
int getLine(char *line, int *toBackground, int *redirection, char *redirectionFile, int ml);

char buffer[MAX_LENGTH];
char *bufp = buffer;

int main(int argc, char *argv[])
{
	int fd;
	pid_t childPid;
	char *argVec[MAX_ARG];
	char line[MAX_LENGTH];
	char redirectionFile[MAX_LENGTH];
	int toBackground;
	int redirection;

	if(argc > 1 || (argc > 1 && strcmp(argv[1],"--help") == 0))
		usageErr("%s - a simple SHELL interpreter\nNo arguments required\n",argv[0]);

	toBackground = false;
	redirection = false;
	while(getLine(line, &toBackground, &redirection, redirectionFile, MAX_LENGTH) != EOF){
		stripToArg(line, argVec);
		if(argVec[0] != NULL){
			switch(childPid = fork()){
				case -1:/* error */
					errExit("fork");

				case 0 :/* child  */
					if(redirection){
						fd = open(redirectionFile, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
						close(1);
						dup(fd);
					}
					execvp(argVec[0], argVec);
					if(errno == ENOENT){ /* file not found */
						fprintf(stderr,"%s: command not found\n",argVec[0]);
						exit(EXIT_FAILURE);
					}
					else
						errExit("execve");

				default:/* parent */
					if(!toBackground && wait(NULL) == -1)
						errExit("wait");
					redirection = false;
					toBackground = false;
					break;
			}
		}
	}
	putchar('\n');

	exit(EXIT_SUCCESS);
}


/* takes a line and returns its length on success or EOF if error */
int getLine(char *line, int *toBackground, int *redirection, char *redirectionFile, int ml)
{
	int c;
	int len;

	printf(">>> ");

	len = 0;
	while(len < ml && (c = getch()) != '\n' && c != '&' && c != '>' && c != EOF){
		*line++ = c;
		len++;
	}
	*line = '\0';
	if(len == 0 && c == EOF)
		return EOF;
	if(c == '&')
		*toBackground = true;
	if(c == '>'){
		*redirection = true;

		while(isspace(c = getch())); // skipping white space between '>' and filename, if any
		if(!isspace(c))
			ungetch(c);

		while(!isspace(c = getch()))
			*redirectionFile++ = c;
		*redirectionFile = '\0';
	}

	if(c != '\n' && c != '&')
		ungetch(c);

	return len;
}

int getch(void)
{
	return bufp-buffer > 0 ? *--bufp : getchar();
}

void ungetch(int c)
{
	*bufp++ = c;
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
