/*
 * sockets.c
 *
 *  Created on: 16 nov. 2019
 *      Author: utnso
 */
#include "sockets.h"

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

	if(connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
		printf("error");
		return -1;
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

void crear_buffer(t_paquete* paquete){
	paquete->buffer = (t_buffer*)malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(op_code codigo){
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

int iniciar_socket_escucha(char* ip, char* puerto){
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(ip, puerto, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            continue;
        }
        break;
    }

	listen(socket_servidor, SOMAXCONN);

    freeaddrinfo(servinfo);

    return socket_servidor;
}

int recibir_operacion(int socket_cliente){
	int cod_op;
	int error = recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL);

	if(error == 0)
			return error;

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
	while(desplazamiento < size){
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = (char*)malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

t_proceso* crear_proceso(int id, char* ip){
	t_proceso* proceso = (t_proceso*)malloc(sizeof(t_proceso));
	proceso->id = id;
	proceso->ip = string_duplicate(ip);
	proceso->tablaDeSegmentos = list_create();
	return proceso;
}

t_proceso* buscar_proceso(t_list* paqueteRecibido, char* ipCliente){
	int id = *((int*) list_get(paqueteRecibido, 0)); // ACA se muere?

	bool mismoipid(void* arg) {
		t_proceso* cliente = (t_proceso*) arg;
		return ((cliente->id) == id && !(strcmp(ipCliente, cliente->ip)));
	}

	return list_find(PROCESS_TABLE, mismoipid);

}

void admitir_nuevo_cliente(fd_set *master, int* fdmax, int socketEs){
			struct sockaddr_in remoteaddr;
            int addrlen = sizeof(remoteaddr);
            int newfd = accept(socketEs, (void*)&remoteaddr, &addrlen);
         	FD_SET(newfd, master); // añadir al conjunto maestro
             *fdmax = newfd;
             printf("Nuevo cliente de la ip %s en el socket %d\n", inet_ntoa(remoteaddr.sin_addr), newfd);
}

void atender_cliente(fd_set* master, int socketCli){
	t_paquete* paquete_respuesta = NULL;
	int cod_error;
	int id_cliente, cantidad_de_bytes, flags;
	void* buffer = NULL;
	t_proceso* cliente_a_atender = NULL;
	uint32_t direccion_pedida, direccion;
	t_list* paqueteRecibido = NULL;
	char* ipCli = (char*)malloc(sizeof(char)*20);
	ipCliente(socketCli, ipCli);

	int cod_op = recibir_operacion(socketCli);
	printf("Codigo operacion %d\n", cod_op);

	if(cod_op!=DESCONEXION){
		paqueteRecibido = recibir_paquete(socketCli);
		cliente_a_atender=buscar_proceso(paqueteRecibido, ipCli);
		switch(cod_op){
		case MUSE_INIT:
			id_cliente = *((int*)list_get(paqueteRecibido, 0));
			cod_error = muse_init(cliente_a_atender, ipCli, id_cliente);
			send(socketCli, &cod_error, sizeof(int), 0);
		break;
		case MUSE_CLOSE:
			muse_close(cliente_a_atender);
			FD_CLR(socketCli, master);
			close(socketCli);
		break;
		case MUSE_ALLOC:
			direccion = muse_alloc(cliente_a_atender, *((int*)list_get(paqueteRecibido,1)));
			send(socketCli, &direccion, sizeof(uint32_t), 0);
		break;
		case MUSE_FREE:
			id_cliente = *((int*)list_get(paqueteRecibido, 0));
			direccion_pedida = *((uint32_t*)list_get(paqueteRecibido, 1));
			muse_free(cliente_a_atender,direccion_pedida);
		break;
		case MUSE_GET: // TODO: ver caso de que se pase del limite
			cantidad_de_bytes = *((int*) list_get(paqueteRecibido, 1));
			buffer = muse_get(cliente_a_atender, paqueteRecibido);
			if (buffer == NULL){
				cod_error = -1;
				send(socketCli, &cod_error, sizeof(int), 0);
			} else {
				paquete_respuesta = crear_paquete(10);
				agregar_a_paquete(paquete_respuesta, buffer, cantidad_de_bytes);
				enviar_paquete(paquete_respuesta, socketCli);
				eliminar_paquete(paquete_respuesta);
			}
		break;
		case MUSE_CPY:
				cod_error = muse_cpy(cliente_a_atender, paqueteRecibido);
				send(socketCli, &cod_error, sizeof(int), 0);
		break;
		case MUSE_MAP:
			id_cliente = *((int*)list_get(paqueteRecibido, 0));
			buffer = (char*)malloc(sizeof((char*)list_get(paqueteRecibido,1)));
			strcpy(buffer, (char*)list_get(paqueteRecibido,1));
			cantidad_de_bytes = *((int*)list_get(paqueteRecibido, 2)); //este seria el length a mappear
			flags = *((int*)list_get(paqueteRecibido, 3));
			direccion = muse_map(cliente_a_atender,buffer,cantidad_de_bytes,flags);
			send(socketCli, &direccion, sizeof(uint32_t), 0);
		break;
		case MUSE_SYNC:
			id_cliente = *((int*)list_get(paqueteRecibido, 0));
			cantidad_de_bytes = *((int*)list_get(paqueteRecibido, 1)); //cantidad de bytes a guardar en el archivo
			direccion_pedida = *((uint32_t*)list_get(paqueteRecibido, 2)); //direccion a partir de la cual hacer el sync
			muse_sync(cliente_a_atender,direccion_pedida,(size_t)cantidad_de_bytes);

			send(socketCli, &cod_error, sizeof(int), 0);
		break;
		case MUSE_UNMAP:
			id_cliente = *((int*)list_get(paqueteRecibido, 0));
			direccion_pedida = *((uint32_t*)list_get(paqueteRecibido, 1));
			muse_unmap(cliente_a_atender,direccion_pedida);
			send(socketCli, &cod_error, sizeof(int), 0);
		break;
		}
	}
	free(ipCli);
	free(buffer);
		//free(paqueteRecibido); //TODO:fijarse como eliminar la lista de las commons

}
