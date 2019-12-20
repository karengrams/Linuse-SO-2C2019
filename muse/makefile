OBJS:= muse-server.o segmentacion.o paginacion.o frames.o sockets.o virtual-memory.o muse-main.o
OBJSTEST:= libmuse.o
OUT:= muse
CC:= gcc
FLAGS:= -g -Wall
LFLAGS:= -lcommons -lm -lpthread -lrt
BUILD:=bin

tests: sockets.so $(OBJSTEST) $(TEST)
	$(CC) -L..$(BUILD) -g -Wall -fpic ..$(BUILD)$(OBJSTEST) recursiva.c -o ..$(BUILD)recursiva -lcommons -lsockets
	$(CC) -L..$(BUILD) -g -Wall -fpic ..$(BUILD)$(OBJSTEST) archivo_de_swap_supermasivo.c -o ..$(BUILD)archivo_de_swap_supermasivo -lcommons -lsockets 

muse: $(OBJS) sockets.so
	$(CC) -L../$(BUILD) -g -Wall ../$(BUILD)/muse-server.o ../$(BUILD)/segmentacion.o ../$(BUILD)/paginacion.o ../$(BUILD)/frames.o ../$(BUILD)/virtual-memory.o ../$(BUILD)/muse-main.o -o ../$(BUILD)/$(OUT) $(LFLAGS) -lcommons -lsockets

libmuse.o:
	$(CC) -L..$(BUILD) $(FLAGS) -c libmuse.c -o ..$(BUILD)libmuse.o -lcommons

muse-server.o: 
	$(CC) -L ../$(BUILD)/ -I../sockets/ -Wall -c muse-server.c -o ../$(BUILD)/muse-server.o -lcommons -lsockets

muse-main.o:
	$(CC) -L ../$(BUILD)/  -I../sockets/ $(FLAGS) -c  muse-main.c -o ../$(BUILD)/muse-main.o -lcommons -lsockets -lpthread

segmentacion.o:
	$(CC) -L./$(BUILD) $(FLAGS) -c  segmentacion/segmentacion.c -o ../$(BUILD)/segmentacion.o -lcommons -lsockets

paginacion.o:
	$(CC) -c $(FLAGS)  paginacion/paginacion.c -o ../$(BUILD)/paginacion.o -lcommons

frames.o:
	$(CC) -c $(FLAGS)  paginacion/frames.c -o ../$(BUILD)/frames.o -lcommons

sockets.o:
	$(CC) -c -Wall ../sockets/sockets.c -o ../$(BUILD)/sockets.o -lcommons

sockets.so: sockets.o
	$(CC) -shared -o ../$(BUILD)/libsockets.so ../$(BUILD)/sockets.o -lcommons

virtual-memory.o:
	$(CC) -c $(FLAGS) -fpic virtual-memory/virtual-memory.c -o ../$(BUILD)/virtual-memory.o -lcommons

clean:
	rm -f ../bin/muse-server.o ../bin/segmentacion.o ../bin/paginacion.o ../bin/frames.o ../bin/sockets.o ../bin/virtual-memory.o ../bin/muse-main.o ../$(BUILD)/$(OBJSTEST)
