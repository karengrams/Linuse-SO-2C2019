OBJS:= muse.o segmentacion.o paginacion.o frames.o sockets.o virtual-memory.o muse-main.o
OBJSTEST:= libmuse.o
TEST:= recursiva.o archivo_de_swap_supermasivo.o revolucion_compartida_de_mentiritas.o
OUT:= muse
CC:= gcc
FLAGS:= -g -Wall
LFLAGS:= -lcommons -lm -lpthread -lrt
BUILD:=bin/

tests: sockets.so $(OBJSTEST) $(TEST)
	$(CC) -L./$(BUILD) -g -Wall -fpic $(BUILD)$(OBJSTEST) recursiva.c -o $(BUILD)recursiva -lcommons -lsockets
	$(CC) -L./$(BUILD) -g -Wall -fpic $(BUILD)$(OBJSTEST) archivo_de_swap_supermasivo.c -o $(BUILD)archivo_de_swap_supermasivo -lcommons -lsockets 

muse: sockets.so $(OBJS)
	$(CC) -L./$(BUILD) -g -Wall bin/muse.o bin/segmentacion.o bin/paginacion.o bin/frames.o bin/virtual-memory.o bin/muse-main.o -o $(BUILD)$(OUT) $(LFLAGS) -lcommons -lsockets

libmuse.o:
	$(CC) -L./$(BUILD) $(FLAGS) -c libmuse.c -o $(BUILD)libmuse.o -lcommons

muse.o:
	$(CC) -L./$(BUILD) $(FLAGS) -c -fpic  muse.c -o $(BUILD)muse.o -lcommons

muse-main.o:
	$(CC) -L./$(BUILD) $(FLAGS) -c -fpic muse-main.c -o $(BUILD)muse-main.o -lcommons -lpthread

segmentacion.o:
	$(CC) -L./$(BUILD) $(FLAGS) -c -fpic segmentacion/segmentacion.c -o $(BUILD)segmentacion.o -lcommons -lsockets

paginacion.o:
	$(CC) -c $(FLAGS) -fpic paginacion/paginacion.c -o $(BUILD)paginacion.o -lcommons

frames.o:
	$(CC) -c $(FLAGS) -fpic paginacion/frames.c -o $(BUILD)frames.o -lcommons

sockets.o:
	$(CC) -c -Wall sockets/sockets.c -o $(BUILD)sockets.o -lcommons

sockets.so: sockets.o
	$(CC) -shared -o $(BUILD)libsockets.so $(BUILD)sockets.o -lcommons

virtual-memory.o:
	$(CC) -c $(FLAGS) -fpic virtual-memory/virtual-memory.c -o $(BUILD)virtual-memory.o -lcommons
	
recursiva-test:
	LD_LIBRARY_PATH=$LD_LIBRARY_PATH:~/Escritorio/tp-2019-2c-Lo-dejaria-todo-porque-funcionara/memoria./sockets/ ./$(BUILD)recursiva

archivo_de_swap_supermasivo-test:
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):.~/Escritorio/tp-2019-2c-Lo-dejaria-todo-porque-funcionara/memoria./sockets/ ./$(BUILD)archivo_de_swap_supermasivo

clean:
	rm -f bin/muse.o bin/segmentacion.o bin/paginacion.o bin/frames.o bin/sockets.o bin/virtual-memory.o bin/muse-main.o $(BUILD)$(OBJSTEST) $(BUILD)archivo_de_swap_supermasivo.o $(BUILD)recursiva.o

valgrind_leakcheck: $(BUILD)/$(OUT)
	valgrind --tool=memcheck --track-origins=yes --leak-check=full --show-leak-kinds=all $(OUT)

valgrind_extreme: $(BUILD)/$(OUT)
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes $(OUT)