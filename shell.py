import os
import sys

def errExit(errMsg):
	print errMsg
	sys.exit(1)

def getLine(l1, l2, pipeEnable=False, toBackground=False, redirection=False, redirectionFile=[""]):
	fd = sys.stdin
	len1 = 0
	print ">>> ",
	while True:
		c = fd.read(1)
		if c != '\n' and c != '&' and c != '>' and c != '|' and c != '':
			l1[0] += c
		else:
			break
		len1 += 1

	if len1 == 0 and c == '':
		return None

	if c == '&':
		toBackground = True

	if c == '>':
		redirection = True
		c = fd.read(1)
		while c.isspace():
			c = fd.read(1)

		while c.isalnum():
			redirectionFile[0] += c
			c = fd.read(1)

	if c == '|':
		pipeEnable = True
		len2 = 0
		while True:
			c = fd.read(1)
			if c != '\n' and c != '':
				l2[0] += c
			else:
				break
		if len2 == 0 and c == '' :
                        errExit("no program provided after |\n");
		

	return  pipeEnable, toBackground, redirection, redirectionFile


def main():
	argc = len(sys.argv)
	if argc > 1 or (argc > 1 and sys.argv[1] == "--help"):
		errExit("Usage: %s - a simple  Shell interpreter\nNo arguments required" %sys.argv[0])

	pipeEnable = toBackground = redirection = redirectionFile = None

	while True:
		l1 = [""]
		l2 = [""]

		ret = getLine(l1, l2)
		if ret == None:
			break

		pipeEnable, toBackground, redirection, redirectionFile = ret
		
		argVec = l1[0].split()
		argVec2 = l2[0].split()

		pfd = os.pipe()

		if argVec == []:
			continue

		childPid = os.fork()
		if childPid == -1:
			errExit("fork")

		elif childPid == 0:
			if pipeEnable:
				os.close(pfd[0])
				if pfd[1] != 1:
					os.dup2(pfd[1], 1)
					os.close(pfd[1])

			if redirection:
				fd = open(redirectionFile[0], 'w')
				if fd != 1:
					os.dup2(fd.fileno(),1) 

			try:
				os.execlp(argVec[0], *argVec)
			except OSError:
				print "%s: command not found" %argVec[0]
			else:
				errExit("execlp")

		else:
			if not pipeEnable and not toBackground:
				os.wait()
			redirection = False
			toBackground = False

		if pipeEnable:
			childPid = os.fork()
			if childPid == -1:
				errExit("fork")

			elif childPid == 0:
				os.close(pfd[1])
				if pfd[0] != 0:
					os.dup2(pfd[0], 0)
					os.close(pfd[0])

				try:
					os.execlp(argVec2[0], *argVec2)
				except OSError:
					print "%s: command not found" %argVec2[0]
				else:
					errExit("execlp")

			os.close(pfd[0])
			os.close(pfd[1])
			os.wait()
			os.wait()

			pipeEnable = False
			
	print

main()
