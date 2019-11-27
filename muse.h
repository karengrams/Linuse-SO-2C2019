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
#include "segmentacion/segmentacion.h"
#include "paginacion/frames.h"
#include "paginacion/paginacion.h"
#include "sockets/sockets.h"

t_list* PROCESS_TABLE;
t_list* MAPPED_SHARED_FILES;

typedef struct mapped_shared_file_t{
	void *file_mmap;
	char *path; // Es la forma mas simple de comparar (creo)
	t_list *paginas_asignadas;
	t_list *procesos; // Con estos sacas los opens y vas viendo que proceso lo tiene abierto
}__attribute__((packed)) mapped_shared_file;

typedef struct{
	int nro_pag;
	frame *ptr_frame;
}__attribute__((packed)) frames_compartidos;

void inicializar_tabla_procesos();
void inicializar_tabla_archivos_compartidos();

t_config* leer_config();
void liberacion_de_recursos(void*,t_config*);
int leer_del_config(char*, t_config*);
int muse_init(t_proceso*, char*, int);
uint32_t muse_alloc(t_proceso*, int);
void muse_free(t_proceso *, uint32_t);
void* muse_get(t_proceso*, t_list*);
int muse_cpy(t_proceso* , t_list*);
void harcodeo_un_poquito_las_cosas(segment*);
void hardcodeo_para_muse_sync(segment*);
mmapmetadata* crear_mmapmetadata(char *, size_t , int ,segment* );
mapped_shared_file* buscar_archivo_abierto(char*);
uint32_t muse_map(t_proceso*,char*, size_t, int);
int muse_sync(t_proceso* ,uint32_t , size_t );
int muse_unmap(uint32_t);
int main();

#endif /* MUSE_H_ */
