OBJS	= muse-main.o muse-server.o segmentacion.o sockets.o paginacion.o frames.o virtual-memory.o
SOURCE	= muse-main.c muse-server.c segmentacion/segmentacion.c sockets/sockets.c paginacion/paginacion.c paginacion/frames.c virtual-memory/virtual-memory.c
HEADER	= muse-main.h muse-structures.h muse-server.h segmentacion/segmentacion.h sockets/sockets.h paginacion/paginacion.h paginacion/frames.h virtual-memory/virtual-memory.h
OUT	= muse-server
CC	 = gcc
FLAGS	 = -g3 -c -Wall
LFLAGS	 = -lcommons -lm -lpthread -lrt

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

muse-main.o: muse-main.c
	$(CC) $(FLAGS) muse-main.c 

muse.o: muse-server.c
	$(CC) $(FLAGS) muse-server.c 

segmentacion.o: segmentacion/segmentacion.c
	$(CC) $(FLAGS) segmentacion/segmentacion.c 

sockets.o: sockets/sockets.c
	$(CC) $(FLAGS) sockets/sockets.c 

paginacion.o: paginacion/paginacion.c
	$(CC) $(FLAGS) paginacion/paginacion.c 

frames.o: paginacion/frames.c
	$(CC) $(FLAGS) paginacion/frames.c 

virtual-memory.o: virtual-memory/virtual-memory.c
	$(CC) $(FLAGS) virtual-memory/virtual-memory.c 

clean:
	rm -f $(OBJS)

debug: $(OUT)
	valgrind $(OUT)

valgrind: $(OUT)
	valgrind $(OUT)

valgrind_leakcheck: $(OUT)
	valgrind --leak-check=full $(OUT)

valgrind_extreme: $(OUT)
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes $(OUT)