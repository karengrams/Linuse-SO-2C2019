
#include "libMuse.h"
int SOCKET;
int ID;

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
