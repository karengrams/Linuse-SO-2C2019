/*
 * suse-logs.h
 *
 *  Created on: 18 dic. 2019
 *      Author: utnso
 */

#ifndef SUSE_LOGS_H_
#define SUSE_LOGS_H_

#include "suse-structures.h"
#include <sys/time.h>

double diferencia_entre_timevals(struct timeval, struct timeval);
double tiempo_que_paso_desde_inicio();
double tiempo_que_paso_desde_colaActual(struct timeval);
void* _suma_hilos(void*, void*);
int total_hilos_en_ready_y_exec();
bool podemos_agregar_hilos_a_ready();
void loggear_semaforos(void);
void loggear_procesos(void);
void escribir_logs(int, int);
void escribirLog(int);

#endif /* SUSE_LOGS_H_ */
