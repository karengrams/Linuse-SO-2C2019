OBJS	= muse.o segmentacion.o paginacion.o frames.o sockets.o virtual-memory.o
SOURCE	= muse.c segmentacion/segmentacion.c paginacion/paginacion.c paginacion/frames.c sockets/sockets.c virtual-memory/virtual-memory.c
HEADER	= muse.h segmentacion/segmentacion.h paginacion/paginacion.h paginacion/frames.h sockets/sockets.h virtual-memory/virtual-memory.h
OUT	= muse
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = -lcommons -lm -lpthread

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

muse.o: muse.c
	$(CC) $(FLAGS) muse.c

segmentacion.o: segmentacion/segmentacion.c
	$(CC) $(FLAGS) segmentacion/segmentacion.c

paginacion.o: paginacion/paginacion.c
	$(CC) $(FLAGS) paginacion/paginacion.c

frames.o: paginacion/frames.c
	$(CC) $(FLAGS) paginacion/frames.c

sockets.o: sockets/sockets.c
	$(CC) $(FLAGS) sockets/sockets.c

virtual-memory.o: virtual-memory/virtual-memory.c
	$(CC) $(FLAGS) virtual-memory/virtual-memory.c

clean:
	rm -f $(OBJS)