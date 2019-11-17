OBJS	= muse.o segmentacion/segmentacion.o paginacion/paginacion.o paginacion/frames.o sockets/sockets.o
SOURCE	= muse.c segmentacion/segmentacion.c paginacion/paginacion.c paginacion/frames.c sockets/sockets.c
HEADER	= segmentacion/segmentacion.h paginacion/paginacion.h paginacion/frames.h sockets/sockets.h
OUT	= muse
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = -lcommons

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

muse.o: muse.c
	$(CC) $(FLAGS) muse.c 

segmentacion/segmentacion.o: segmentacion/segmentacion.c
	$(CC) $(FLAGS) segmentacion/segmentacion.c 

paginacion/paginacion.o: paginacion/paginacion.c
	$(CC) $(FLAGS) paginacion/paginacion.c 

paginacion/frames.o: paginacion/frames.c
	$(CC) $(FLAGS) paginacion/frames.c 

sockets/sockets.o: sockets/sockets.c
	$(CC) $(FLAGS) sockets/sockets.c 


clean:
	rm -f $(OBJS) $(OUT)	
