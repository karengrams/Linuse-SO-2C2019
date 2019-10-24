#include "utilsSockets.h"
#include "utils.c"


void ipCliente(int socketCli, char* ipCli){
		struct sockaddr_in addr;
		socklen_t addr_size = sizeof(struct sockaddr_in);
		getpeername(socketCli, (struct sockaddr *)&addr, &addr_size);
		strcpy(ipCli,inet_ntoa(addr.sin_addr)) ; //Me guardo la ip del cliente que estoy trantando en este momento
}



int crear_conexion(char *ip, char* puerto)
{
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


void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(op_code codigo)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{

	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}


void* serializar_paquete(t_paquete* paquete, int bytes)
{
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


void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)

{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
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

	t_paquete* paquete_respuesta;
	int cod_error;
	int id_cliente, cantidad_de_bytes, flags;
	char* buffer = malloc(30);
	uint32_t direccion_pedida, direccion;
	t_list* lista;
	char* ipCli = (char*)malloc(sizeof(char)*20);
	ipCliente(socketCli, ipCli);

	int cod_op = recibir_operacion(socketCli);
	lista = recibir_paquete(socketCli);

	//t_proceso* cliente_a_atender = buscar_proceso(lista, tabla_de_procesos, ipCli); //lo pongo comentado porque
	//falta la tabladeprocesos.//


		switch(cod_op)
			{
			case DESCONEXION:
				printf("Se desconecto el socket %d\n", socketCli);
				FD_CLR(socketCli, master);
				close(socketCli);
				break;

			case MUSE_INIT:

				//if(cliente_a_atender != NULL){
						//cod_error = -1; YA EXISTE EN NUESTRA TABLA ERROR
						//} else {
				//list_add(tabla_de_procesos, cliente_a_atender); Si no existe lo agregamos
				//cod_error = 0; }

				send(socketCli, &cod_error, sizeof(int), 0);
				break;

			case MUSE_CLOSE:

				// borrar tablas de este proceso.
				//
				close(socketCli);
				break;

			case MUSE_ALLOC:
				id_cliente = *((int*)list_get(lista, 0));
				cantidad_de_bytes = *((int*)list_get(lista, 1));

				printf("MUSE_ALLOC, el proceso %d, de la ip %s nos esta pidiendo "
				"%d bytes de memoria \n", id_cliente,
				ipCli, cantidad_de_bytes);

				//Magia de MUSE

				direccion = &cantidad_de_bytes; //direccion super random para pruebas
				printf("Se le asigna la posicion %x\n", direccion);
				send(socketCli, &direccion, sizeof(uint32_t), 0);
				break;

			case MUSE_FREE:
				id_cliente = *((int*)list_get(lista, 0));
				direccion_pedida = *((uint32_t*)list_get(lista, 1));

				printf("MUSE_FREE, el proceso %d de la ip %s nos esta pidiendo que "
				"liberemos la memoria de la direccion %x \n", id_cliente,
				ipCli, direccion_pedida);
				break;

			case MUSE_GET:
				id_cliente = *((int*)list_get(lista, 0));
				cantidad_de_bytes = *((int*)list_get(lista, 1));
				direccion_pedida = *((uint32_t*)list_get(lista, 2));
				strcpy(buffer, "putita seras vos"); //respuesta random para probar
				//notese que buffer es void*
				printf("MUSE_GET, el proceso %d de la ip %s nos esta pidiendo"
						" %d bytes de la posicion %x de memoria \n", id_cliente,
						ipCli, cantidad_de_bytes, direccion_pedida);
				printf("Le mandamos %s\n", buffer);

				//magia de MUSE

				if (cod_error == -1)
					send(socketCli, &cod_error, sizeof(int), 0);

				paquete_respuesta = crear_paquete(cod_error);
				agregar_a_paquete(paquete_respuesta, buffer, cantidad_de_bytes);
				//literalmente habra que agregar la direccion pedida en el agregar_paquete (despues de
				//chequear que este todo en orden para pasar lo contenido en esa direccion
				enviar_paquete(paquete_respuesta, socketCli);
				eliminar_paquete(paquete_respuesta);
				break;

			case MUSE_CPY:
				id_cliente = *((int*)list_get(lista, 0));
				cantidad_de_bytes = *((int*)list_get(lista, 1));
				memcpy(buffer, list_get(lista,2), cantidad_de_bytes);
				direccion_pedida = *((uint32_t*)list_get(lista, 3));

				printf("MUSE_CPY, el proceso %d de la ip %s quiere "
				"copiar %d bytes en la direccion de memoria %x\n", id_cliente, ipCli,
				cantidad_de_bytes, direccion_pedida);
				printf("Lo que quiere copiar en formato texto es: \n %s \n", (char*)buffer);

				//Magia de MUSE

				send(socketCli, &cod_error, sizeof(int), 0);
				break;

			case MUSE_MAP:
				id_cliente = *((int*)list_get(lista, 0));
				strcpy(buffer, (char*)list_get(lista,1));
				cantidad_de_bytes = *((int*)list_get(lista, 2)); //este seria el length a mappear
				flags = *((int*)list_get(lista, 3));

				printf("MUSE_MAP, el proceso %d de la ip %s quiere mappear %d bytes del archivo del path \n %s \n",
						id_cliente, ipCli, cantidad_de_bytes, (char*)buffer);

				//magia de MUSE
				//guardar la direccion del map en direccion y enviarla

				direccion = &flags; //direccion random para pruebas
				send(socketCli, &direccion, sizeof(uint32_t), 0);
				break;

			case MUSE_SYNC:
				 id_cliente = *((int*)list_get(lista, 0));
				 cantidad_de_bytes = *((int*)list_get(lista, 1)); //cantidad de bytes a guardar en el archivo
				 direccion_pedida = *((uint32_t*)list_get(lista, 2)); //direccion a partir de la cual hacer el sync

				 printf("MUSE_SYNC, el proceso %d de la ip %s quiere sincronizar %d bytes de la direccion %x \n",
				 		id_cliente, ipCli, cantidad_de_bytes, direccion_pedida);

				 //Magia de MUSE

				send(socketCli, &cod_error, sizeof(int), 0);
				break;

			 case MUSE_UNMAP:
				id_cliente = *((int*)list_get(lista, 0));
				direccion_pedida = *((uint32_t*)list_get(lista, 1));

				printf("MUSE_UNMAP, el proceso %d de la ip %s quiere unmappear la direccion %x de memoria \n",
						id_cliente, ipCli, direccion_pedida);

				//Magia de MUSE

				send(socketCli, &cod_error, sizeof(int), 0);
				break;

			}

		free(ipCli);
		free(buffer);

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



