CC:= gcc
FLAGS:= -g -Wall
BUILD:=../linuse-tests-programs/bin

sockets.o:
	$(CC) -c -Wall ../sockets/sockets.c -o ../$(BUILD)/sockets.o -lcommons

sockets: sockets.o
	$(CC) -shared -o ../$(BUILD)/libsockets.so ../$(BUILD)/sockets.o -lcommons
