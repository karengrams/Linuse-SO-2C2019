/*ip y puerto random*/

#include "servidor.c"

int main(){
	fd_set master;   // conjunto maestro de descriptores de fichero
	fd_set read_fds; // conjunto temporal para lectura de descriptores de fichero para select()
	int fdmax;        // Ultimo socket recibido
	FD_ZERO(&master);    // borra los conjuntos maestro y temporal
	FD_ZERO(&read_fds);
	int socketEs = iniciar_socket_escucha("127.0.0.1", "44444"); // obtener socket para listen

    FD_SET(socketEs, &master);// añadir socketEscucha al conjunto maestro
    fdmax = socketEs;


while(1){
       read_fds = master;
       select(fdmax+1, &read_fds, NULL, NULL, NULL);
     for(int i = 0; i <= fdmax; i++) { // explorar conexiones existentes en busca de datos que leer
         if (FD_ISSET(i, &read_fds)) { //Hay datos que leer...
           if (i == socketEs) { //si se recibe en el socket escucha hay nuevas conexiones que aceptar
        	   	   	admitirNuevoCliente(&master, &fdmax, i); //agregar al master los nuevos clientes
           	   	  } else {
           	   		atenderCliente(&master, i); //leer mensajes y paquetes e imprimirlos por consola
           	   	  }
         	 }
     	 }
	}
}


