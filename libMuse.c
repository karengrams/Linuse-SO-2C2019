#include "libMuse.h"

int muse_init(int id, char* ip, int puerto){

	char* puertoChar = (char*)malloc(sizeof(char)*8);
	sprintf(puertoChar, "%d", puerto); //Pasa el puerto a char* para la func crear_conexion
	int socketCli = crear_conexion(ip, puertoChar);

	if(socketCli == -1)
		return socketCli; //si falla tiene que retornar -1

	//falta armar paquete con ip y puerto del proceso que llamo a esta funcion
	//y pasarle ese paquete a MUSE para que guarde el IP-ID de cada proceso/hilo

	free(puertoChar);
	return 0;
}


