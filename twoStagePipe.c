#include<stdio.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<unistd.h>
#include"tlpi_hdr.h"
#include"error_functions.c"

main()
{
	int cp;
	char *arg1[] = {"ls", NULL};
	char *arg2[] = {"sort", NULL};
	char *arg3[] = {"wc", NULL};

	// pipe between ls and sort
	int pfd1[2];
	pipe(pfd1);

	// pipe between sort and wc
	int pfd2[2];
	pipe(pfd2);

	// first child : ls
	cp = fork();

	if(cp == -1) // error
		errExit("fork 1");

	else if(cp ==  0){ // child
		close(pfd1[0]);
		close(pfd2[0]);
		close(pfd2[1]);

		dup2(pfd1[1], STDOUT_FILENO);
		close(pfd1[1]);

		execvp(arg1[0],arg1);
		errExit("execvp 1");
	}

	// second child : sort
	cp = fork();

	if(cp == -1) // error
		errExit("fork 2");

	else if(cp ==  0){ // child
		close(pfd1[1]);
		close(pfd2[0]);

		dup2(pfd1[0], STDIN_FILENO);
		close(pfd1[0]);
		
		dup2(pfd2[1], STDOUT_FILENO);
		close(pfd2[1]);

		execvp(arg2[0], arg2);
		errExit("execvp 2");
	}

	// third child : wc
	cp = fork();

	if(cp == -1) // error
		errExit("fork 3");

	else if(cp ==  0){ // child
		close(pfd1[0]);
		close(pfd1[1]);
		close(pfd2[1]);

		dup2(pfd2[0], STDIN_FILENO);
		close(pfd2[0]);

		execvp(arg3[0],arg3);
		errExit("execvp 3");
	}

	close(pfd1[0]);
	close(pfd1[1]);
	close(pfd2[1]);
	close(pfd2[0]);

	wait(NULL);
	wait(NULL);
	wait(NULL);
}
