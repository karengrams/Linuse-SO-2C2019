#include "suse.h"
#include "utils.c"

t_list* colaNEW;
t_list* colaREADY;
t_list* listaEXEC;
t_list* listaEXIT;
t_list* listaBLOCKED;
t_config* CONFIG;


char* puerto_listen(){
	return config_get_string_value(CONFIG, "LISTEN_PORT");
}

int grado_de_multiprogramacion_maximo(){
	return config_get_int_value(CONFIG, "MAX_MULTIPROG");
}

int alpha_sjf(){
	return config_get_int_value(CONFIG, "ALPHA_SJF");
}

bool _not_null(void* elem){
	t_execute* elemento = (t_execute*) elem;
	return (elemento->thread != NULL);
}

void* _suma_hilos(void* seed, void* elem){
	int inicial = *((int*) seed);
	t_cola_ready* elemento = (t_cola_ready*) elem;
	inicial =+ list_size(elemento->lista_threads);
	memcpy(seed, &inicial, sizeof(int));
	return seed;
}


int total_hilos_en_ready_y_exec(){
	int seed = 0;
	int ready = *((int*)list_fold(colaREADY,&seed, &_suma_hilos));
	int exec = list_size(list_filter(listaEXEC, &_not_null));
	return ready+exec;
}

bool podemos_agregar_hilos_a_ready(){
	return total_hilos_en_ready_y_exec() < grado_de_multiprogramacion_maximo();
}





void crear_entrada_en_cola_ready(int fd){
	t_cola_ready* entradaNueva = malloc(sizeof(t_cola_ready));
	entradaNueva->socket_fd = fd;
	entradaNueva->lista_threads = list_create();
	list_add(colaREADY, entradaNueva);
}

void crear_entrada_en_lista_execute(int fd){
	t_execute* entradaNueva = malloc(sizeof(t_execute));
	entradaNueva->socket_fd = fd;
	entradaNueva->thread = NULL;
	list_add(listaEXEC, entradaNueva);
}

void crear_thread(int fd, int tid){
	t_thread* nuevoThread = malloc(sizeof(t_thread));
	nuevoThread->socket_fd = fd;
	nuevoThread->tid = tid;
	nuevoThread->tiempo_creacion = time();
	nuevoThread->tiempo_ejecucion = 0;
	nuevoThread->tiempo_en_cola_actual = 0;
	nuevoThread->tiempo_total_en_exec = 0;
	nuevoThread->tiempo_total_en_ready = 0;
	nuevoThread->ultima_estimacion = 0;
	nuevoThread->ultima_rafaga=0;
	list_add(colaNEW, nuevoThread);
}

void atenderCliente(void* elemento){
	int socketCli = *((int*)elemento);
	t_list* paqueteRecibido = NULL, *colaAAplicarSJF;
	t_execute* hiloEnEjecucion;
	int tid;

	while(1){
		int cod_op = recibir_operacion(socketCli);

		switch(cod_op){

		case SUSE_INIT:
			paqueteRecibido = recibir_paquete(socketCli);
			tid = *((int*)paqueteRecibido->head->data);
			if(!tid){ //si el tid es 0 es el programa principal
				crear_entrada_en_cola_ready(socketCli);
				crear_entrada_en_lista_execute(socketCli);
			}
				crear_thread(socketCli, tid);

			break;

		case SUSE_SCHEDULE:
			bool _mismo_fd(void* elem){
				t_cola_ready* elemento = (t_cola_ready*)elem;
				return elemento->socket_fd == socketCli;
			}
			colaAAplicarSJF = ((t_cola_ready*)list_find(colaREADY, &_mismo_fd))->lista_threads;
			hiloEnEjecucion = (t_execute*)list_find(listaEXEC, &_mismo_fd);


			break;
		case SUSE_CLOSE:
			break;
		case SUSE_JOIN:
			break;
		case SUSE_SIGNAL:
			break;
		case SUSE_WAIT:
			break;
		}
	}
}




int main(){
	CONFIG = config_create("suse.config");
	colaNEW = list_create();
	listaEXIT = list_create();
	listaBLOCKED = list_create();
	colaREADY = list_create();
	listaEXEC = list_create();




	pthread_t hilo;
	int socketEscucha = iniciar_servidor(puerto_listen(),"127.0.0.1");
	int cliente;

	while(1){
		cliente = esperar_cliente(socketEscucha);
		pthread_create(&hilo, NULL, atenderCliente, &cliente);
	}
}
