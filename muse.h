/*
 * muse.h
 *
 *  Created on: 8 nov. 2019
 *      Author: utnso
 */

#ifndef MUSE_H_
#define MUSE_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/bitarray.h>
#include "segmentacion/segmentacion.h"
#include "paginacion/frames.h"
#include "paginacion/paginacion.h"
#include "sockets/sockets.h"

t_list* PROCESS_TABLE;


t_config* leer_config();
void liberacion_de_recursos(void*,t_config*);
int leer_del_config(char*, t_config*);
int muse_init(t_proceso*, char*, int);
uint32_t muse_alloc(t_proceso*, int);
void muse_free(t_proceso *, uint32_t);
void* muse_get(t_proceso*, t_list*);
int muse_cpy(t_proceso* , t_list*);
int main();

#endif /* MUSE_H_ */
