#include "servidor.h"


void iteratorInts(int* value)
	{
		printf("%d\n", *value);
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
             printf("Nuevo cliente de la ip %s en el "
              "socket %d\n", inet_ntoa(remoteaddr.sin_addr), newfd);

}

void atenderCliente(fd_set* master, int socketCli){

	struct sockaddr_in addr;
	socklen_t addr_size = sizeof(struct sockaddr_in);
	getpeername(socketCli, (struct sockaddr *)&addr, &addr_size);
	char* ipCli = malloc(sizeof(char)*20);
	strcpy(ipCli, inet_ntoa(addr.sin_addr));; //Me guardo la ip del cliente que estoy trantando en este momento

	t_list* lista;
	int cod_op = recibir_operacion(socketCli);

	lista = recibir_paquete(socketCli);

		switch(cod_op)
			{
			case DESCONEXION:
				printf("Se desconecto el socket %d\n", socketCli);
				FD_CLR(socketCli, master);
				close(socketCli);
				break;

			case MUSE_INIT:
				printf("MUSE_INIT, se inician estructuras administrativas "
				"del proceso %d de la ip %s \n", *((int*)lista->head->data), ipCli);

				//Falta inicializar tablas y estructuras administrativas para este proceso.
				break;

			case MUSE_CLOSE:
				printf("MUSE_CLOSE, se liberan "
				"estructuras administrativas del proceso %d "
				"de la ip %s \n", *((int*)lista->head->data), ipCli);
				//cerrar conexion, borrar tablas de este proceso.
				break;

			case MUSE_ALLOC:
				printf("MUSE_ALLOC, el proceso %d, de la ip %s nos esta pidiendo "
				"%d bytes de memoria \n", *((int*)lista->head->data),
				ipCli, *((int*)lista->head->next->data));
				break;

			case MUSE_FREE:
				printf("MUSE_FREE, el proceso %d de la ip %s nos esta pidiendo que "
				"liberemos la memoria de la direccion %x \n", *((int*)lista->head->data),
				ipCli, *((uint32_t*)lista->head->next->data));
				break;
			/*case MUSE_GET:
			case MUSE_CPY:
			case MUSE_MAP:
			case MUSE_SYNC:
			case MUSE_UNMAP:
			*/

			}
		free(ipCli);
}


int recibir_operacion(int socket_cliente)
{
	int cod_op;
	int error = recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL);

	if(error == 0)
			return error;

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
