import os
import sys

def errExit(errMsg):
	print errMsg
	sys.exit(1)

def getLine(l1, l2, l3, pipeEnable=False, pipe2Enable = False, toBackground=False, redirection=False, redirectionFile=[""]):
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
			if c != '\n' and c != '' and c != '|':
				l2[0] += c
			else:
				break
		if len2 == 0 and c == '' :
                        errExit("no program provided after |\n");

		if c == '|':
			pipe2Enable = True
			len2 = 0
			while True:
				c = fd.read(1)
				if c != '\n' and c != '':
					l3[0] += c
				else:
					break
			if len2 == 0 and c == '' :
				errExit("no program provided after |\n");
		

	return  pipeEnable, pipe2Enable, toBackground, redirection, redirectionFile


def main():
	argc = len(sys.argv)
	if argc > 1 or (argc > 1 and sys.argv[1] == "--help"):
		errExit("Usage: %s - a simple  Shell interpreter\nNo arguments required" %sys.argv[0])

	pipeEnable = toBackground = redirection = redirectionFile = None

	while True:
		l1 = [""]
		l2 = [""]
		l3 = [""]

		ret = getLine(l1, l2, l3)
		if ret == None:
			break

		pipeEnable, pipe2Enable, toBackground, redirection, redirectionFile = ret
		
		argVec = l1[0].split()
		argVec2 = l2[0].split()
		argVec3 = l3[0].split()

		if pipeEnable:
			pfd = os.pipe()

		if pipe2Enable:
			pfd2 = os.pipe()

		if argVec == []:
			continue

		childPid = os.fork()
		if childPid == -1:
			errExit("fork")

		elif childPid == 0:
			if pipeEnable:
				os.close(pfd[0])
				if pfd[1] != sys.stdout.fileno():
					os.dup2(pfd[1], sys.stdout.fileno())
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
				if pfd[0] != sys.stdin.fileno():
					os.dup2(pfd[0], sys.stdin.fileno())
					os.close(pfd[0])

				if pipe2Enable:
					os.close(pfd2[0]) 
					if pfd2[1] != sys.stdout.fileno():
						os.dup2(pfd2[1],sys.stdout.fileno())
						os.close(pfd2[1])

				try:
					os.execlp(argVec2[0], *argVec2)
				except OSError:
					print "%s: command not found" %argVec2[0]
				else:
					errExit("execlp")


		if pipe2Enable:
			childPid = os.fork()
			if childPid == -1:
				errExit("fork")

			elif childPid == 0:
				os.close(pfd2[1])
				if pfd2[0] != sys.stdin.fileno():
					os.dup2(pfd2[0], sys.stdin.fileno())
					os.close(pfd2[0])

				try:
					os.execlp(argVec2[0], *argVec2)
				except OSError:
					print "%s: command not found" %argVec2[0]
				else:
					errExit("execlp")

		if pipeEnable:
			os.close(pfd[0])
			os.close(pfd[1])
			os.wait()
			os.wait()

		if pipe2Enable:
			os.close(pfd2[0])
			os.close(pfd2[1])
			os.wait()

		pipeEnable = False
		pipe2Enable = False
			
	print

main()
