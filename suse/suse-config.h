/*
 * suse-config.h
 *
 *  Created on: 18 dic. 2019
 *      Author: utnso
 */

#ifndef SUSE_CONFIG_H_
#define SUSE_CONFIG_H_

#include <commons/config.h>
#include "suse-structures.h"

int tamanio_vector(char** array);
int* pasar_a_vector_de_int(char** array);
int* valores_iniciales_semaforos();
int* valores_maximos_semaforos();
char** ids_semaforos();
int timerLog();
char* puerto_listen();
int grado_de_multiprogramacion_maximo();
double alpha_sjf();

#endif /* SUSE_CONFIG_H_ */
