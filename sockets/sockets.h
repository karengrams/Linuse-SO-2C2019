/*
 * sockets.h
 *
 *  Created on: 16 nov. 2019
 *      Author: utnso
 */

#ifndef SOCKETS_SOCKETS_H_
#define SOCKETS_SOCKETS_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include <stdbool.h>
#include <commons/bitarray.h>
#include "../paginacion/paginacion.h"
#include "../paginacion/frames.h"
#include "../segmentacion/segmentacion.h"
#define ERROR -1;

t_list* PROCESS_TABLE;
t_list* MAPPED_SHARED_FILES;

typedef struct{
	int id;
	char* ip;
	int totalMemoriaPedida;
	int totalMemoriaLiberada;
	t_list* tablaDeSegmentos;
} t_proceso;

typedef enum {
	DESCONEXION = 0,
	MUSE_INIT = 10,
	MUSE_ALLOC = 11,
	MUSE_FREE = 12,
	MUSE_GET = 13,
	MUSE_CPY = 14,
	MUSE_MAP = 15,
	MUSE_SYNC = 16,
	MUSE_UNMAP = 17,
	MUSE_CLOSE = 18,
} op_code;

typedef struct{
	int size;
	void* stream;
}t_buffer;

typedef struct{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

void ipCliente(int, char*);
int crear_conexion(char *, char*);
void crear_buffer(t_paquete*);
t_paquete* crear_paquete(op_code);
void agregar_a_paquete(t_paquete*, void*, int);
void* serializar_paquete(t_paquete* , int);
void enviar_paquete(t_paquete*, int);
void eliminar_paquete(t_paquete*);
int iniciar_socket_escucha(char*, char*);
int recibir_operacion(int);
void* recibir_buffer(int*, int);
t_list* recibir_paquete(int);
t_proceso* crear_proceso(int, char*);
t_proceso* buscar_proceso(t_list*, char*);
int posicion_en_lista_proceso(t_proceso*);
void liberar_proceso(t_proceso*);
void admitir_nuevo_cliente(fd_set*, int*, int);
void atender_cliente(fd_set*, int);


#endif /* SOCKETS_SOCKETS_H_ */
