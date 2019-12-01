#include "suse.h"


int max_multi;

t_list* colaNEW;
t_list* colaREADY;
t_list* listaEXEC;
t_list* listaEXIT;
t_list* listaBLOCKED;

t_log* logger;

sem_t sem_manejoThreads;
sem_t sem_disponibleColaNEW;
sem_t sem_disponibleColaREADY;
sem_t sem_multiprocesamiento;
sem_t sem_refreshConfig;


