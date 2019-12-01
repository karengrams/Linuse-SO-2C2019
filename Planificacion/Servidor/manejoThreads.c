#include "suse.h"

extern sem_t sem_manejoThreads;

t_thread newThread(int tid) {
	t_thread* newThread = malloc(sizeof(t_thread));
	newThread->id_proceso = NULL; //habria que pasar aca el id del cliente
	newThread->tid = tid;
	newThread->tiempo_creacion = clock();
	newThread->tiempo_ejecucion = 0;
	newThread->tiempo_espera = 0;
	newThread->tiempo_uso_cpu = 0;
	newThread->porcentaje_tiempo_ejecucion = 0;


	return newThread;
}

t_thread findThread(int tid, t_list* list) {
	sem_wait(&sem_manejoThreads);

	for (int i = 0; i < list_size(list); i++) {
		t_thread* thread = list_get(list, i);
		if (thread->tid == tid) {

			sem_post(&sem_manejoThreads);
			return thread;
		}
	}

	t_thread* threadNotFound = malloc(sizeof(t_thread));
	threadNotFound->tid = -1;
	sem_post(&sem_manejoThreads);

	return threadNotFound;
}

int removeThreadFromList(int tid, t_list* list) {

	for (int i = 0; i < list_size(list); i++) {

		t_thread* thread = list_get(list, i);
		if (thread->tid == tid) {
			list_remove(list, i);
			return 1;
		}
	}
	return -1;
}

int moveThread(int tid, t_list* listFrom, t_list* listTo) {

	t_thread* thread = findThread(tid, listFrom);

	if (thread->tid == -1) { //Es un script fallido, se puede hacerle free
		free(thread);
		return -1;
	}

	sem_wait(&sem_manejoThreads);
	t_thread* aux_thread = thread;

	if ((removeThreadFromList(tid, listFrom)) == -1) {
		sem_post(&sem_manejoThreads);
		return -1;
	}

	list_add(listTo, aux_thread);
	sem_post(&sem_manejoThreads);

	return 1;
}

t_list getAllThreads() {
	t_list* allThreads = list_create();

	t_list* aux1 = colaNEW;
	t_list* aux2 = colaREADY;
	t_list* aux3 = listaEXEC;
	t_list* aux4 = listaEXIT;
	t_list* aux5 = listaBLOCKED;

	list_add_all(allThreads,aux1);
	list_add_all(allThreads,aux2);
	list_add_all(allThreads,aux3);
	list_add_all(allThreads,aux4);
	list_add_all(allThreads,aux5);

	free(aux1);
	free(aux2);
	free(aux3);
	free(aux4);
	free(aux5);

	list_sort(allThreads,orderByPID);

	return allThreads;
}

t_thread getShortestJobThread(t_thread thread,t_thread otherThread) {
	return (thread->tiempo_ejecucion <= otherThread->tiempo_ejecucion)? thread : otherThread;
}

t_thread orderByPID(t_thread thread,t_thread otherThread) {
	return (thread->id_proceso <= otherThread->id_proceso)? thread : otherThread;
}
