/*
 * structures.h
 *
 *  Created on: 4 dic. 2019
 *      Author: utnso
 */

#ifndef MUSE_STRUCTURES_H_
#define MUSE_STRUCTURES_H_

#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <semaphore.h>
#include <commons/log.h>
#include <stdint.h>

typedef struct t_frame frame;
typedef struct t_page page;
typedef struct t_heapmetadata heapmetadata;
typedef struct t_segmentheapmetadata segmentheapmetadata;
typedef struct t_pages_in_frames pages_in_frames;
typedef struct t_mapped_file mapped_file;
typedef struct t_segmentmmapmetadata segmentmmapmetadata;
typedef struct t_segment segment;


sem_t mutex_log;
sem_t mutex_process_list;
sem_t mutex_shared;
sem_t mutex_write_shared_files;
sem_t mutex_write_frame;
sem_t mutex_process_list;
sem_t binary_free_frame;
sem_t mutex_free;

t_log *logger_info;
t_log *logger_trace;
t_log *logger_error;

typedef enum {
	DESCONEXION = 0,
	MUSE_INIT = 10,
	MUSE_ALLOC = 11,
	MUSE_FREE = 12,
	MUSE_GET = 13,
	MUSE_CPY = 14,
	MUSE_MAP = 15,
	MUSE_SYNC = 16,
	MUSE_UNMAP = 17,
	MUSE_CLOSE = 18,
} op_code;

typedef enum segment_type_t {
	HEAP, MMAP,
} segment_type;

struct t_page{
	bool bit_presencia;
	bool bit_modificado;
	bool bit_uso;
	int nro_pagina;
	int nro_frame;
	frame *frame;
} __attribute__((packed));

struct t_segment {
	int nro_segmento;
	segment_type tipo;
	uint32_t base_logica;
	int tamanio; //  tam. pedido
	t_list *metadatas; // Hay dos tipos segmentheapmetadata o mmapmetadata
	t_list *tabla_de_paginas;
}__attribute__((packed));

struct t_heapmetadata{
	bool ocupado;
	int bytes;
}__attribute__((packed));

struct t_segmentheapmetadata{
	heapmetadata *metadata;
	uint32_t posicion_inicial;
}__attribute__((packed));

struct t_segmentmmapmetadata{
	char *path;
	int tam_mappeado;
}__attribute__((packed));

struct t_mapped_file{
	int nro_file;
	void *file;
	char *path; // Es la forma mas simple de comparar (creo)
	int tam_archivo;
	int flag;
	t_list *paginas_min_asignadas;
	t_list *procesos; // Con estos sacas los opens y vas viendo que proceso lo tiene abierto
}__attribute__((packed));

struct t_frame{
	void *memoria;
	int nro_frame;
}__attribute__((packed));

struct t_pages_in_frames{
	page* ptr_page;
	frame *ptr_frame;
}__attribute__((packed));

#endif /* MUSE_STRUCTURES_H_ */
