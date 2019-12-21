
OBJS:= suse-server.o suse-main.o suse-planificador-largo-plazo.o suse-logs.o suse-config.o
OUT:= suse
CC:= gcc
FLAGS:= -g -Wall
LFLAGS:= -lcommons -lm -lpthread -lrt
BUILD:=../linuse-tests-programs/bin

suse: $(OBJS)
	$(CC) -L../$(BUILD) -g -Wall ../$(BUILD)/suse-server.o ../$(BUILD)/suse-config.o  ../$(BUILD)/suse-main.o ../$(BUILD)/suse-planificador-largo-plazo.o ../$(BUILD)/suse-logs.o -o ../$(BUILD)/$(OUT) $(LFLAGS) -lcommons -lsockets

suse-server.o:
	$(CC) -L ../$(BUILD)/  -I../sockets/ $(FLAGS) -c  suse-server.c -o ../$(BUILD)/suse-server.o -lcommons -lsockets

suse-planificador-largo-plazo.o:
	$(CC) -c $(FLAGS) suse-planificador-largo-plazo.c -o ../$(BUILD)/suse-planificador-largo-plazo.o -lcommons -lpthread

suse-logs.o:
	$(CC) -c $(FLAGS)  suse-logs.c -o ../$(BUILD)/suse-logs.o -lcommons

suse-config.o:
	$(CC) -c $(FLAGS)  suse-config.c -o ../$(BUILD)/suse-config.o -lcommons

suse-main.o:
	$(CC) -L ../$(BUILD)/  -I../sockets/ $(FLAGS) -c  suse-main.c -o ../$(BUILD)/suse-main.o -lcommons -lsockets -lpthread

clean:
	rm -f ../$(BUILD)/suse-logs.o ../$(BUILD)/suse-config.o ../$(BUILD)/suse-main.o ../$(BUILD)/suse-planificador-largo-plazo.o ../$(BUILD)/suse-server.o ../$(BUILD)/suse-config.o
