UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CFLAGS := -D_XOPEN_SOURCE -Wno-deprecated-declarations
endif

CC := gcc
NAME := hilolayex
BUILD=bin

all: clean $(NAME).so

clean:
	$(RM) *.o
	$(RM) *.so
	$(RM) -r bin/
	mkdir -p bin

$(NAME).o:
	$(CC) -I /home/utnso/tp-2019-2c-Lo-dejaria-todo-porque-funcionara/sockets/ -c -Wall $(CFLAGS) -fpic /home/utnso/tp-2019-2c-Lo-dejaria-todo-porque-funcionara/suse/hilolay_alumnos.c -o $(BUILD)/$(NAME).o -lhilolay
	$(CC) -c -Wall $(CFLAGS) -fpic /home/utnso/tp-2019-2c-Lo-dejaria-todo-porque-funcionara/sockets/sockets.c -o $(BUILD)/socketssuse.o -lcommons
  $(CC) -c -Wall $(CFLAGS) -fpic /home/utnso/tp-2019-2c-Lo-dejaria-todo-porque-funcionara/sockets/sockets.c -o $(BUILD)/socketsmuse.o -lcommons


$(NAME).so: $(NAME).o
	$(CC) -shared -o $(BUILD)/lib$(NAME).so $(BUILD)/$(NAME).o -lhilolay
	$(CC) -shared -o $(BUILD)/libsocketssuse.so $(BUILD)/socketssuse.o -lcommons
  $(CC) -shared -o $(BUILD)/libsocketsmuse.so $(BUILD)/socketsmuse.o -lcommons


entrega:
	$(CC) -L./$(BUILD)/ -I /home/utnso/tp-2019-2c-Lo-dejaria-todo-porque-funcionara/sockets/  -Wall $(CFLAGS) -o $(BUILD)/archivo_de_swap_supermasivo archivo_de_swap_supermasivo.c libmuse.c -l$(NAME) - lsocketssusesuse -lcommons -lsocketsmuse
	$(CC) -L./$(BUILD)/ -I /home/utnso/tp-2019-2c-Lo-dejaria-todo-porque-funcionara/sockets/ -Wall $(CFLAGS) -o $(BUILD)/audiencia audiencia.c -l$(NAME) -lhilolay - lsocketssuse -lsocketsmuse 
	$(CC) -L./$(BUILD)/ -I /home/utnso/tp-2019-2c-Lo-dejaria-todo-porque-funcionara/sockets/ -Wall $(CFLAGS) -o $(BUILD)/caballeros_de_SisOp_Afinador caballeros_de_SisOp_Afinador.c -l$(NAME) -lhilolay - lsocketssuse -lsocketsmuse
	$(CC) -L./$(BUILD)/ -I /home/utnso/tp-2019-2c-Lo-dejaria-todo-porque-funcionara/sockets/ -Wall $(CFLAGS) -o $(BUILD)/caballeros_de_SisOp_Solo caballeros_de_SisOp_Solo.c -l$(NAME) -lhilolay - lsocketssuse -lsocketsmuse
	$(CC) -L./$(BUILD)/ -I /home/utnso/tp-2019-2c-Lo-dejaria-todo-porque-funcionara/sockets/  -Wall $(CFLAGS) -o $(BUILD)/estres_compartido estres_compartido.c libmuse.c -l$(NAME) -lcommons  -lhilolay - lsocketssuse -lsocketsmuse
	$(CC) -L./$(BUILD)/ -I /home/utnso/tp-2019-2c-Lo-dejaria-todo-porque-funcionara/sockets/ -Wall $(CFLAGS) -o $(BUILD)/estres_privado estres_privado.c libmuse.c -l$(NAME) -lhilolay - lsocketssuse -lcommons -lsocketsmuse
	$(CC) -L./$(BUILD)/ -I /home/utnso/tp-2019-2c-Lo-dejaria-todo-porque-funcionara/sockets/ -Wall $(CFLAGS) -o $(BUILD)/recursiva recursiva.c libmuse.c -l$(NAME) -lhilolay - lsocketssuse -lcommons -lsocketsmuse
	$(CC) -L./$(BUILD)/ -I /home/utnso/tp-2019-2c-Lo-dejaria-todo-porque-funcionara/sockets/ -Wall $(CFLAGS) -o $(BUILD)/revolucion_compartida revolucion_compartida.c libmuse.c -l$(NAME) -lcommons  -lhilolay - lsocketssuse -lsocketsmuse
	$(CC) -L./$(BUILD)/ -I /home/utnso/tp-2019-2c-Lo-dejaria-todo-porque-funcionara/sockets/ -Wall $(CFLAGS) -o $(BUILD)/revolucion_privada revolucion_privada.c libmuse.c -l$(NAME) -lhilolay - lsocketssuse -lcommons -lsocketsmuse

archivo_de_swap_supermasivo:
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):./$(BUILD) ./$(BUILD)/archivo_de_swap_supermasivo

audiencia:
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):./$(BUILD) ./$(BUILD)/audiencia

caballeros_de_SisOp_Afinador:
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):./$(BUILD) ./$(BUILD)/caballeros_de_SisOp_Afinador

caballeros_de_SisOp_Solo:
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):./$(BUILD) ./$(BUILD)/caballeros_de_SisOp_Solo

estres_compartido:
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):./$(BUILD) ./$(BUILD)/estres_compartido

estres_privado:
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):./$(BUILD) ./$(BUILD)/estres_privado

recursiva:
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):./$(BUILD) ./$(BUILD)/recursiva

revolucion_compartida:
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):./$(BUILD) ./$(BUILD)/revolucion_compartida

revolucion_privada:
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):./$(BUILD) ./$(BUILD)/revolucion_privada
