OBJS	= suse.o utils.o
SOURCE	= muse.c utils.c
HEADER	= muse.h utils.h
OUT	= suse
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = -lcommons -lm -lpthread -lrt

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

suse.o: suse.c
	$(CC) $(FLAGS) suse.c

utils.o: utils.c
	$(CC) $(FLAGS) utils.c


clean:
	rm -f $(OBJS)

valgrind_leakcheck: $(OUT)
	valgrind --tool=memcheck --track-origins=yes --leak-check=full --show-leak-kinds=all $(OUT)

valgrind_extreme: $(OUT)
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes $(OUT)