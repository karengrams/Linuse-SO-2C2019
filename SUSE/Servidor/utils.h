#ifndef UTILS_H_
	#define UTILS_H_

	#include<stdio.h>
	#include<stdlib.h>
	#include<sys/socket.h>
	#include<unistd.h>
	#include<netdb.h>
	#include<commons/log.h>
	#include<commons/collections/list.h>
	#include<string.h>

typedef enum {
	DESCONEXION = -1,
	SUSE_INIT = 1,
	SUSE_SCHEDULE = 2,
	SUSE_JOIN = 3,
	SUSE_CLOSE = 4,
	SUSE_WAIT = 5,
	SUSE_SIGNAL = 6,
} op_code;

	void* recibir_buffer(int*, int);

	int iniciar_servidor(char*, char*);
	int esperar_cliente(int);
	t_list* recibir_paquete(int);
	void recibir_mensaje(int);
	int recibir_operacion(int);

#endif
