/*Basicamente es el codigo del tp0 con un ip y puerto ya fijados en codigo*/


#include "cliente.h"
#include<readline/readline.h>



int main(){
	int socketCli = crear_conexion("127.0.0.1", "44444");

	while(1){
		t_paquete* paquete = armar_paquete();

			enviar_paquete(paquete, socketCli);


}
	return 1;
}



t_log* iniciar_logger() {
	return log_create("tp0.log", "TP0", 1, LOG_LEVEL_INFO);
}

void leer_consola(t_log* logger) {
	char* leido = readline(">");

	while(strncmp(leido, "", 1) != 0) {
		log_info(logger, leido);
		free(leido);
		leido = readline(">");
	}

	free(leido);
}

t_paquete* armar_paquete() {
	t_paquete* paquete = crear_paquete();

	char* leido = readline(">");

	while(strncmp(leido, "", 1) != 0) {
		agregar_a_paquete(paquete, leido, strlen(leido) + 1);
		free(leido);
		leido = readline(">");
	}

	free(leido);

	return paquete;
}



