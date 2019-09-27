#include "cliente.c"

int muse_init(int id, char* ip, int puerto){

	char* puertoChar = (char*)malloc(sizeof(char)*8);
	sprintf(puertoChar, "%d", puerto); //Pasa el puerto a char* para la func crear_conexion

	int socketCli = crear_conexion(ip, puertoChar);

	if(socketCli == -1)
		return socketCli; //si falla tiene que retornar -1

	t_paquete* paquete = crear_paquete(MUSE_INIT);
	agregar_a_paquete(paquete, &id, sizeof(int));

	//FALTA AGREGAR EL IP (no el que se pasa por parametro a esta funcion,
	//sino que el propio del proceso que llama a esta funcion) PARA QUE MUSE LO IDENTIFIQUE COMO ID-IP
	//Y no se si agregarlo como string o de que forma.

	enviar_paquete(paquete, socketCli);
	eliminar_paquete(paquete);

	free(puertoChar);
	return 0;
}


