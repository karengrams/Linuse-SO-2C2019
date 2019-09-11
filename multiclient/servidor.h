#ifndef SERVIDOR_H_
#define SERVIDOR_H_

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

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

t_log* logger;

int iniciar_socket_escucha(char* ip, char* puerto);
void admitirNuevoCliente(fd_set *master, int* fdmax, int socketEs);
void atenderCliente(int socketCli);
void* recibir_buffer(int*, int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);




#endif /* SERVIDOR_H_ */
