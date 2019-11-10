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
#include "utils/utilsSockets.h"
#include "utils/utils.h"

t_config* leer_config();
void liberacion_de_recursos(void*,t_config*);
int leer_del_config(char*, t_config*);
//int magia_muse_cpy(t_proceso* proceso, t_list* paqueteRecibido);
//int magia_muse_init(t_proceso* cliente_a_atender, char* ipCliente, int id);
//void* magia_muse_get(t_proceso* , t_list*);
uint32_t muse_alloc(t_proceso* proceso, int tam);
void muse_free(t_proceso *, uint32_t );
int main();

#endif /* MUSE_H_ */
