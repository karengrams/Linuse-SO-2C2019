#ifndef SUSE_H_
#define SUSE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>

typedef enum{
	JOIN = 1,
	SEMAPHORE = 2,
	BLOCKED_READY = 3,
} t_estado;



typedef struct{
	int fd;
	sem_t* semaforo;
} __attribute__((packed)) t_cosa;


typedef struct {
	int socket_fd; //socket del proceso que se comunica con suse
    int tid; //tid asignado por hilolay
    double tiempo_creacion; //valor de time(0) al momento de entrar a la cola new
    struct timeval tiempo_en_cola_actual;  //se le asigna time(0) cuando entra en ready o en exec
    double tiempo_total_en_ready;  //cuando sale de ready se le suma time(0) - tiempo_en_cola_actual
    double tiempo_total_en_exec; //cuando sale de exec se le suma time(0) - tiempo_en_cola_actual
    double ultima_rafaga;  //cuando sale de exec se le asigna time(0) - tiempo_en_cola_actual
    double ultima_estimacion; //la primera es 0
} __attribute__((packed)) t_thread;

typedef struct{
	int socket_fd;
	t_list* lista_threads;
}__attribute__((packed)) t_cola_ready;

typedef struct{
	int socket_fd;
	t_thread* thread;
} __attribute__((packed)) t_execute;

typedef struct{
	t_thread* thread;
	t_estado estado; //1 bloqueado por suse_join(tid), 2 por suse_wait(semaforo),
					 //3 ya puede pasar a ready cuando el grado de multiprogramacion lo permita
	int tid; //Valor del tid en caso de join, valor de la posicion del vector de semaforos en caso de wait
} __attribute__((packed)) t_blocked;

#endif
