/*
 * suse-planificador-largo-plazo.h
 *
 *  Created on: 18 dic. 2019
 *      Author: utnso
 */

#ifndef SUSE_PLANIFICADOR_LARGO_PLAZO_H_
#define SUSE_PLANIFICADOR_LARGO_PLAZO_H_

#include <semaphore.h>
#include <commons/collections/list.h>
#include "suse-structures.h"
#include "suse-logs.h"
#include "suse-config.h"

void move_de_new_a_ready();
void* hay_blocked_ready();
void *planificador_largo_plazo();
bool podemos_agregar_hilos_a_ready();

#endif /* SUSE_PLANIFICADOR_LARGO_PLAZO_H_ */
