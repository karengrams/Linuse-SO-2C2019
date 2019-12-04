#include <time.h>
#include <signal.h>
#include <sys/time.h>
#include "suse.h"
#include "utils.c"

t_list* colaNEW;
t_list* colaREADY;
t_list* listaEXEC;
t_list* listaEXIT;
t_list* listaBLOCKED;
t_config* CONFIG;

int* SEM_VALOR;
int* SEM_MAX;
char** SEM_IDS;

//FUNCIONES DEL CONFIG
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
		printf("%d\n", atoi(array[i]));
		vector[i] = (atoi(array[i]));
	}

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
char* puerto_listen(){
	return config_get_string_value(CONFIG, "LISTEN_PORT");
}

int grado_de_multiprogramacion_maximo(){
	return config_get_int_value(CONFIG, "MAX_MULTIPROG");
}

int alpha_sjf(){
	return config_get_int_value(CONFIG, "ALPHA_SJF");
}



//FUNCIONES DE LOS LOGS
void escribir_logs(int motivo){
	if (motivo == 0){
		//loggear "Finalizo hilo"
	} else {
		//loggear "Timer"
	}

	//Logs posta
}

void escribirLog(int signal){
	escribir_logs(1);
}



//FUNCIONES AUXILIARES DE SUSE_CREATE

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
	nuevoThread->tiempo_creacion = time(0);
	nuevoThread->tiempo_en_cola_actual = 0; //se le asigna time(0) cuando entra en ready o en exec
	nuevoThread->tiempo_total_en_exec = 0; //cuando sale de exec se le suma time(0) - tiempo_en_cola_actual
	nuevoThread->tiempo_total_en_ready = 0; //cuando sale de ready se le suma time(0) - tiempo_en_cola_actual
	nuevoThread->ultima_estimacion = 0;
	nuevoThread->ultima_rafaga=0; //cuando sale de exec se le asigna time(0) - tiempo_en_cola_actual
	list_add(colaNEW, nuevoThread);
}


//FUNCIONES AUXILIARES DE SUSE_SCHEDULE

void swap_threads(t_execute* nodoExec, t_thread* nuevoHilo, t_list* cola){
	t_thread* victima = nodoExec->thread;

	if(victima!=NULL){ //si victima == NULL no habia procesos ejecutando en ese momento
					   //en suse_close si estaba ejecutando cuando termina se pone NULL

	victima->tiempo_total_en_exec =+ time(0)-victima->tiempo_en_cola_actual;
	victima->ultima_rafaga = time(0) - victima->tiempo_en_cola_actual;
	victima->tiempo_en_cola_actual = time(0); //seteamos el tiempo en que entra a ready
	list_add(cola, victima);//desplazamos a la victima
	}

	nodoExec->thread = nuevoHilo; //agregamos el nuevo hilo a exec
	nuevoHilo->tiempo_total_en_ready =+ time(0)-nuevoHilo->tiempo_en_cola_actual; //sumamos el tiempo que estuvo en ready
	nuevoHilo->tiempo_en_cola_actual = time(0); //seteamos el tiempo en que entra a exe
}

int rafaga_estimada(void* elem){
	t_thread* hilo = (t_thread*) elem;
	return (alpha_sjf()*hilo->ultima_rafaga + (1-alpha_sjf())*hilo->ultima_estimacion);
}

bool _menor_rafaga_estimada(void* elem, void* elem2){
	return rafaga_estimada(elem) < rafaga_estimada(elem2);
}

t_thread* algoritmo_SJF(t_list* lista){
	list_sort(lista, &_menor_rafaga_estimada);
	t_thread* hiloElegido = (t_thread*)list_remove(lista, 0);
	hiloElegido->ultima_estimacion = rafaga_estimada(hiloElegido); //le asigno la nueva ultima estiamcion de sjf
	return hiloElegido;
}


//FUNCIONES AUXILIARES DEL SUSE_CLOSE
void buscar_hilos_blockeados_por_este(tid){ //probablemente no haya o haya solo uno que sea el proceso padre....

	bool _blocked_por_tid(void* elem){
		t_blocked* elemento = (t_blocked*)elem;
		return (elemento->estado == JOIN) && (elemento->tid == tid);
	}

	t_list* listaBloqueados = list_filter(listaBLOCKED, &_blocked_por_tid);

	void _cambiar_a_blocked_ready(void* elem){
		t_blocked* elemento = (t_blocked*) elem;
		elemento->estado = BLOCKED_READY;
	}
	list_iterate(listaBloqueados, &_cambiar_a_blocked_ready);
}

void buscar_y_pasarlo_a_exit(int fd){ //Este codigo quedo asquerosamente feo

	t_thread* hilo ;

	bool _mismo_fd(void* elem){
		t_cola_ready* elemento = (t_cola_ready*)elem;
		return elemento->socket_fd == fd;
	}

	t_execute* execution = list_find(listaEXEC, &_mismo_fd);//lo buscamos en exec

	hilo = execution->thread;
	hilo->tiempo_total_en_exec =+ time(0) - hilo->tiempo_en_cola_actual;
	execution->thread = NULL;
	list_add(listaEXIT, hilo);

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

	return list_any_satisfy(listaEXIT, &_mismo_tidfd);
}

void buscar_y_pasarlo_a_blocked(int fd,int razon, t_estado estado){
	t_thread* hilo ;

		bool _mismo_fd(void* elem){
			t_cola_ready* elemento = (t_cola_ready*)elem;
			return elemento->socket_fd == fd;
		}

		t_execute* execution = list_find(listaEXEC, &_mismo_fd);//lo buscamos en exec

		hilo = execution->thread;
		hilo->tiempo_total_en_exec =+ time(0) - hilo->tiempo_en_cola_actual;
		execution->thread = NULL;
		t_blocked* blocked = crear_entrada_blocked(hilo, estado, razon);
		list_add(listaBLOCKED, blocked);

}

//FUNCIONES AUXILIARES DE WAIT Y SIGNAL

int posicion_en_vector(char* nombre, char** array){
	int longitud = tamanio_vector(array);

	for(int i=0; i<longitud; i++){
		if(!strcmp(nombre, array[i]))
			return i;
	}
	return -1;
}

int hacer_signal(int posicion){
	if(SEM_VALOR[posicion]<SEM_MAX[posicion]){
		SEM_VALOR[posicion]++;

		if(SEM_VALOR[posicion]<=0){	//Hay threads bloqueados por este semaforo
				bool _blocked_por_sem(void* elem){
					t_blocked* blocked = (t_blocked*)elem;
					return ((blocked->estado == SEMAPHORE) && (blocked->tid == posicion));
				}
			t_blocked* hilo = list_find(listaBLOCKED, &_blocked_por_sem); //buscamos el primer blockeado
			hilo->estado = BLOCKED_READY; // y lo ponemos como blocked ready
		}
		return 1;
	} else { //Caso raro, no hacer nada?
		return 0;
	}
}

int hacer_wait(int posicion, int fd){
	if(SEM_VALOR[posicion]>0){ //Si es mayor a 0 lo toma
		SEM_VALOR[posicion]--;
		return 1;
	} else { //Si es menor o igual a 0, lo debe decrementar y bloquearse (buscar la cola exe del fd y desplazar ese thread)
		buscar_y_pasarlo_a_blocked(fd, posicion, SEMAPHORE);
		return 0;
	}
}

//HILO DE ATENCION A CLIENTES, UN HILO POR CADA SOCKET CONECTADO

void atenderCliente(void* elemento){
	int socketCli = *((int*)elemento);
	t_list* paqueteRecibido = NULL, *colaAAplicarSJF;
	t_cola_ready* nodoReady;
	t_execute* nodoEnEjecucion;
	t_thread* hiloAEjecutar;
	int tid, posicionEnArray, error;
	char* nombreSemaforo;

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
			nodoReady = (t_cola_ready*)list_find(colaREADY, &_mismo_fd); //buscamos la lista ready de este proceso
			colaAAplicarSJF = nodoReady->lista_threads;
			nodoEnEjecucion = (t_execute*)list_find(listaEXEC, &_mismo_fd); //buscamos el hilo en ejecucion de este proceso

			hiloAEjecutar = algoritmo_SJF(colaAAplicarSJF); //Que esta funcion haga el remove de la cola de ready y lo retorne
			swap_threads(nodoEnEjecucion, hiloAEjecutar, colaAAplicarSJF); //mueve el hiloEnEjecucion a ready y el hiloAEjecutar a execute
			tid = hiloAEjecutar->tid;

			send(socketCli, &tid, sizeof(int), 0); //mandamos el tid del hilo que pusimos a ejecutar

			break;

		case SUSE_CLOSE:
			paqueteRecibido = recibir_paquete(socketCli);
			tid = *((int*)paqueteRecibido->head->data);
			buscar_y_pasarlo_a_exit(socketCli); //Agarra el ult que esta en la cola exe de el socket y lo pasa a exit
			buscar_hilos_blockeados_por_este(tid);
			escribir_logs(0);
			break;

		case SUSE_JOIN:
			paqueteRecibido = recibir_paquete(socketCli);
			tid = *((int*)paqueteRecibido->head->data);

			if(!tid_ya_esta_en_exit(tid, socketCli)){ //puede pasar que el thread al cual le hacen join ya termino
				buscar_y_pasarlo_a_blocked(socketCli, tid, JOIN);//Agarra el ult que esta en la cola exe de el socket y lo pasa a blocked por join con ese tid
			}
			break;

		case SUSE_SIGNAL:
			paqueteRecibido = recibir_paquete(socketCli);
			tid = *((int*)list_get(paqueteRecibido, 0));
			nombreSemaforo = list_get(paqueteRecibido,1);

			posicionEnArray = posicion_en_vector(nombreSemaforo, SEM_IDS);
			error = hacer_signal(posicionEnArray);
			break;

		case SUSE_WAIT:
			paqueteRecibido = recibir_paquete(socketCli);
			tid = *((int*)list_get(paqueteRecibido, 0));
			nombreSemaforo = list_get(paqueteRecibido,1);

			posicionEnArray = posicion_en_vector(nombreSemaforo, SEM_IDS);
			error = hacer_wait(posicionEnArray, socketCli);
			break;
		}
	}
}




//FUNCIONES AUXILIARES DEL PLANIFICADOR DE LARGO PLAZO
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

void move_de_new_a_ready(){

	if(colaNEW->elements_count){ //Si hay elementos que agregar en new
		t_thread* hilo = list_remove(colaNEW, 0); //sacamos el primero porque FIFO
		int socket = hilo->socket_fd;

		bool _mismo_fd(void* elem){
			t_cola_ready* elemento = (t_cola_ready*)elem;
			return elemento->socket_fd == socket;
		}
		t_cola_ready* colaReady = (t_cola_ready*)list_find(colaREADY, &_mismo_fd);
		list_add(colaReady->lista_threads, hilo);
		hilo->tiempo_en_cola_actual = time(0); //momento en que ingresa a ready
	}
}

void* hay_blocked_ready(){
	bool _esta_blocked_ready(void* elem){
		t_blocked* hilo = (t_blocked*)elem;
		return hilo->estado == BLOCKED_READY;
	}
	return list_remove_by_condition(listaBLOCKED, &_esta_blocked_ready);
}

//ESTE HILO VA A MOVER LOS HILOS DESDE NEW O BLOCKED A READY
void planificador_largo_plazo(){
	while(1){
		if(podemos_agregar_hilos_a_ready()){

			t_blocked* hilo = (t_blocked*)hay_blocked_ready();

			if(hilo != NULL){
				int socket = hilo->thread->socket_fd;

				bool _mismo_fd(void* elem){
					t_cola_ready* elemento = (t_cola_ready*)elem;
					return elemento->socket_fd == socket;
				}
				t_cola_ready* colaReady = (t_cola_ready*)list_find(colaREADY, &_mismo_fd);
				list_add(colaReady->lista_threads, hilo->thread);
				hilo->thread->tiempo_en_cola_actual = time(0); //momento de ingreso a ready
			} else {
				move_de_new_a_ready();
			}

		}
	}
}





int main(){
	//INICIAMOS VARIABLES GLOBALES
	CONFIG = config_create("suse.config");
	colaNEW = list_create();
	listaEXIT = list_create();
	listaBLOCKED = list_create();
	colaREADY = list_create();
	listaEXEC = list_create();

	SEM_IDS = ids_semaforos();
	SEM_VALOR = valores_iniciales_semaforos();
	SEM_MAX = valores_maximos_semaforos();

	//ALARMA PARA LOGS
	signal(SIGALRM, &escribirLog);
	struct itimerval intervalo;
	struct timeval tiempoInicial;
	struct timeval inter;

	tiempoInicial.tv_sec = timerLog();
	tiempoInicial.tv_usec = 0;

	inter.tv_sec = timerLog();
	inter.tv_usec = 0;

	intervalo.it_value = tiempoInicial;
	intervalo.it_interval = inter;


	//CREAMOS HILOS DE PLANIF Y ATENCION
	pthread_t hiloAtencion, hiloPlanif;


	int socketEscucha = iniciar_servidor("127.0.0.1", puerto_listen());
	int cliente;

	pthread_create(&hiloPlanif, NULL, &planificador_largo_plazo, NULL);


	while(1){
		cliente = esperar_cliente(socketEscucha);
		pthread_create(&hiloAtencion, NULL, &atenderCliente, &cliente);
	}
}
