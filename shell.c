#include<fcntl.h>
#include<ctype.h>
#include<stdbool.h>
#include"error_functions.c"

#define MAX_ARG 100
#define MAX_LENGTH 500

extern char **environ;

int getch(void);
void ungetch(int);
void stripToArg(char *, char **);
int getLine(char *, char **, int *, int *, int *, char *, int);

char buffer[MAX_LENGTH];
char *bufp = buffer;

int main(int argc, char *argv[])
{
	int fd;
	pid_t childPid;
	int redirection;
	int pipeEnable;
	int toBackground;
	char *line2 = NULL;
	char *argVec[MAX_ARG];
	char *argVec2[MAX_ARG];
	char line[MAX_LENGTH];
	char redirectionFile[MAX_LENGTH];

	if(argc > 1 || (argc > 1 && strcmp(argv[1],"--help") == 0))
		usageErr("%s - a simple SHELL interpreter\nNo arguments required\n",argv[0]);

	int pfd[2];
	if(pipe(pfd) == -1)
		errExit("pipe");

	pipeEnable = false;
	redirection = false;
	toBackground = false;
	while(getLine(line, &line2, &pipeEnable, &toBackground, &redirection, redirectionFile, MAX_LENGTH) != EOF){
		stripToArg(line, argVec);
		stripToArg(line2, argVec2);
		if(argVec[0] != NULL){
			switch(childPid = fork()){
				/* error */
				case -1:
					errExit("fork");
				/* child  */
				case 0 :
					if(pipeEnable){
						if(close(pfd[0]) == -1)
							errExit("close 1");
						if(pfd[1] != STDOUT_FILENO){
							if(dup2(pfd[1], STDOUT_FILENO) == -1)
								errExit("dup2 1");
							if(close(pfd[1]) == -1)
								errExit("close 2");
						}
					}

					if(redirection){
						fd = open(redirectionFile, O_WRONLY | O_TRUNC | O_CREAT, 
								S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
						close(STDOUT_FILENO);	/* closing stdout */
						dup(fd);		/* fd and 1 points to same file */
					}

					execvp(argVec[0], argVec);
					if(errno == ENOENT){ /* file not found */
						fprintf(stderr,"%s: command not found\n",argVec[0]);
						exit(EXIT_FAILURE);
					}
					else
						errExit("execve");
				/* parent */
				default:
					if(!pipeEnable && !toBackground && wait(NULL) == -1)
						errExit("wait");
					/* resetting flags */
					redirection = false;
					toBackground = false;
					break;
			}
		}

		if(pipeEnable){
			switch(fork()){
				/* error */
				case -1:
					errExit("fork");
				/* child  */
				case 0 :
					if(pipeEnable){
						if(close(pfd[1]) == -1)
							errExit("close 3");
						if(pfd[0] != STDIN_FILENO){
							if(dup2(pfd[0], STDIN_FILENO) == -1)
								errExit("dup2 2");
							if(close(pfd[0]) == -1)
								errExit("close 4");
						}
					}
					execvp(argVec2[0], argVec2);
					if(errno == ENOENT){ /* file not found */
						fprintf(stderr,"%s: command not found\n",argVec[0]);
						exit(EXIT_FAILURE);
					}
					else
						errExit("execve");
				/* parent */
				default:
					break;
			}

			if (close(pfd[0]) == -1)
				errExit("close 5");
			if (close(pfd[1]) == -1)
				errExit("close 6");
			if (wait(NULL) == -1)
				errExit("wait 1");
			if (wait(NULL) == -1)
				errExit("wait 2");

			pipeEnable = false;
		}

	}
	putchar('\n');

	exit(EXIT_SUCCESS);
}


/* takes a line and returns its length on success or EOF if error */
int getLine(char *line, char **line2, int *pipeEnable, int *toBackground, int *redirection, char *redirectionFile, int ml)
{
	int c;
	char *lp;
	int len, len2;

	printf(">>> ");

	len = 0;
	while(len < ml && (c = getch()) != '\n' && c != '&' && c != '>' && c != '|' && c != EOF){
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

		while(isspace(c = getch())); /* skipping white space between '>' and filename, if any */
		if(!isspace(c))
			ungetch(c);

		while(!isspace(c = getch()))
			*redirectionFile++ = c;
		*redirectionFile = '\0';
	}
	if(c == '|'){
		len2 = 0;
		if((*line2 = (char *)malloc(sizeof(ml))) == NULL)
			errExit("malloc");
		lp = *line2;
		while(len2 < ml && (c = getch()) != '\n' && c != EOF){
			*lp++ = c;
			len2++;
		}
		*lp = '\0';
		if(len2 == 0 && c == EOF){
			fprintf(stderr, "no program provided after |\n");
			free(line2);
			exit(EXIT_FAILURE);
		}
		*pipeEnable = true;
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
	if(line == NULL)
		return;

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
