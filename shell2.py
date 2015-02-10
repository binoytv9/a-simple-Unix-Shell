import sys
import os

def errExit(msg):
	print msg
	sys.exit(1)

def split(command):
	string = ""
	c = []
	o = []
	for char in command:
		if char == '>' or  char == '|' or char == '&':
			if string:
				c.append(string.split())
			o.append(char)
			string = ""
		else:
			string += char
	if string:
		c.append(string.split())

	return c, o



def main():
	argc = len(sys.argv)
	if argc > 1 or (argc > 1 and sys.argv[1] == "--help"):
		errExit("Usage: %s - a simple  Shell interpreter\nNo arguments required" %sys.argv[0])

	while True:
		try:
			command = raw_input(">> ")
		except EOFError:
			break

		if len(command) == 0:
			continue

		cVec, oVec = split(command)
		for i in cVec:
			print i

		for i in oVec:
			print i

		#continue
		childPid = os.fork()
		if childPid == -1:
			errExit("fork")
		elif childPid == 0:
			try:
				os.execlp(argVec[0], *argVec)
			except OSError:
				print "%s: command not found" %argVec[0]
			else:
				errExit("execlp")
		else:
			os.wait()
	print

main()
