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
#include <string.h>
#include <stdbool.h>
#include <semaphore.h>
#include <commons/bitarray.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include "sockets-structures.h"

#define ERROR -1;

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

int esperar_cliente(int);
int recibir_operacion(int);
void* recibir_buffer(int*, int);
t_list* recibir_paquete(int);
int iniciar_socket(int);
void ipCliente(int, char*);
int crear_conexion(char *, char*);
void crear_buffer(t_paquete*);
t_paquete* crear_paquete(int);
void agregar_a_paquete(t_paquete*, void*, int);
void* serializar_paquete(t_paquete* , int);
void enviar_paquete(t_paquete*, int);
void eliminar_paquete(t_paquete*);
t_proceso* crear_proceso(int, char*);
void liberar_proceso(t_proceso*);
void admitir_nuevo_cliente(fd_set*, int*, int);
void liberar_conexion(int);

#endif /* SOCKETS_SOCKETS_H_ */
