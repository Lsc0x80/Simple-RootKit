CC=gcc

all: checkbin test1 test2 rkit

test1: test1.c
	$(CC) -o bin/test1 test1.c
test2: test2.c
	$(CC) -o bin/test2 test2.c
rkit: rkit.c
	$(CC) -fPIC -shared -o bin/rootkit.so rkit.c -ldl
checkbin:
	@if [ ! -d "bin" ]; then \
		mkdir bin; \
	fi