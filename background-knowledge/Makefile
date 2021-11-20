# #号是注释
hello:hello.o
	gcc hello.o -o hello

hello.o:hello.S
	gcc -c hello.s -o hello.o

hello.S:hello.i
	gcc -S hello.i -o hello.s

hello.i:hello.c	
	gcc -E hello.c -o hello.i

.PHONY:
clean:
	rm -rf hello.o hello.S hello.i hello
