#include "suse.h"

extern sem_t sem_disponibleColaREADY;
extern sem_t sem_multiprocesamiento;
extern t_list* colaREADY;
extern t_list* listaEXEC;
extern t_list* listaEXIT;
extern t_log* logger;
extern int max_multi;
extern int grado_multi;


void planificadorNEWaREADY() {

	while (1) {

		sem_wait(&sem_disponibleColaREADY);
		sem_wait(&sem_multiprocesamiento);

		if(grado_multi < max_multi) {
			FIFO();
		}
	}

}

void FIFO() {
	t_thread* thread = list_get(colaNEW,0);
	moveThread(thread->tid,colaNEW,colaREADY);
	return;
}

void SJF() {
	list_sort(colaREADY,getShortestJobThread);
	t_thread* thread = list_get(colaREADY,0);
	moveThread(thread->tid,colaREADY,listaEXEC);
	return;
}
