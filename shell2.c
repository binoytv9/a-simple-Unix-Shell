#include<sys/types.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<ctype.h>
#include<stdbool.h>
#include"error_functions.c"

#define None -1
#define MAX_ARG 100
#define MAX_LENGTH 500

extern char **environ;

int getch(void);
void ungetch(int);
char **stripToArg(char *lp);
int getLine(char *line, int ml);
void insertToStack(char *lp, char **cp[], char *op);

char buffer[MAX_LENGTH];
char *bufp = buffer;

int main(int argc, char *argv[])
{
	int i;
	int fd;
	char **argVec;
	char line[MAX_LENGTH];
	char operatorArr[MAX_ARG];	// to store &, >, |, etc
	char **commandArr[MAX_ARG];	// to store command

	if(argc > 1 || (argc > 1 && strcmp(argv[1],"--help") == 0))
		usageErr("%s - a simple SHELL interpreter\nNo arguments required\n",argv[0]);

	while(getLine(line, MAX_LENGTH) != EOF){

//printf("line = %s\n",line);

		if(strlen(line) == 0)
			continue;

		insertToStack(line, commandArr, operatorArr);
		char ***cp = commandArr;
		char *op = operatorArr;

/*
		printf("\nc\n");
		while(*cp != NULL){
			char **cpp = *cp;
			while(*cpp != NULL){
				printf("%s\n",*cpp++);
				getchar();
			}
			cp++;
		}

		printf("\no\n");
		while(*op != '\0'){
			printf("%c\n",*op++);
			getchar();
		}

		cp = commandArr;
		op = operatorArr;
*/

		int new_pfd[2];
		int old_pfd[] = {-1, -1};
		int prevOperator = None;
		int nextOperator = None;

		int childNum = 0; // to store number of child process created
		while(*cp != NULL){
			childNum++;
			argVec = *cp;

			nextOperator = *op;
			if(nextOperator == '|'){
				// creating pipe
				if(pipe(new_pfd) == -1)
					errExit("pipe");
			}


			switch(fork()){
				// error
				case -1:
					errExit("fork");
				// child
				case 0 :
					if(prevOperator == '|'){
						if(old_pfd[0] != STDIN_FILENO){
							if(dup2(old_pfd[0], STDIN_FILENO) == -1)
								errExit("dup2 1");
							if(close(old_pfd[0]) == -1)
								errExit("close 2");
						}
					}

printf("\n%c %s %c old:%d %d new:%d %d \n",prevOperator,argVec[0],nextOperator,old_pfd[0],old_pfd[1],new_pfd[0],new_pfd[1]);	

					if (nextOperator == '|')
						if(new_pfd[1] != STDOUT_FILENO){
							if(dup2(new_pfd[1], STDOUT_FILENO) == -1)
								errExit("dup2 2");
							if(close(new_pfd[1]) == -1)
								errExit("close 4");
						}
					else{
						fprintf(stderr,"\nundefined operator : %c\n",nextOperator);
						exit(EXIT_FAILURE);
					}

					execvp(argVec[0], argVec);
					// execvp failed
					if(errno == ENOENT){ // file not found
						fprintf(stderr,"%s: command not found\n",argVec[0]);
						exit(EXIT_FAILURE);
					}
					else
						errExit("execve");
				// parent
				default:
/*
					if(wait(NULL) == -1)
						errExit("wait");
*/

					prevOperator = nextOperator;
					old_pfd[0] = new_pfd[0];
					old_pfd[1] = new_pfd[1];
					break;
			}


		cp++;
		op++;
		}
	
/*	
		if(nextOperator == '|'){
			if(close(new_pfd[0]) == -1)
				errExit("close 5");
			if(close(new_pfd[1]) == -1)
				errExit("close 6");
		}

*/
//printf("childNum %d\n",childNum);
		// calling wait() for each child
		for(i = 0; i < childNum; i++)	
			if(wait(NULL) == -1)
				errExit("wait %d",i);
	}
	putchar('\n');

	exit(EXIT_SUCCESS);
}

void insertToStack(char *lp, char **cp[], char *op)
{
	char process[MAX_LENGTH];
	char *pp = process;
	char **processArg;

	while(*lp != '\0'){
		if(*lp == '&' || *lp == '>' || *lp == '|'){
			*pp = '\0';
			*cp++ = stripToArg(process);
			*op++ = *lp;
			pp = process;
		}
		else
			*pp++ = *lp;
		lp++;
	}
	*pp = '\0';
	*cp++ = stripToArg(process);

	*cp = NULL;
	*op = '\0';
}


/* takes a line and returns its length on success or EOF if error */
int getLine(char *line, int ml)
{
	int c;
	int len;

	printf(">>> ");

	len = 0;
	while(len < ml && (c = getch()) != '\n' && c != EOF){
		*line++ = c;
		len++;
	}
	*line = '\0';
	if(len == 0 && c == EOF)
		return EOF;

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
char **stripToArg(char *lp)
{
	char *wp;
	char **ap;
	char **processArg;
	char word[MAX_LENGTH];


	if((processArg = (char **)malloc(MAX_ARG)) == NULL)
		errExit("malloc");

	ap = processArg;

	if(lp == NULL)
		return;

	wp = word;
	while(*lp != '\0'){
		if(*lp == ' '){
			*wp = '\0';
			if(strlen(word) > 0)
				*ap++ = strdup(word);
			wp = word;
		}
		else
			*wp++ = *lp;
		lp++;
	}

	*wp = '\0';
	if(strlen(word) > 0)
		*ap++ = strdup(word);

	*ap = NULL;
	return processArg;
}
