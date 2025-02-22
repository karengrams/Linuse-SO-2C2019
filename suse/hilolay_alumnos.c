#include "hilolay_alumnos.h"

int SOCKETSUSE;

int suse_create(int tid){

	t_paquete* paquete = crear_paquete(SUSE_INIT);
	agregar_a_paquete(paquete, &tid, sizeof(int));
	enviar_paquete(paquete,SOCKETSUSE);
	eliminar_paquete(paquete);
	return 0;
}

int suse_schedule_next(){
	int codigo = SUSE_SCHEDULE;
	int tid;
	send(SOCKETSUSE, &codigo, sizeof(int), 0);
	recv(SOCKETSUSE, &tid, sizeof(int), 0);

	return tid;
}

int suse_join(int tid){
	t_paquete* paquete = crear_paquete(SUSE_JOIN);
	agregar_a_paquete(paquete, &tid, sizeof(int));
	enviar_paquete(paquete,SOCKETSUSE);
	eliminar_paquete(paquete);
	return 0;
}

int suse_wait(int tid, char* idSemaforo){
	int exito;

	t_paquete* paquete = crear_paquete(SUSE_WAIT);
	agregar_a_paquete(paquete, &tid, sizeof(int));
	agregar_a_paquete(paquete, idSemaforo, (strlen(idSemaforo)+1));
	enviar_paquete(paquete, SOCKETSUSE);
	eliminar_paquete(paquete);

	recv(SOCKETSUSE, &exito, sizeof(int), 0);

	return exito;
}

int suse_signal(int tid, char* idSemaforo){
	t_paquete* paquete = crear_paquete(SUSE_SIGNAL);
	agregar_a_paquete(paquete, &tid, sizeof(int));
	agregar_a_paquete(paquete, idSemaforo, (strlen(idSemaforo)+1));
	enviar_paquete(paquete, SOCKETSUSE);
	eliminar_paquete(paquete);
	return 0;
}

int suse_close(int tid){
	t_paquete* paquete = crear_paquete(SUSE_CLOSE);
	agregar_a_paquete(paquete, &tid, sizeof(int));
	enviar_paquete(paquete, SOCKETSUSE);
	eliminar_paquete(paquete);
	int error;
	recv(SOCKETSUSE, &error, sizeof(int),0);
	//return 0;
}

static struct hilolay_operations hiloops = {
		.suse_create = &suse_create,
		.suse_schedule_next = &suse_schedule_next,
		.suse_join = &suse_join,
		.suse_close = &suse_close,
		.suse_wait = &suse_wait,
		.suse_signal = &suse_signal
};

void hilolay_init(){

	//iniciar conexion con suse
	SOCKETSUSE = crear_conexion("192.168.0.21", getenv("PUERTO"));
	init_internal(&hiloops);

}
