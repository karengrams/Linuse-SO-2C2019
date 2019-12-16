OBJS	= muse.o segmentacion.o paginacion.o frames.o sockets.o virtual-memory.o muse-main.o
OBJSTEST= libmuse.o archivo_de_swap_supermasivo.o recursiva.o 
SOURCE	= muse.c segmentacion/segmentacion.c paginacion/paginacion.c paginacion/frames.c sockets/sockets.c virtual-memory/virtual-memory.c muse-main.c
HEADER	= muse.h segmentacion/segmentacion.h paginacion/paginacion.h paginacion/frames.h sockets/sockets.h virtual-memory/virtual-memory.h muse-main.h
OUT	= muse
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = -lcommons -lm -lpthread -lrt

tests:
	$(CC) -Wall recursiva.c libmuse.o sockets.o -o recursiva -lcommons
	$(CC) -Wall archivo_de_swap_supermasivo.c libmuse.o sockets.o -o archivo_de_swap_supermasivo -lcommons 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

libmuse.o: libmuse.c
	$(CC) $(FLAGS) libmuse.c sockets/sockets.h -lcommons

muse.o: muse.c 
	$(CC) $(FLAGS) muse.c segmentacion/segmentacion.c paginacion/paginacion.c paginacion/frames.c sockets/sockets.c -lcommons

muse-main.o: muse-main.c
	$(CC) $(FLAGS) muse-main.c -lpthread -lcommons

segmentacion.o: segmentacion/segmentacion.c
	$(CC) $(FLAGS) segmentacion/segmentacion.c paginacion/paginacion.c paginacion/frames.c -lcommons

paginacion.o: paginacion/paginacion.c
	$(CC) $(FLAGS) paginacion/paginacion.c paginacion/frames.c virtual-memory/virtual-memory.c -lcommons

frames.o: paginacion/frames.c
	$(CC) $(FLAGS) paginacion/frames.c paginacion/paginacion.c -lcommons

sockets.o: sockets/sockets.c
	$(CC) $(FLAGS) sockets/sockets.c -lcommons

virtual-memory.o: virtual-memory/virtual-memory.c
	$(CC) $(FLAGS) virtual-memory/virtual-memory.c -lcommons
	
recursiva-test:
	./recursiva

archivo_de_swap_supermasivo-test:
	./archivo_de_swap_supermasivo

clean:
	rm -f $(OBJS) $(OBJSTEST)

valgrind_leakcheck: $(OUT)
	valgrind --tool=memcheck --track-origins=yes --leak-check=full --show-leak-kinds=all $(OUT)

valgrind_extreme: $(OUT)
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes $(OUT)