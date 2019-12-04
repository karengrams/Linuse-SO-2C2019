#include "hilolay_alumnos.h"
#include "utils-cli.h"

int SOCKET;


void hilolay_init(){

	//iniciar conexion con suse
	SOCKET = crear_conexion("127.0.0.1", "48480"); //De donde sacamos el puerto?
	init_internal(main_ops); //esta funcion crea el hilo del programa padre y llama a suse_create

}

int suse_create(int tid){
	t_paquete* paquete = crear_paquete(SUSE_INIT);
	agregar_a_paquete(paquete, &tid, sizeof(int));
	enviar_paquete(paquete,SOCKET);
	eliminar_paquete(paquete);
	return 0;
}

int suse_schedule(){
	int codigo = SUSE_SCHEDULE;
	int tid;
	send(SOCKET, &codigo, sizeof(int), 0);
	recv(SOCKET, &tid, sizeof(int), 0);
	return tid;
}

int suse_join(int tid){
	t_paquete* paquete = crear_paquete(SUSE_JOIN);
	agregar_a_paquete(paquete, &tid, sizeof(int));
	enviar_paquete(paquete,SOCKET);
	eliminar_paquete(paquete);
	return 0;
}

int suse_wait(int tid, char* idSemaforo){
	int exito;

	t_paquete* paquete = crear_paquete(SUSE_WAIT);
	agregar_a_paquete(paquete, &tid, sizeof(int));
	agregar_a_paquete(paquete, idSemaforo, (strlen(idSemaforo)+1));
	enviar_paquete(paquete, SOCKET);
	eliminar_paquete(paquete);

	recv(SOCKET, &exito, sizeof(int), 0);
	return exito;
}

int suse_signal(int tid, char* idSemaforo){
	t_paquete* paquete = crear_paquete(SUSE_SIGNAL);
	agregar_a_paquete(paquete, &tid, sizeof(int));
	agregar_a_paquete(paquete, idSemaforo, (strlen(idSemaforo)+1));
	enviar_paquete(paquete, SOCKET);
	eliminar_paquete(paquete);
	return 0;
}

int suse_close(int tid){
	t_paquete* paquete = crear_paquete(SUSE_CLOSE);
	agregar_a_paquete(paquete, &tid, sizeof(int));
	enviar_paquete(paquete, SOCKET);
	eliminar_paquete(paquete);
	return 0;
}
