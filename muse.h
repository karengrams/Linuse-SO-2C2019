/*
 * muse.h
 *
 *  Created on: 8 nov. 2019
 *      Author: utnso
 */

#ifndef MUSE_H_
#define MUSE_H_
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <fcntl.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <commons/bitarray.h>
#include <commons/string.h>
#include "segmentacion/segmentacion.h"
#include "paginacion/frames.h"
#include "paginacion/paginacion.h"
#include "virtual-memory/virtual-memory.h"
#include "structures.h"

t_list* PROCESS_TABLE;
t_list* MAPPED_FILES;

t_config* config;

void inicializar_tabla_procesos();
void inicializar_tabla_archivos_compartidos();
void inicializar_memoria_virtual(int);
t_config* leer_config();
void liberacion_de_recursos(int);
int leer_del_config(char*, t_config*);
int museinit(t_proceso*, char*, int);
void museclose(t_proceso*);
uint32_t museAlloc(t_proceso*, int);
void musefree(t_proceso *, uint32_t);
void* museget(t_proceso*, t_list*);
int musecpy(t_proceso* , t_list*);
mapped_file* buscar_archivo_abierto(char*);
uint32_t musemap(t_proceso*,char*, size_t, int);
int musesync(t_proceso* ,uint32_t , size_t );
int museunmap(t_proceso*,uint32_t);
segment* ultimo_segmento_heap(t_proceso*);
int memoria_libre_en_segmento(segment*);
int cantidad_total_de_segmentos_en_sistema();
int suma_frames_libres();
int memory_leaks_proceso(t_proceso*);

#endif /* MUSE_H_ */
