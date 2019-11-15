#include "libMuse.h"
int SOCKET;
int ID;


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

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

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
	paquete->buffer = (t_buffer*)malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(op_code codigo)
{
	t_paquete* paquete = (t_paquete*)malloc(sizeof(t_paquete));
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













int muse_init(int id, char* ip, int puerto){
	int error;
	char* puertoChar = malloc(sizeof(char)*8);
	sprintf(puertoChar, "%d", puerto); //Pasa el puerto a char* para la func crear_conexion

	int socketCli = crear_conexion(ip, puertoChar);
	free(puertoChar);

	if(socketCli == -1)
		return socketCli; //si falla tiene que retornar -1


	SOCKET = socketCli;
	ID = id; //Me guardo los datos del proceso que llama a libMuse

	t_paquete* paquete = crear_paquete(MUSE_INIT);
	agregar_a_paquete(paquete, &ID, sizeof(int));
	enviar_paquete(paquete, SOCKET);
	eliminar_paquete(paquete);
	recv(SOCKET, &error, sizeof(int), 0);
	return error;
}

void muse_close(){
	t_paquete* paquete = crear_paquete(MUSE_CLOSE);
	agregar_a_paquete(paquete, &ID, sizeof(int));
	enviar_paquete(paquete, SOCKET);
	eliminar_paquete(paquete);
}

uint32_t muse_alloc(uint32_t tam){
	uint32_t direccion ;
	t_paquete* paquete = crear_paquete(MUSE_ALLOC);
	agregar_a_paquete(paquete, &ID, sizeof(int));
	agregar_a_paquete(paquete, &tam, sizeof(uint32_t));
	enviar_paquete(paquete, SOCKET);
	eliminar_paquete(paquete);
	//recibir la direccion del MUSE
	recv(SOCKET, &direccion, sizeof(uint32_t), 0);
	return direccion;
}

void muse_free(uint32_t dir){
	t_paquete* paquete = crear_paquete(MUSE_FREE);
	agregar_a_paquete(paquete, &ID, sizeof(int));
	agregar_a_paquete(paquete, &dir, sizeof(uint32_t));
	enviar_paquete(paquete, SOCKET);
	eliminar_paquete(paquete);
	//MUSE no responde nada.
}

int muse_get(void* dst, uint32_t src, size_t n){
	int error;
	t_paquete* paquete = crear_paquete(MUSE_GET);
	agregar_a_paquete(paquete, &ID, sizeof(int));
	agregar_a_paquete(paquete, &n, sizeof(int)); //primero el tamanio
	agregar_a_paquete(paquete, &src, sizeof(uint32_t)); //despues la direccion de la cual quiero recuperar datos
	enviar_paquete(paquete, SOCKET);
	eliminar_paquete(paquete);

	error = recibir_operacion(SOCKET);

	if (error == -1)
		return error;

	t_list* lista = recibir_paquete(SOCKET);

	memcpy(dst, (char*)list_get(lista, 0), n); //recibir paquete con datos pedidos y copiarlo en dst con memcpy

	return error;
}

int muse_cpy(uint32_t dst, void* src, int n){
	int error;
	t_paquete* paquete = crear_paquete(MUSE_CPY);
	agregar_a_paquete(paquete, &ID, sizeof(int));
	agregar_a_paquete(paquete, &n, sizeof(int)); //primero el tamanio. Sera necesario??? ya estoy acortandolo en el agregar a paquete...
	agregar_a_paquete(paquete, src, n); //n bytes de src que van a ser guardados en la memoria de MUSE
	agregar_a_paquete(paquete, &dst, sizeof(uint32_t)); //direccion donde lo va a guardar
	enviar_paquete(paquete, SOCKET);
	eliminar_paquete(paquete);

	recv(SOCKET, &error, sizeof(int), 0);

	return error;
}

uint32_t muse_map(char *path, size_t length, int flags){
	printf("%s\n", path);
	uint32_t direccion;
	t_paquete* paquete = crear_paquete(MUSE_MAP);
	agregar_a_paquete(paquete, &ID, sizeof(int));
	agregar_a_paquete(paquete, path, strlen(path)+1); //path del archivo
	agregar_a_paquete(paquete, &length, sizeof(size_t));
	agregar_a_paquete(paquete, &flags, sizeof(int));
	enviar_paquete(paquete, SOCKET);
	eliminar_paquete(paquete);

	recv(SOCKET, &direccion, sizeof(uint32_t), 0);
	return direccion;
}

int muse_sync(uint32_t addr, size_t len){
	int error;

	t_paquete* paquete = crear_paquete(MUSE_SYNC);
	agregar_a_paquete(paquete, &ID, sizeof(int));
	agregar_a_paquete(paquete, &len, sizeof(size_t));
	agregar_a_paquete(paquete, &addr, sizeof(uint32_t));
	enviar_paquete(paquete, SOCKET);
	eliminar_paquete(paquete);

	recv(SOCKET, &error, sizeof(int), 0);
	return error;
}

int muse_unmap(uint32_t dir){
	int error;

	t_paquete* paquete = crear_paquete(MUSE_UNMAP);
	agregar_a_paquete(paquete, &ID, sizeof(int));
	agregar_a_paquete(paquete, &dir, sizeof(uint32_t));
	enviar_paquete(paquete, SOCKET);
	eliminar_paquete(paquete);

	recv(SOCKET, &error, sizeof(int), 0);
	return error;
}
