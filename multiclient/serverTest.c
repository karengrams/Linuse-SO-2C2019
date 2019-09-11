/*ip y puerto random*/

#include "servidor.h"

int main(){
	fd_set master;   // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal para lectura de descriptores de fichero para select()
	int fdmax;        // Ultimo socket recibido
	int newfd;        // descriptor de socket de nueva conexión aceptada
	char buf[256];    // buffer para datos del cliente
	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);

	int socketEs = iniciar_socket_escucha("127.0.0.1", "44444"); // obtener socket para listen

    FD_SET(socketEs, &master);// añadir socketEscucha al conjunto maestro
    fdmax = socketEs;


while(1){
       read_fds = master;
       select(fdmax+1, &read_fds, NULL, NULL, NULL);
     for(int i = 0; i <= fdmax; i++) { // explorar conexiones existentes en busca de datos que leer
         if (FD_ISSET(i, &read_fds)) { //Hay datos que leer...
           if (i == socketEs) { //si se recibe en el socket escucha hay nuevas conexiones que aceptar
        	   	   	admitirNuevoCliente(&master, &fdmax, socketEs); //agregar al master los nuevos clientes
           	   	  } else {
           	   		atenderCliente(i); //leer mensajes y paquetes e imprimirlos por consola
      	   	  }
       	 	}
       	 }
       }
}


