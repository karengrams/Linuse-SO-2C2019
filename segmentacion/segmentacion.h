#ifndef SEGMENTACION_H
#define SEGMENTACION_H

#include <commons/list.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>


typedef enum{
	HEAP,
	MMAP,
} segment_type;

typedef struct{
	segment_type tipo;
	uint32_t baseLogica; //void* o uint32_t ???
	int tamanio; //list_size(tablaDePaginas) * tamanioPagina
	t_list* tablaDePaginas;
} t_segmento;


/**
	* @NAME: crear_segmento
	* @DESC: Reserva memoria e inicializa los structs para luego agregarlos a sus listas/tablas
	* @PARAMS:
	* 			int type — tipo de segmento. Puede ser HEAP O MMAP
	* 			void* baseLogica — donde comienza el segmento referido a la tabla de segmentos del
	* 							   proceso
	* 			int tamanio — tamaño del segmento a crear
**/
t_segmento* crear_segmento(segment_type type, uint32_t baseLogica, int tamanio, t_list* listaPaginas);

/**
	* @NAME: posicion_en_lista_segmento
	* @DESC: segun el segmento, devuelve la posicion que se encuentra en la tabla de segmentos, es decir, su base logica.
	* @PARAMS:
	* 			t_segmento* elemento — elemento a buscar en la tabla de segmentos.
	* 			t_list* lista — puntero a tabla de segmentos del proceso
**/
int posicion_en_lista_segmento(t_segmento* elemento, t_list* lista);

/**
	* @NAME: segmento_puede_agrandarse
	* @DESC: determina si un segmento dado puede agrandarse o no.
	* @PARAMS:
	* 			t_segmento* segmento — segmento a determinar si puede agrandarse o no
	* 			int tamanio — tam. a agrandar
	* 			t_list* listaDeMarcos
**/
bool segmento_puede_agrandarse(t_segmento* segmento, int tamanio, t_list* listaDeMarcos);

/**
	* @NAME: segmento_puede_agrandarse
	* @DESC: Hace una copia de un segmento de forma continua
	* @PARAMS:
	* 			t_segmento* segmento — segmento a realizar copia
	* 			void* segmentoMappeado — segmento donde vamos a guardar la copia
	* 			t_list* listaDeMarcos
 */
void mappear_segmento(t_segmento* segmento, void* segmentoMappeado, t_list* listaDeMarcos);

/**
	* @NAME: posicion_en_lista_segmento
	* @DESC: dado un segmento devuelve si tiene cierta cantidad de espacio pedida disponible
	* @PARAMS:
	* 		   t_segmento* segmento — segmento a analizar
	* 		   int tamanio — tam. a disponer
	* 		   t_list* listaDeMarcos
 */
bool segmento_tiene_espacio(t_segmento* segmento, int tamanio, t_list* listaDeMarcos);

/**
	* @NAME: liberar_segmentos
	* @DESC: librerar tabla de segmentos.
	* @PARAMS:
	* 		  t_list* segmentos — puntero a tabla de segmentos
 */
void liberar_segmentos(t_list* segmentos);


#endif /* muse-segmentacion_h */
