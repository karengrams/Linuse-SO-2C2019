#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include "suse.h"
#include "utils.h"

t_list* colaNEW;
t_list* colaREADY;
t_list* listaEXEC;
t_list* listaEXIT;
t_list* listaBLOCKED;

t_list* LISTA_SEMAFOROS;

t_config* CONFIG;
t_log* LOG;

int* SEM_VALOR;
int* SEM_MAX;
char** SEM_IDS;

sem_t sem_new;
sem_t sem_ready;
sem_t sem_exit;
sem_t sem_run;
sem_t sem_blocked;
sem_t sem_execute;
sem_t semaforos_suse; //deberia ser un array de semaforos para ser mas precisos, despues veo

struct timeval TIEMPO_INICIO_PROGRAMA;

int FUNCIONAR = 1;
int SOCKET_ESCUCHA;


//FUNCIONES DEL CONFIG
bool _not_null(void* elem){
	t_execute* elemento = (t_execute*) elem;
	return (elemento->thread != NULL);
}

int tamanio_vector(char** array){
	char* a = array[0];
	int i = 0;
	while(a!=NULL){
		i++;
		a = array[i];
	}
	return i;
}

int* pasar_a_vector_de_int(char** array){
	int longitud = tamanio_vector(array);
	int* vector = (int*)malloc(longitud*sizeof(int));

	for(int i=0; i<longitud; i++){
		vector[i] = (atoi(array[i]));
		free(array[i]);
	}
	free(array);
	return vector;
}

int* valores_iniciales_semaforos(){
	return pasar_a_vector_de_int(config_get_array_value(CONFIG, "SEM_INIT"));
}

int* valores_maximos_semaforos(){
	return pasar_a_vector_de_int(config_get_array_value(CONFIG, "SEM_MAX"));
}

char** ids_semaforos(){
	return config_get_array_value(CONFIG, "SEM_IDS");
}

int timerLog(){
	return config_get_int_value(CONFIG, "METRICS_TIMER");
}
int* puerto_listen(){
	return config_get_int_value(CONFIG, "LISTEN_PORT");
}

int grado_de_multiprogramacion_maximo(){
	return config_get_int_value(CONFIG, "MAX_MULTIPROG");
}

double alpha_sjf(){
	return config_get_double_value(CONFIG, "ALPHA_SJF");
}



//FUNCIONES DE LOS LOGS
double diferencia_entre_timevals(struct timeval tf, struct timeval ti){
	return (((tf.tv_sec*1000)+(tf.tv_usec/1000))-((ti.tv_sec*1000)+(ti.tv_usec/1000)));
}

double tiempo_que_paso_desde_inicio(){
	struct timeval tiempo;
	gettimeofday(&tiempo, NULL);
	return diferencia_entre_timevals(tiempo, TIEMPO_INICIO_PROGRAMA);
}

double tiempo_que_paso_desde_colaActual(struct timeval cola){
	struct timeval tiempo;
	gettimeofday(&tiempo, NULL);
	return diferencia_entre_timevals(tiempo, cola);
}



void* _suma_hilos(void* seed, void* elem){
	int inicial = *((int*) seed);
	t_cola_ready* elemento = (t_cola_ready*) elem;
	inicial = inicial + list_size(elemento->lista_threads);
	memcpy(seed, &inicial, sizeof(int));
	return seed;
}

int total_hilos_en_ready_y_exec(){
	int seed = 0;
	sem_wait(&sem_ready);
	int ready = *((int*)list_fold(colaREADY,&seed, &_suma_hilos));
	sem_post(&sem_ready);

	sem_wait(&sem_execute);
	t_list* coso = list_filter(listaEXEC, &_not_null);
	sem_post(&sem_execute);
	int exec = list_size(coso);

	list_destroy(coso);
	return ready+exec;
}

//void imprimir_semaforos(){
//	int a, b, c, d, e, f, g;
//	sem_getvalue(&sem_blocked, &a);
//	sem_getvalue(&sem_execute, &b);
//	sem_getvalue(&sem_exit, &c);
//	sem_getvalue(&sem_new, &d);
//	sem_getvalue(&sem_ready, &e);
//	sem_getvalue(&sem_run, &f);
//	sem_getvalue(&semaforos_suse, &g);
//	printf("Semaforos: %d\n%d\n%d\n%d\n%d\n%d\n%d\n", a,b,c,d,e,f,g);
//}

bool podemos_agregar_hilos_a_ready(){
	return total_hilos_en_ready_y_exec() < grado_de_multiprogramacion_maximo();
}


void loggear_semaforos(void){

	int tamanio = tamanio_vector(SEM_IDS);


	for(int i = 0 ; i<tamanio; i++){

		sem_wait(&semaforos_suse);
		log_info(LOG, "Valor del semaforo %s: %d", SEM_IDS[i], SEM_VALOR[i]);
		sem_post(&semaforos_suse);
	}

}


void loggear_procesos(void){

	long double momentoLog = tiempo_que_paso_desde_inicio();

	void* _suma_tiempos_ejecucion(void* seed, void*elem){
		double inicial = *((double*) seed);
		t_thread* hilo = (t_thread*)elem;
		inicial = inicial + (momentoLog - hilo->tiempo_creacion);
		memcpy(seed, &inicial, sizeof(double));
		return seed;
	}

	void* _suma_tiempos_ejecucion_blocked(void* seed, void* elem){
		double inicial = *((double*) seed);
		t_blocked* block = (t_blocked*)elem;
		t_thread* hilo = block->thread;
		inicial = inicial + (momentoLog - hilo->tiempo_creacion);
		memcpy(seed, &inicial, sizeof(double));
		return seed;
	}



	//sem_wait(&sem_ready);
	int tamanio = list_size(colaREADY);
	//sem_post(&sem_ready);

	for(int i=0; i<tamanio; i++){

		//sem_wait(&sem_ready);
		t_cola_ready* ready = list_get(colaREADY, i);
		//sem_post(&sem_ready);

		int hiloEjecutando = 0;
		int fd = ready->socket_fd;

			bool _mismo_fd_en_new(void* elem){
					t_thread* hilo = (t_thread*)elem;
					return hilo->socket_fd == fd;
				}
			bool _mismo_fd_en_blocked(void* elem){
					t_blocked* hilo = (t_blocked*)elem;
					t_thread* coso = hilo->thread;
					return  coso->socket_fd == fd;
				}
			bool _mismo_fd_en_exec(void* elem){
					t_execute* hilo = (t_execute*)elem;
					return hilo->socket_fd == fd;
				}

		//sem_wait(&sem_new);
		t_list* listaNew = list_filter(colaNEW, &_mismo_fd_en_new);
		//sem_post(&sem_new);

		//sem_wait(&sem_blocked);
		t_list* listaBlocked = list_filter(listaBLOCKED, &_mismo_fd_en_blocked);
		//sem_post(&sem_blocked);

		//sem_wait(&sem_execute);
		t_execute* execute = list_find(listaEXEC, &_mismo_fd_en_exec);
		//sem_post(&sem_execute);

		//sem_wait(&sem_exit);
		t_list* listaExit = list_filter(listaEXIT, &_mismo_fd_en_new);
		//sem_post(&sem_exit);

		if(execute->thread != NULL)
			 hiloEjecutando = 1;


		log_info(LOG, "Socket %d:", fd);
		log_info(LOG, "Hilos en cola NEW: %d", list_size(listaNew));
		log_info(LOG, "Hilos en cola READY: %d", list_size(ready->lista_threads));
		log_info(LOG, "Hilos en cola BLOCKED: %d", list_size(listaBlocked));
		log_info(LOG, "Hilos en cola RUN: %d", hiloEjecutando);
		log_info(LOG, "Hilos en cola EXIT: %d\n\n", list_size(listaExit));

		double semilla = 0;
		list_fold(listaNew, &semilla, _suma_tiempos_ejecucion);
		list_fold(ready->lista_threads, &semilla, _suma_tiempos_ejecucion);
		list_fold(listaExit, &semilla, _suma_tiempos_ejecucion);
		list_fold(listaBlocked, &semilla, _suma_tiempos_ejecucion_blocked);

		if(execute->thread!= NULL)
			semilla = semilla + (momentoLog - execute->thread->tiempo_creacion);


		if(execute->thread!= NULL){
			t_thread* hilo = execute->thread;
			double tiempoEjecucion = momentoLog-hilo->tiempo_creacion;
			log_info(LOG, "\nHilo %d: \nTiempo de ejecucion: %f \nTiempo de espera: %f \nTiempo de uso de CPU: %f \nPorcentaje "
			"tiempo de ejecucion: %f", hilo->tid , tiempoEjecucion , (hilo->tiempo_total_en_ready), ((hilo->tiempo_total_en_exec) + tiempo_que_paso_desde_colaActual(hilo->tiempo_en_cola_actual)), tiempoEjecucion/semilla);
			}


		void loggear_threads(void* elem){
			t_thread* hilo = (t_thread*)elem;
			double tiempoEjecucion = momentoLog-hilo->tiempo_creacion;
			log_info(LOG, "\nHilo %d: \nTiempo de ejecucion: %f \nTiempo de espera: %f \nTiempo de uso de CPU: %f \nPorcentaje "
			"tiempo de ejecucion: %f", hilo->tid , tiempoEjecucion , (hilo->tiempo_total_en_ready), (hilo->tiempo_total_en_exec), tiempoEjecucion/semilla);
		}

		void loggear_threads_ready(void* elem){
					t_thread* hilo = (t_thread*)elem;
					double tiempoEjecucion = momentoLog-hilo->tiempo_creacion;
					log_info(LOG, "\nHilo %d: \nTiempo de ejecucion: %f \nTiempo de espera: %f \nTiempo de uso de CPU: %f \nPorcentaje "
					"tiempo de ejecucion: %f", hilo->tid , tiempoEjecucion , ((hilo->tiempo_total_en_ready)+tiempo_que_paso_desde_colaActual(hilo->tiempo_en_cola_actual)), (hilo->tiempo_total_en_exec), tiempoEjecucion/semilla);
				}

		void loggear_blockeds(void* elem){
					t_blocked* block = (t_blocked*)elem;
					t_thread* hilo = block->thread;
					double tiempoEjecucion = momentoLog - hilo->tiempo_creacion;
					log_info(LOG, "\nHilo %d: \nTiempo de ejecucion: %f \nTiempo de espera: %f \nTiempo de uso de CPU: %f \nPorcentaje "
					"tiempo de ejecucion: %f\n\n\n", hilo->tid , tiempoEjecucion , (hilo->tiempo_total_en_ready), (hilo->tiempo_total_en_exec), tiempoEjecucion/semilla);
				}


		list_iterate(listaNew, &loggear_threads);
		list_iterate(listaExit, &loggear_threads);
		list_iterate(ready->lista_threads, &loggear_threads_ready);
		list_iterate(listaBlocked, &loggear_blockeds);

		list_destroy(listaNew);
		list_destroy(listaExit);
		list_destroy(listaBlocked);
	}
}


void escribir_logs(int motivo, int socket){
	if (motivo == -1){
		log_info(LOG, "Timer log ");
	} else {
		log_info(LOG, "Finalizo el hilo %d del socket %d", motivo, socket);
	}



	log_info(LOG, "Grado actual de multiprogramacion: %d", total_hilos_en_ready_y_exec());
	loggear_semaforos();
//
	sem_wait(&sem_blocked);
	sem_wait(&sem_execute);
	sem_wait(&sem_exit);
	sem_wait(&sem_new);
	sem_wait(&sem_ready);
	sem_wait(&sem_run);

	loggear_procesos();

	sem_post(&sem_run);
	sem_post(&sem_ready);
	sem_post(&sem_new);
	sem_post(&sem_exit);
	sem_post(&sem_execute);
	sem_post(&sem_blocked);




}

void escribirLog(int signal){
	escribir_logs(-1, -1);
}



//FUNCIONES AUXILIARES DE SUSE_CREATE

void crear_entrada_en_cola_ready(int fd){
	t_cola_ready* entradaNueva = malloc(sizeof(t_cola_ready));
	entradaNueva->socket_fd = fd;
	entradaNueva->lista_threads = list_create();

	sem_wait(&sem_ready);
	list_add(colaREADY, entradaNueva);
	sem_post(&sem_ready);
}

t_execute* crear_entrada_en_lista_execute(int fd){
	t_execute* entradaNueva = malloc(sizeof(t_execute));
	entradaNueva->socket_fd = fd;
	entradaNueva->thread = NULL;

	sem_wait(&sem_execute);
	list_add(listaEXEC, entradaNueva);
	sem_post(&sem_execute);
	return entradaNueva;
}

t_thread* crear_thread(int fd, int tid){
	t_thread* nuevoThread = malloc(sizeof(t_thread));
	nuevoThread->socket_fd = fd;
	nuevoThread->tid = tid;
	nuevoThread->tiempo_creacion = tiempo_que_paso_desde_inicio();
	(nuevoThread->tiempo_en_cola_actual).tv_sec = 0;
	(nuevoThread->tiempo_en_cola_actual).tv_usec = 0;
	nuevoThread->tiempo_total_en_exec = 0; //cuando sale de exec se le suma time(0) - tiempo_en_cola_actual
	nuevoThread->tiempo_total_en_ready = 0; //cuando sale de ready se le suma time(0) - tiempo_en_cola_actual
	nuevoThread->ultima_estimacion = 0;
	nuevoThread->ultima_rafaga=0; //cuando sale de exec se le asigna time(0) - tiempo_en_cola_actual

	return nuevoThread;
}

void agregar_a_ready(int fd, t_thread* hilo){

	bool _mismo_fd(void* elem){
			t_cola_ready* elemento = (t_cola_ready*)elem;
			return elemento->socket_fd == fd;
		}
	sem_wait(&sem_ready);
	t_cola_ready* nodoReady = list_find(colaREADY, _mismo_fd);
	list_add(nodoReady->lista_threads,hilo);
	sem_post(&sem_ready);
}

//FUNCIONES AUXILIARES DE SUSE_SCHEDULE

void swap_threads(t_execute* nodoExec, t_thread* nuevoHilo){
		sem_wait(&sem_execute);
		nodoExec->thread = nuevoHilo; //agregamos el nuevo hilo a exec
		nuevoHilo->tiempo_total_en_ready =	nuevoHilo->tiempo_total_en_ready + tiempo_que_paso_desde_colaActual(nuevoHilo->tiempo_en_cola_actual); //sumamos el tiempo que estuvo en ready
		gettimeofday(&nuevoHilo->tiempo_en_cola_actual,NULL); //seteamos el tiempo en que entra a exe
		sem_post(&sem_execute);
}

long double rafaga_estimada(void* elem){
	t_thread* hilo = (t_thread*) elem;
	return ((alpha_sjf()*hilo->ultima_rafaga) + ((1-alpha_sjf())*hilo->ultima_estimacion));
}

bool _menor_rafaga_estimada(void* elem, void* elem2){
	return (rafaga_estimada(elem) < rafaga_estimada(elem2));
}

t_thread* algoritmo_SJF(t_list* lista){
	sem_wait(&sem_ready);
	list_sort(lista, &_menor_rafaga_estimada);
	t_thread* hiloElegido = (t_thread*)list_remove(lista, 0);
	hiloElegido->ultima_estimacion = rafaga_estimada(hiloElegido); //le asigno la nueva ultima estiamcion de sjf
	sem_post(&sem_ready);
	return hiloElegido;
}


//FUNCIONES AUXILIARES DEL SUSE_CLOSE
void buscar_hilos_blockeados_por_este(int tid, int fd){ //probablemente no haya o haya solo uno que sea el proceso padre....

	bool _blocked_por_tid(void* elem){
		t_blocked* elemento = (t_blocked*)elem;
		return ((elemento->estado == JOIN) && (elemento->tid == tid) && (elemento->thread->socket_fd == fd));
	}

	sem_wait(&sem_blocked);
	t_list* listaBloqueados = list_filter(listaBLOCKED, &_blocked_por_tid);


	void _cambiar_a_blocked_ready(void* elem){
		t_blocked* elemento = (t_blocked*) elem;
		elemento->estado = BLOCKED_READY;
	}

	list_iterate(listaBloqueados, &_cambiar_a_blocked_ready);
	sem_post(&sem_blocked);
	list_destroy(listaBloqueados);
}

void buscar_y_pasarlo_a_exit(int fd){ //Este codigo quedo asquerosamente feo

	t_thread* hilo ;

	bool _mismo_fd(void* elem){
		t_execute* elemento = (t_execute*)elem;
		return elemento->socket_fd == fd;
	}

	t_execute* execution = list_find(listaEXEC, &_mismo_fd);//lo buscamos en exec

	hilo = execution->thread;
	hilo->tiempo_total_en_exec = hilo->tiempo_total_en_exec + tiempo_que_paso_desde_colaActual(hilo->tiempo_en_cola_actual);
	execution->thread = NULL;

	sem_wait(&sem_exit);
	list_add(listaEXIT, hilo);
	sem_post(&sem_exit);
	}



//FUNCIONES AUXILIARES SUSE_JOIN
t_blocked* crear_entrada_blocked(t_thread* hilo, t_estado estado, int razonBlock){

	t_blocked* entrada = malloc(sizeof(t_blocked));
	entrada->thread = hilo;
	entrada->estado = estado;
	entrada->tid = razonBlock;
	return entrada;
}

bool tid_ya_esta_en_exit(int tid, int fd){
	bool _mismo_tidfd(void*elem){
		t_thread* hilo = (t_thread*)elem;
		return ((hilo->socket_fd == fd) && (hilo->tid == tid));
	}

	sem_wait(&sem_exit);
	bool respuesta = list_any_satisfy(listaEXIT, &_mismo_tidfd);
	sem_post(&sem_exit);

	return respuesta;
}

void buscar_y_pasarlo_a_blocked(int fd,int razon, t_estado estado){
	t_thread* hilo ;

		bool _mismo_fd(void* elem){
			t_execute* elemento = (t_execute*)elem;
			return elemento->socket_fd == fd;
		}

		t_execute* execution = list_find(listaEXEC, &_mismo_fd);//lo buscamos en exec



		hilo = execution->thread;
		hilo->tiempo_total_en_exec = hilo->tiempo_total_en_exec + tiempo_que_paso_desde_colaActual(hilo->tiempo_en_cola_actual);
		hilo->ultima_rafaga = tiempo_que_paso_desde_colaActual(hilo->tiempo_en_cola_actual);
		execution->thread = NULL;
		sem_wait(&sem_blocked);
		t_blocked* blocked = crear_entrada_blocked(hilo, estado, razon);


		list_add(listaBLOCKED, blocked);
		sem_post(&sem_blocked);

}

//FUNCIONES AUXILIARES DE WAIT Y SIGNAL

int posicion_en_vector(char* nombre, char** array){
	int longitud = tamanio_vector(array);
	int i;
	for(i=0; i<longitud; i++){
		if(!strcmp(nombre, array[i]))
			return i;
	}
	return -1;
}

int hacer_signal(int posicion){


	if(SEM_VALOR[posicion]<SEM_MAX[posicion])
		SEM_VALOR[posicion]++;

	if(SEM_VALOR[posicion]<=0){	//Hay threads bloqueados por este semaforo

				bool _blocked_por_sem(void* elem){
					t_blocked* blocked = (t_blocked*)elem;
					return ((blocked->estado == SEMAPHORE) && (blocked->tid == posicion));
				}

			sem_wait(&sem_blocked);
			t_blocked* hilo = list_find(listaBLOCKED, &_blocked_por_sem); //buscamos el primer blockeado

			if(hilo!=NULL)
				hilo->estado = BLOCKED_READY; // y lo ponemos como blocked ready
			sem_post(&sem_blocked);
	}

	return 0;
}

int hacer_wait(int posicion, int fd){


	SEM_VALOR[posicion]--;

	if(SEM_VALOR[posicion]>=0){ //Si es mayor a 0 lo toma
		return 1;
	} else { //Si es menor o igual a 0, lo debe decrementar y bloquearse (buscar la cola exe del fd y desplazar ese thread)
		sem_wait(&sem_execute);
		buscar_y_pasarlo_a_blocked(fd, posicion, SEMAPHORE);
		sem_post(&sem_execute);
		return 0;
	}
}

t_cosa* iniciar_cosa(sem_t* semaforo, int fd){
	t_cosa* cosa = malloc(sizeof(t_cosa));
	cosa->fd = fd;
	cosa->semaforo = semaforo;
	return cosa;
}

//HILO DE ATENCION A CLIENTES, UN HILO POR CADA SOCKET CONECTADO

void atenderCliente(void* elemento){
	int socketCli = *((int*)elemento);

	sem_t semaforoLoco;
	sem_init(&semaforoLoco, 0, 0);
	t_cosa* cosa = iniciar_cosa(&semaforoLoco, socketCli);
	list_add(LISTA_SEMAFOROS, cosa);


	t_list* paqueteRecibido = NULL, *colaAAplicarSJF = NULL;
	t_cola_ready* nodoReady;
	t_execute* nodoEnEjecucion;
	t_thread* hiloAEjecutar, *nuevoThread;
	int tid, posicionEnArray, error, valorsem;
	char* nombreSemaforo;
	bool _mismo_fd(void* elem){
			t_cola_ready* elemento = (t_cola_ready*)elem;
			return elemento->socket_fd == socketCli;
		}

		void _destruir_paquete(void* elem){
			free(elem);
		}
	while(1){

		int cod_op = recibir_operacion(socketCli);

		switch(cod_op){

		case SUSE_INIT:

			paqueteRecibido = recibir_paquete(socketCli);
			tid = *((int*)paqueteRecibido->head->data);
			nuevoThread = crear_thread(socketCli, tid);

			if(!tid){ //si el tid es 0 es el programa principal
				crear_entrada_en_cola_ready(socketCli);
				nodoEnEjecucion = crear_entrada_en_lista_execute(socketCli);
				nodoEnEjecucion->thread = nuevoThread;
				gettimeofday(&nuevoThread->tiempo_en_cola_actual,NULL);

			} else {
				sem_wait(&sem_new);
				list_add(colaNEW, nuevoThread);
				sem_post(&sem_new);
				}
			list_destroy_and_destroy_elements(paqueteRecibido, _destruir_paquete);
			break;

		case SUSE_SCHEDULE:

			sem_wait(&sem_ready);
			colaAAplicarSJF = ((t_cola_ready*)list_find(colaREADY, &_mismo_fd))->lista_threads; //buscamos la lista ready de este proceso
			sem_post(&sem_ready);
			sem_wait(&sem_execute);
			nodoEnEjecucion = (t_execute*)list_find(listaEXEC, &_mismo_fd); //buscamos el hilo en ejecucion de este proceso
			sem_post(&sem_execute);
			nuevoThread = nodoEnEjecucion->thread;

			if(nuevoThread != NULL){
				nuevoThread->tiempo_total_en_exec = nuevoThread->tiempo_total_en_exec + tiempo_que_paso_desde_colaActual(nuevoThread->tiempo_en_cola_actual);
				nuevoThread->ultima_rafaga =  tiempo_que_paso_desde_colaActual(nuevoThread->tiempo_en_cola_actual);
				gettimeofday(&nuevoThread->tiempo_en_cola_actual,NULL); //seteamos el tiempo en que entra a ready
				list_add(colaAAplicarSJF, nuevoThread);//desplazamos a la victima
				sem_post(&semaforoLoco); //Agregamos una instancia a la cola ready
			}

			sem_wait(&semaforoLoco);
			colaAAplicarSJF = ((t_cola_ready*)list_find(colaREADY, &_mismo_fd))->lista_threads;
			hiloAEjecutar = algoritmo_SJF(colaAAplicarSJF); //Que esta funcion haga el remove de la cola de ready y lo retorne
			swap_threads(nodoEnEjecucion, hiloAEjecutar); //mueve el hiloEnEjecucion a ready y el hiloAEjecutar a execute

			tid = hiloAEjecutar->tid;

//			if((nodoEnEjecucion->thread != NULL) && (list_size(colaAAplicarSJF) == 0)){
//				tid = nodoEnEjecucion->thread->tid;
//			} else {
//				sem_wait(&semaforoLoco);
//				colaAAplicarSJF = ((t_cola_ready*)list_find(colaREADY, &_mismo_fd))->lista_threads;
//				hiloAEjecutar = algoritmo_SJF(colaAAplicarSJF); //Que esta funcion haga el remove de la cola de ready y lo retorne
//				error = swap_threads(nodoEnEjecucion, hiloAEjecutar, colaAAplicarSJF); //mueve el hiloEnEjecucion a ready y el hiloAEjecutar a execute
//
//				if(error == 0)
//					sem_post(&semaforoLoco);
//
//				tid = hiloAEjecutar->tid;
//			}

			send(socketCli, &tid, sizeof(int), 0); //mandamos el tid del hilo que pusimos a ejecutar
			break;

		case SUSE_JOIN:

			paqueteRecibido = recibir_paquete(socketCli);
			tid = *((int*)paqueteRecibido->head->data);

			if(!tid_ya_esta_en_exit(tid, socketCli)){ //puede pasar que el thread al cual le hacen join ya termino
				buscar_y_pasarlo_a_blocked(socketCli, tid, JOIN);//Agarra el ult que esta en la cola exe de el socket y lo pasa a blocked por join con ese tid
			}

			list_destroy_and_destroy_elements(paqueteRecibido, _destruir_paquete);
			break;

		case SUSE_SIGNAL:
			paqueteRecibido = recibir_paquete(socketCli);
			tid = *((int*)list_get(paqueteRecibido, 0));
			nombreSemaforo = list_get(paqueteRecibido,1);

			posicionEnArray = posicion_en_vector(nombreSemaforo, SEM_IDS);

			sem_wait(&semaforos_suse);
			error = hacer_signal(posicionEnArray);
			sem_post(&semaforos_suse);

			list_destroy_and_destroy_elements(paqueteRecibido, _destruir_paquete);
			break;

		case SUSE_WAIT:
			paqueteRecibido = recibir_paquete(socketCli);
			tid = *((int*)list_get(paqueteRecibido, 0));
			nombreSemaforo = list_get(paqueteRecibido,1);
			posicionEnArray = posicion_en_vector(nombreSemaforo, SEM_IDS);

			sem_wait(&semaforos_suse);
			error = hacer_wait(posicionEnArray, socketCli);
			sem_post(&semaforos_suse);

			send(socketCli, &error, sizeof(int), 0);

			list_destroy_and_destroy_elements(paqueteRecibido, _destruir_paquete);
			break;

		case SUSE_CLOSE:
			paqueteRecibido = recibir_paquete(socketCli);
			tid = *((int*)paqueteRecibido->head->data);
			sem_wait(&sem_execute);
			buscar_y_pasarlo_a_exit(socketCli); //Agarra el ult que esta en la cola exe de el socket y lo pasa a exit
			sem_post(&sem_execute);
			buscar_hilos_blockeados_por_este(tid, socketCli);
			escribir_logs(tid, socketCli);

			if(!tid){
				list_destroy_and_destroy_elements(paqueteRecibido, &_destruir_paquete);
				close(socketCli);
				pthread_exit(NULL);
				return;
			} //Fin del hilo



			list_destroy_and_destroy_elements(paqueteRecibido, &_destruir_paquete);
			send(socketCli, &tid, sizeof(int), 0);
			break;
		}

	}
}




//FUNCIONES AUXILIARES DEL PLANIFICADOR DE LARGO PLAZO



void move_de_new_a_ready(){

	if(colaNEW->elements_count!=0){ //Si hay elementos que agregar en new
		sem_wait(&sem_new);
		t_thread* hilo = list_remove(colaNEW, 0); //sacamos el primero porque FIFO
		sem_post(&sem_new);

		int socket = hilo->socket_fd;

		bool _mismo_fd(void* elem){
			t_cola_ready* elemento = (t_cola_ready*)elem;
			return elemento->socket_fd == socket;
		}
		sem_wait(&sem_ready);
		t_cola_ready* colaReady = (t_cola_ready*)list_find(colaREADY, &_mismo_fd);
		sem_post(&sem_ready);

		list_add(colaReady->lista_threads, hilo);
		gettimeofday(&hilo->tiempo_en_cola_actual,NULL); //momento en que ingresa a ready

		sem_wait(&semaforos_suse);
		t_cosa* cosa = list_find(LISTA_SEMAFOROS, _mismo_fd);
		sem_post(cosa->semaforo);
		sem_post(&semaforos_suse);
	}
}

void* hay_blocked_ready(){
	bool _esta_blocked_ready(void* elem){
		t_blocked* hilo = (t_blocked*)elem;
		return hilo->estado == BLOCKED_READY;
	}

	void* respuesta = list_remove_by_condition(listaBLOCKED, &_esta_blocked_ready);

	return respuesta;
}

//ESTE HILO VA A MOVER LOS HILOS DESDE NEW O BLOCKED A READY
void planificador_largo_plazo(){
	while(FUNCIONAR){

		if(podemos_agregar_hilos_a_ready()){
			sem_wait(&sem_blocked);
			t_blocked* hilo = (t_blocked*)hay_blocked_ready();
			sem_post(&sem_blocked);
			if(hilo != NULL){

				int socket = hilo->thread->socket_fd;

				t_thread* thread = hilo->thread;

				bool _mismo_fd(void* elem){
					t_cola_ready* elemento = (t_cola_ready*)elem;
					return elemento->socket_fd == socket;
				}
				sem_wait(&sem_ready);
				t_cola_ready* colaReady = (t_cola_ready*)list_find(colaREADY, &_mismo_fd);
				sem_post(&sem_ready);

				list_add(colaReady->lista_threads, thread);

				gettimeofday(&thread->tiempo_en_cola_actual,NULL); //momento de ingreso a ready
				free(hilo);
				sem_wait(&semaforos_suse);
				t_cosa* cosa = list_find(LISTA_SEMAFOROS, _mismo_fd);
				sem_post(cosa->semaforo);
				sem_post(&semaforos_suse);

			} else {

				move_de_new_a_ready();
			}

		}
	}
	pthread_exit(NULL);
}



//FUNCIONES DE LIBERAR RECURSOS

void _liberar_exit(void* elem){
	free(elem);
}

void _liberar_ready(void* elem){
	t_cola_ready* cola = (t_cola_ready*)elem;
	list_destroy_and_destroy_elements(cola->lista_threads, &_liberar_exit);
	free(elem);
}
void _liberar_execute(void* elem){
	t_execute* cola = (t_execute*)elem;
	free(cola->thread);
	free(elem);
}

void _liberar_semaforos(void* elem){
	t_cosa* cosa = (t_cosa*)elem;
	sem_close((cosa->semaforo));
	//free(cosa->semaforo);
	free(elem);
}

void liberar_sem_ids(){
	for(int i=0 ; i<tamanio_vector(SEM_IDS); i++){
		free(SEM_IDS[i]);
	}
	free(SEM_IDS);
}

void liberar_recursos(int sig){
	list_destroy(colaNEW);
	list_destroy(listaBLOCKED);
	list_destroy_and_destroy_elements(listaEXIT, &_liberar_exit);
	list_destroy_and_destroy_elements(colaREADY, &_liberar_ready);
	list_destroy_and_destroy_elements(listaEXEC, &_liberar_execute);

	list_destroy_and_destroy_elements(LISTA_SEMAFOROS, &_liberar_semaforos);
	config_destroy(CONFIG);
	log_destroy(LOG);

	sem_close(&sem_blocked);
	sem_close(&sem_execute);
	sem_close(&sem_exit);
	sem_close(&sem_new);
	sem_close(&sem_ready);
	sem_close(&sem_run);
	sem_close(&semaforos_suse);
	close(SOCKET_ESCUCHA);
	//liberar_sem_ids();
	//free(SEM_MAX);
	//free(SEM_VALOR);
	FUNCIONAR = 0;
}

void iniciar_semaforos(){
	sem_init(&sem_new,0,1);
	sem_init(&sem_ready,0,1);
	sem_init(&sem_run,0,1);
	sem_init(&sem_blocked,0,1);
	sem_init(&sem_exit,0,1);
	sem_init(&semaforos_suse,0,1);
	sem_init(&sem_execute, 0, 1);
}


int main(){
	//INICIAMOS VARIABLES GLOBALES
	gettimeofday(&TIEMPO_INICIO_PROGRAMA, NULL);
	CONFIG = config_create("suse.config");

	LOG = log_create("suse.log", "suse.c", true, LOG_LEVEL_INFO);


	colaNEW = list_create();
	listaEXIT = list_create();
	listaBLOCKED = list_create();
	colaREADY = list_create();
	listaEXEC = list_create();
	LISTA_SEMAFOROS = list_create();

	SEM_IDS = ids_semaforos();
	SEM_VALOR = valores_iniciales_semaforos();
	SEM_MAX = valores_maximos_semaforos();

	iniciar_semaforos();

	signal(SIGINT, &liberar_recursos); //Cuando generemos esta signal se liberan los recursos y termina el proceso
	//ALARMA PARA LOGS
	signal(SIGALRM, &escribirLog);
	struct itimerval intervalo;
	struct timeval tiempoInicial;


	tiempoInicial.tv_sec = timerLog();
	tiempoInicial.tv_usec = 0;

	intervalo.it_value = tiempoInicial;
	intervalo.it_interval = tiempoInicial;
	setitimer(ITIMER_REAL, &intervalo, NULL);

	//CREAMOS HILOS DE PLANIF Y ATENCION
	pthread_t hiloAtencion, hiloPlanif;


	SOCKET_ESCUCHA = iniciar_socket(puerto_listen());
	int cliente;

	pthread_create(&hiloPlanif, NULL, &planificador_largo_plazo, NULL);


	while(FUNCIONAR){
		cliente = esperar_cliente(SOCKET_ESCUCHA);

		if(cliente!=-1){
			pthread_create(&hiloAtencion, NULL, &atenderCliente, &cliente);
			pthread_detach(hiloAtencion);
		}
	}

	//pthread_join(hiloPlanif, NULL);
	printf("\nADIOS\n");
	return EXIT_SUCCESS;
}
