#include "servidor.h"


void iterator(char* value)
	{
		printf("%s\n", value);
	}


int iniciar_socket_escucha(char* ip, char* puerto)

{
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

void admitirNuevoCliente(fd_set *master, int* fdmax, int socketEs){
			struct sockaddr_in remoteaddr;
            int addrlen = sizeof(remoteaddr);
            int newfd = accept(socketEs, (void*)&remoteaddr, &addrlen);
         	FD_SET(newfd, master); // añadir al conjunto maestro
             *fdmax = newfd;
             printf("selectserver: new connection from %s on "
              "socket %d\n", inet_ntoa(remoteaddr.sin_addr), newfd);

}

void atenderCliente(int socketCli){
	t_list* lista;
	int cod_op = recibir_operacion(socketCli);

		switch(cod_op)
			{
			case MENSAJE:
				recibir_mensaje(socketCli);
				break;
			case PAQUETE:
				lista = recibir_paquete(socketCli);
				printf("Me llegaron los siguientes valores:\n del socket %d", socketCli);
				list_iterate(lista, (void*) iterator);
				break;
			}
}


int recibir_operacion(int socket_cliente)
{
	int cod_op;
	recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL);
	return cod_op;
	}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);
	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s, del socket %d", buffer, socket_cliente);
	printf("MENSAJE %s\n", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
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
