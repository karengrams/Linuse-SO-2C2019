/*
 * suse-planificador-plazo.c
 *
 *  Created on: 18 dic. 2019
 *      Author: utnso
 */

#include "suse-planificador-largo-plazo.h"

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

bool podemos_agregar_hilos_a_ready(){
	return total_hilos_en_ready_y_exec() < grado_de_multiprogramacion_maximo();
}


//ESTE HILO VA A MOVER LOS HILOS DESDE NEW O BLOCKED A READY
void* planificador_largo_plazo(){
	while(FUNCIONAR){



		sem_wait(&hilos_para_agregar);

		sem_wait(&multiprogramacion);

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
	pthread_exit(NULL);
}
