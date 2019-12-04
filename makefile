OBJS	= muse.o segmentacion.o paginacion.o frames.o sockets.o virtual-memory.o muse-main.o
SOURCE	= muse.c segmentacion/segmentacion.c paginacion/paginacion.c paginacion/frames.c sockets/sockets.c virtual-memory/virtual-memory.c muse-main.c
HEADER	= muse.h segmentacion/segmentacion.h paginacion/paginacion.h paginacion/frames.h sockets/sockets.h virtual-memory/virtual-memory.h muse-main.h
OUT	= muse
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = -lcommons -lm -lpthread

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

muse.o: muse.c
	$(CC) $(FLAGS) muse.c

muse-main.o: muse-main.c
	$(CC) $(FLAGS) muse-main.c

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

valgrind_leakcheck: $(OUT)
	valgrind --tool=memcheck --track-origins=yes --leak-check=full --show-leak-kinds=all $(OUT)

valgrind_extreme: $(OUT)
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes $(OUT)