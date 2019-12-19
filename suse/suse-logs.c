/*
 * suse-logs.c
 *
 *  Created on: 18 dic. 2019
 *      Author: utnso
 */

#include "suse-logs.h"

//FUNCIONES DE LOS LOGS
bool _not_null(void* elem){
	t_execute* elemento = (t_execute*) elem;
	return (elemento->thread != NULL);
}

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

