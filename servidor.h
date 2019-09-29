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



t_log* logger;

int iniciar_socket_escucha(char* ip, char* puerto);
void admitirNuevoCliente(fd_set *master, int* fdmax, int socketEs);
void atenderCliente(fd_set* master, int socketCli);
void* recibir_buffer(int*, int);
t_list* recibir_paquete(int);

int recibir_operacion(int);




#endif /* SERVIDOR_H_ */
