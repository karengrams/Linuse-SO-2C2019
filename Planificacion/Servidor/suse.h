#ifndef SUSE_H_
	#define SUSE_H_

	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <commons/log.h>
	#include <hilolay/alumnos.h>
	#include <pthread.h>
	#include <semaphore.h>
	#include <time.h>
	#include "utils.h"

	#define DIRCONFIG "/home/utnso/workspace/Suse3/Servidor/suse.config"
	#define NEW "NEW"
	#define READY "READY"
	#define BLOCKED "BLOCKED"
	#define EXIT "EXIT"

	int suse_create(int);
	int suse_schedule_next(void);
	int suse_join(int);
	int suse_close(int);
	int suse_wait(int, char*);
	int suse_signal(int, char*);

	t_thread newThread(int);
	t_thread findThread(int, t_list*);
	int removeThreadFromList(int, t_list*);
	int moveThread(int, t_list*, t_list*);


	static struct hilolay_operations hiloops = {
		.suse_create = &suse_create,
		.suse_schedule_next = &suse_schedule_next,
		.suse_join = &suse_join,
		.suse_close = &suse_close,
		.suse_wait = &suse_wait,
		.suse_signal = &suse_signal
	};

	typedef struct {
		int id_proceso;
	    int tid;
	    int tiempo_creacion;
	    int tiempo_ejecucion; //intervalo en milisegundos desde la creacion hasta el momento de tomar la metrica
	    int tiempo_espera; //suma de los intervalos en milisegundos en estado READY
	    int tiempo_uso_cpu; //suma de los intervalos en milisegundos en estado EXEC
	    float porcentaje_tiempo_ejecucion; //tiempo de ejecucion del hilo dividido la suma del tiempo de ejecucion de todos los hilos del proceso
	} t_thread;

#endif
