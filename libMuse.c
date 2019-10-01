#include "cliente.c"

int SOCKET, ID;
int ERROR = -1;

int muse_init(int id, char* ip, int puerto){

	char* puertoChar = (char*)malloc(sizeof(char)*8);
	sprintf(puertoChar, "%d", puerto); //Pasa el puerto a char* para la func crear_conexion

	int socketCli = crear_conexion(ip, puertoChar);

	if(socketCli == ERROR)
		return socketCli; //si falla tiene que retornar -1


	SOCKET = socketCli;
	ID = id; //Me guardo los datos del proceso que llama a libMuse

	t_paquete* paquete = crear_paquete(MUSE_INIT);
	agregar_a_paquete(paquete, &ID, sizeof(int));
	enviar_paquete(paquete, SOCKET);
	eliminar_paquete(paquete);

	free(puertoChar);
	return 0;
}

void muse_close(){
	t_paquete* paquete = crear_paquete(MUSE_CLOSE);
	agregar_a_paquete(paquete, &ID, sizeof(int));
	enviar_paquete(paquete, SOCKET);
	eliminar_paquete(paquete);
}

uint32_t muse_alloc(uint32_t tam){
	t_paquete* paquete = crear_paquete(MUSE_ALLOC);
	agregar_a_paquete(paquete, &ID, sizeof(int));
	agregar_a_paquete(paquete, &tam, sizeof(uint32_t));
	enviar_paquete(paquete, SOCKET);
	eliminar_paquete(paquete);
	//recibir la direccion del MUSE
	//retornarlo
}

void muse_free(uint32_t dir){
	t_paquete* paquete = crear_paquete(MUSE_FREE);
	agregar_a_paquete(paquete, &ID, sizeof(int));
	agregar_a_paquete(paquete, &dir, sizeof(uint32_t));
	enviar_paquete(paquete, SOCKET);
	eliminar_paquete(paquete);

}
