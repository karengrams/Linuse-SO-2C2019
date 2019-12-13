/*
 * structures.h
 *
 *  Created on: 4 dic. 2019
 *      Author: utnso
 */

#ifndef STRUCTURES_H_
#define STRUCTURES_H_

#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include <semaphore.h>
#include <commons/log.h>
#include <stdint.h>

typedef struct buffer t_buffer;
typedef struct paquete t_paquete;
typedef struct frame_t frame;

sem_t mutex_process_list;
sem_t mutex_shared;
sem_t mutex_write_shared_files;
sem_t mutex_write_frame;
sem_t mutex_process_list;
sem_t binary_free_frame;

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

typedef struct segment_t {
	int nro_segmento;
	segment_type tipo;
	uint32_t base_logica;
	int tamanio; //  tam. pedido
	t_list *metadatas; // Hay dos tipos segmentheapmetadata o mmapmetadata
	t_list *tabla_de_paginas;
}__attribute__((packed)) segment;

typedef struct heapmetadata_t{
	bool ocupado;
	int bytes;
}__attribute__((packed)) heapmetadata;

typedef struct segmentheapmetadata_t{
	heapmetadata *metadata;
	uint32_t posicion_inicial;
}__attribute__((packed)) segmentheapmetadata;

typedef struct segmentmmapmetadata_t{
	char *path;
	int tam_mappeado;
}__attribute__((packed)) segmentmmapmetadata;


typedef struct proceso{
	int id;
	char* ip;
	int totalMemoriaPedida;
	int totalMemoriaLiberada;
	t_list* tablaDeSegmentos;
}t_proceso;

typedef struct mapped_file_t{
	int nro_file;
	void *file;
	char *path; // Es la forma mas simple de comparar (creo)
	int tam_archivo;
	int flag;
	t_list *paginas_min_asignadas;
	t_list *procesos; // Con estos sacas los opens y vas viendo que proceso lo tiene abierto
}__attribute__((packed)) mapped_file;

struct frame_t{
	void *memoria;
	int nro_frame;
}__attribute__((packed));


typedef struct{
	int nro_pag;
	frame *ptr_frame;
}__attribute__((packed)) frames_compartidos;

struct buffer{
	int size;
	void* stream;
};

struct paquete{
	op_code codigo_operacion;
	t_buffer* buffer;
};

#endif /* STRUCTURES_H_ */
