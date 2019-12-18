#include "suse-server.h"

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
char* puerto_listen(){
	return config_get_string_value(CONFIG, "LISTEN_PORT");
}

int grado_de_multiprogramacion_maximo(){
	return config_get_int_value(CONFIG, "MAX_MULTIPROG");
}

double alpha_sjf(){
	return config_get_double_value(CONFIG, "ALPHA_SJF");
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

int swap_threads(t_execute* nodoExec, t_thread* nuevoHilo, t_list* cola){
	t_thread* victima = nodoExec->thread;

	if(victima!=NULL){ //si victima == NULL no habia procesos ejecutando en ese momento
		sem_wait(&sem_ready);
		victima->tiempo_total_en_exec = victima->tiempo_total_en_exec + tiempo_que_paso_desde_colaActual(victima->tiempo_en_cola_actual);
		victima->ultima_rafaga =  tiempo_que_paso_desde_colaActual(victima->tiempo_en_cola_actual);
		gettimeofday(&victima->tiempo_en_cola_actual,NULL); //seteamos el tiempo en que entra a ready
		list_add(cola, victima);//desplazamos a la victima
		sem_post(&sem_ready);
		sem_wait(&sem_execute);
		nodoExec->thread = nuevoHilo; //agregamos el nuevo hilo a exec
		nuevoHilo->tiempo_total_en_ready =	nuevoHilo->tiempo_total_en_ready + tiempo_que_paso_desde_colaActual(nuevoHilo->tiempo_en_cola_actual); //sumamos el tiempo que estuvo en ready
		gettimeofday(&nuevoHilo->tiempo_en_cola_actual,NULL); //seteamos el tiempo en que entra a exe
		sem_post(&sem_execute);
		return 0;
		}
		sem_wait(&sem_execute);
		nodoExec->thread = nuevoHilo; //agregamos el nuevo hilo a exec
		nuevoHilo->tiempo_total_en_ready =	nuevoHilo->tiempo_total_en_ready + tiempo_que_paso_desde_colaActual(nuevoHilo->tiempo_en_cola_actual); //sumamos el tiempo que estuvo en ready
		gettimeofday(&nuevoHilo->tiempo_en_cola_actual,NULL); //seteamos el tiempo en que entra a exe
		sem_post(&sem_execute);
		return 1;
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
	FUNCIONAR = 0;
}

//FUNCIONES DE INICIALIZAR RECURSOS

void iniciar_semaforos(){
	sem_init(&sem_new,0,1);
	sem_init(&sem_ready,0,1);
	sem_init(&sem_run,0,1);
	sem_init(&sem_blocked,0,1);
	sem_init(&sem_exit,0,1);
	sem_init(&semaforos_suse,0,1);
	sem_init(&sem_execute, 0, 1);
}

void inicializar_recursos_de_planificador(){
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
}
