OBJS:= muse-server.o segmentacion.o paginacion.o frames.o virtual-memory.o muse-main.o
OBJSTEST:= libmuse.o
OUT:= muse
CC:= gcc
FLAGS:= -g -Wall
LFLAGS:= -lcommons -lm -lpthread -lrt
BUILD:=../linuse-tests-programs/bin

tests: libmuse.o
#	$(CC) -L../$(BUILD)/  -I../sockets/ -g -Wall -fpic ../$(BUILD)/$(OBJSTEST) recursiva.c -o ../$(BUILD)/recursiva -lcommons -lsocketsmuse -lmuse
	$(CC) -L../$(BUILD)/ -I../sockets/  -g -Wall  ../$(BUILD)/$(OBJSTEST) archivo_de_swap_supermasivo.c -o ../$(BUILD)/archivo_de_swap_supermasivo -lcommons -lsocketsmuse

muse: $(OBJS)
	$(CC) -L../$(BUILD)/ -g -Wall ../$(BUILD)/muse-server.o ../$(BUILD)/segmentacion.o ../$(BUILD)/paginacion.o ../$(BUILD)/frames.o ../$(BUILD)/virtual-memory.o ../$(BUILD)/muse-main.o -o ../$(BUILD)/$(OUT) $(LFLAGS) -lcommons -lsocketsmuse

libmuse.o:
	$(CC) -L../$(BUILD) -I../sockets/ $(FLAGS) -c libmuse.c -o ../$(BUILD)/libmuse.o -lcommons -lsocketsmuse

muse-server.o:
	$(CC) -L ../$(BUILD)/ -I../sockets/ -Wall -c muse-server.c -o ../$(BUILD)/muse-server.o -lcommons -lsocketsmuse

muse-main.o:
	$(CC) -L ../$(BUILD)/  -I../sockets/ $(FLAGS) -c  muse-main.c -o ../$(BUILD)/muse-main.o -lcommons -lsocketsmuse -lpthread

segmentacion.o:
	$(CC) -L./$(BUILD) $(FLAGS) -c  segmentacion/segmentacion.c -o ../$(BUILD)/segmentacion.o -lcommons -lsocketsmuse

paginacion.o:
	$(CC) -c $(FLAGS)  paginacion/paginacion.c -o ../$(BUILD)/paginacion.o -lcommons

frames.o:
	$(CC) -c $(FLAGS)  paginacion/frames.c -o ../$(BUILD)/frames.o -lcommons

virtual-memory.o:
	$(CC) -c $(FLAGS) -fpic virtual-memory/virtual-memory.c -o ../$(BUILD)/virtual-memory.o -lcommons

clean:
	rm -f ../$(BUILD)/muse-server.o ../$(BUILD)/segmentacion.o ../$(BUILD)/paginacion.o ../$(BUILD)/frames.o ../$(BUILD)/virtual-memory.o ../$(BUILD)/muse-main.o ../$(BUILD)/$(OBJSTEST)
