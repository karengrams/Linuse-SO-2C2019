#ifndef SEGMENTACION_H
#define SEGMENTACION_H
#include <commons/collections/list.h>
#include <commons/collections/node.h>
#include "../../sockets/sockets-structures.h"
#include "../paginacion/frames.h"
#include "../paginacion/paginacion.h"
#include "../muse-structures.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>

sem_t mutex_shared_files;
t_list* MAPPED_FILES;

int minimo(int, int);
segment* crear_segmento(segment_type, int, t_list*);
uint32_t calculo_base_logica(segment *, t_list*);
segment* buscar_segmento_heap_para_tam(t_list*, int);
segment* buscar_segmento_heap_expandible_para_tam(t_list*, int);
bool segmento_de_tipo_heap_y_expandible(int, t_list*, void*);
bool segmento_de_tipo_heap_y_con_espacio(int, void*);
bool tiene_espacio_suficiente(int ,void*);
uint32_t limite_segmento(segment*);
bool segmento_puede_agrandarse(segment*, int, t_list*);
int numero_pagina(segment*, uint32_t);
int espacio_libre(segment *);
int desplazamiento_en_pagina(segment*, uint32_t);
segment* buscar_segmento_dada_una_direccion(uint32_t, t_list*);
uint32_t obtener_offset_para_tam(segment *, int );
segmentheapmetadata *buscar_metadata_para_liberar(uint32_t , segment *);
segmentheapmetadata* buscar_metadata_de_segmento_segun(uint32_t, segment*);
void expandir_segmento(segment *,int);
bool metadatas_fusionables(segmentheapmetadata *, segmentheapmetadata *);
segmentheapmetadata* buscar_metadata_para_anidar(t_list *, segmentheapmetadata *);
void buddy_system(segmentheapmetadata *, t_list *);
bool direccion_pisa_alguna_metadata(segment *,uint32_t, int);
segment* crear_segmento_heap(int, t_list* );
segment* crear_segmento_map(int,t_list*);
void recalcular_bases_logicas_de_segmentos(t_list *);
void liberar_recursos_segmento_heap(segment*,t_proceso* );
void eliminar_metadatas(void *,segment*);
void liberar_recursos_segmento_map(segment *,t_proceso*);
void liberar_recursos_del_segmento(segment*,t_proceso*);
void liberar_tabla_de_segmentos(t_proceso*);
void eliminar_segmento_de_tabla(t_proceso*,segment*);
mapped_file* buscar_archivo_abierto(char*);
void update_file_number();
int index_del_segment_metadata(segmentheapmetadata *,t_list *);
void escribir_metadata_en_frame(segment*,segmentheapmetadata*);
void* serializar_heap_metadata(heapmetadata* , int );

#endif /* muse-segmentacion_h */
