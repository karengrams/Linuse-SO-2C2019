#ifndef SUSE_H_
#define SUSE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <pthread.h>

typedef enum {
	DESCONEXION = -1,
	SUSE_INIT = 1,
	SUSE_SCHEDULE = 2,
	SUSE_JOIN = 3,
	SUSE_CLOSE = 4,
	SUSE_WAIT = 5,
	SUSE_SIGNAL = 6,
} op_code;

//	t_thread newThread(int);
//	t_thread findThread(int, t_list*);
//	int removeThreadFromList(int, t_list*);
//	int moveThread(int, t_list*, t_list*);
//

typedef struct {
	int socket_fd;
    int tid;
    int tiempo_creacion; //valor de time() al momento de entrar a la cola new
    int tiempo_en_cola_actual; //valor de time() al momento de entrar en ready o en exec (
    						   //cuando se saca de esa cola se vuelve a llamar a time() y
    						   //la diferencia se suma en el campo que corresponda de los siguientes dos
    int tiempo_total_en_ready;
    int tiempo_total_en_exec;
    int tiempo_ejecucion; //intervalo en milisegundos desde la creacion hasta el momento de tomar la metrica
    int ultima_rafaga; //ultimo tiempo en estado EXEC
    int ultima_estimacion; //la primera es 0
} t_thread;

typedef struct{
	int socket_fd;
	t_list* lista_threads;
} t_cola_ready;

typedef struct{
	int socket_fd;
	t_thread* thread;
} t_execute;

#endif
