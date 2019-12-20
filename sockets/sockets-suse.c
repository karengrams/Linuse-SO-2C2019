/*
 * sockets.c
 *
 *  Created on: 16 nov. 2019
 *      Author: utnso
 */
#include "sockets-suse.h"
#include <netinet/in.h>
#include <arpa/inet.h>
typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

int esperar_cliente(int socket_servidor){
	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);
	int socket_cliente = accept(socket_servidor, (SA*) &dir_cliente, (socklen_t*)&tam_direccion);
	return socket_cliente;
}

int recibir_operacion(int socket_cliente){
	int cod_op;
	int error = recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL);

	if(error == 0)
		return -1;

	return cod_op;
}

void* recibir_buffer(int* size, int socket_cliente){
	void * buffer;
	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);
	return buffer;
}

t_list* recibir_paquete(int socket_cliente){
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;

}


int iniciar_socket_muse(int port){
	int server_socket;
	SA_IN server_addr,client_addr;
	server_socket=socket(AF_INET,SOCK_STREAM,0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr=INADDR_ANY;
	server_addr.sin_port=htons(port);
	bind(server_socket,(SA*)&server_addr,sizeof(server_addr));
	listen(server_socket,1);
	return server_socket;
}


int iniciar_socket(/*char* ip, char* port*/int port){
	int server_socket;
	SA_IN server_addr;
	server_socket=socket(AF_INET,SOCK_STREAM,0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr=INADDR_ANY;
	server_addr.sin_port=htons(port);
	bind(server_socket,(SA*)&server_addr,sizeof(server_addr));
	listen(server_socket,1);
	return server_socket;
}



void ipCliente(int socketCli, char* ipCli){
		struct sockaddr_in addr;
		socklen_t addr_size = sizeof(struct sockaddr_in);
		getpeername(socketCli, (struct sockaddr *)&addr, &addr_size);
		strcpy(ipCli,inet_ntoa(addr.sin_addr)) ; //Me guardo la ip del cliente que estoy trantando en este momento
}

int crear_conexion(char *ip, char* puerto){
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
		printf("error");

	freeaddrinfo(server_info);

	return socket_cliente;
}

void crear_buffer(t_paquete* paquete){
	paquete->buffer = (t_buffer*)malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(int codigo){
	t_paquete* paquete = (t_paquete*)malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio){

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void* serializar_paquete(t_paquete* paquete, int bytes){
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void enviar_paquete(t_paquete* paquete, int socket_cliente){
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete){
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

t_proceso* crear_proceso(int id, char* ip){
	t_proceso* proceso = (t_proceso*)malloc(sizeof(t_proceso));
	proceso->id = id;
	proceso->ip = string_duplicate(ip);
	proceso->tablaDeSegmentos = list_create();
	proceso->totalMemoriaLiberada=0;
	proceso->totalMemoriaPedida=0;
	return proceso;
}

void admitir_nuevo_cliente(fd_set *master, int* fdmax, int socketEs){
			struct sockaddr_in remoteaddr;
            int addrlen = sizeof(remoteaddr);
            int newfd = accept(socketEs, (void*)&remoteaddr, (void*)&addrlen);
         	FD_SET(newfd, master); // añadir al conjunto maestro
             *fdmax = newfd;
             printf("Nuevo cliente de la ip %s en el socket %d\n", inet_ntoa(remoteaddr.sin_addr), newfd);
}

void liberar_conexion(int socket_cliente) {
	close(socket_cliente);
}


