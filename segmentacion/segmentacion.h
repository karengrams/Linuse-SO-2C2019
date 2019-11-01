#ifndef SEGMENTACION_H
#define SEGMENTACION_H

#include <commons/collections/list.h>
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
	uint32_t baseLogica;
	int tamanio; //  tam. ocupado
	t_list* tablaDePaginas;
} segment;


/**
	* @NAME: crear_segmento
	* @DESC: Reserva memoria e inicializa los structs para luego agregarlos a sus listas/tablas
	* @PARAMS:
	* 			int type — tipo de segmento. Puede ser HEAP O MMAP
	* 			void* baseLogica — donde comienza el segmento referido a la tabla de segmentos del
	* 							   proceso
	* 			int tamanio — tamaño del segmento a crear
**/
segment* crear_segmento(segment_type type, uint32_t baseLogica, int tamanio, t_list* listaPaginas);

/**
	* @NAME: posicion_en_lista_segmento
	* @DESC: segun el segmento, devuelve la posicion que se encuentra en la tabla de segmentos, es decir, su base logica.
	* @PARAMS:
	* 			t_segmento* elemento — elemento a buscar en la tabla de segmentos.
	* 			t_list* lista — puntero a tabla de segmentos del proceso
**/
int posicion_en_tabla_segmento(segment* elemento, t_list* tabla_de_segmentos);

/**
	* @NAME: segmento_puede_agrandarse
	* @DESC: determina si un segmento dado puede agrandarse o no.
	* @PARAMS:
	* 			t_segmento* segmento — segmento a determinar si puede agrandarse o no
	* 			int tamanio — tam. a agrandar
	* 			t_list* listaDeMarcos
**/
bool segmento_puede_agrandarse(segment* segmento, int tamanio);

/**
	* @NAME: segmento_puede_agrandarse
	* @DESC: Hace una copiade todas las paginas de un segmento de forma continua, finaliza
	* 		 con una metadata con los campos "ocupado" = false y "bytes" = -1 como flag.
	* @PARAMS:
	* 			t_segmento* segmento — segmento a realizar copia
	* 			void* segmentoMappeado — segmento donde vamos a guardar la copia
	* 			t_list* listaDeMarcos
 */
void mappear_segmento(segment* segmento, void* segmentoMappeado);

/**
	* @NAME: posicion_en_lista_segmento
	* @DESC: dado un segmento devuelve si tiene cierta cantidad de espacio pedida disponible
	* 		 y guarda la direccion en el puntero que se pasa por parametro. (direccion de la
	* 		 metadata que referencia al espacio libre necesitado).
	* @PARAMS:
	* 		   t_segmento* segmento — segmento a analizar
	* 		   int tamanio — tam. a disponer
	* 		   uint32_t* direccionEncontrada puntero donde se guardara la direccion del espacio
	* 		   libre en caso de encontrarse (direccion dentro de el segmento que se esta tratando)
 */
bool segmento_tiene_espacio(segment* segmento, int tamanio, uint32_t* direccionEncontrada);


/**
	* @NAME: reservar_memoria
	* @DESC: Dados una cantidad de bytes, un segmento y una direccion/desplazamiento
	* 		 dentro del segmento crea y modifica las metadatas necesarias para reservar la memoria
	* @PARAMS:
	* 		  int bytesPedidos, uint32_t desplazamiento, segment* segmento
 */

void reservar_memoria(int bytesPedidos, uint32_t desplazamiento, segment* segmento);

/**
	* @NAME: liberar_segmentos
	* @DESC: librerar tabla de segmentos.
	* @PARAMS:
	* 		  t_list* segmentos — puntero a tabla de segmentos
 */
void liberar_segmentos(t_list* segmentos);

/**
	* @NAME: tamanio_segmento
	* @DESC: tamanio total del segmento, independientemente si esta ocupado o libre
	* @PARAMS:
	* 		  segment* segmento
 */

int tamanio_segmento(segment* segmento);

/**
	* @NAME: limite_segmento
	* @DESC: direccion logica del final del segmento, independientemente si esta ocupado o libre
	* @PARAMS:
	* 		  segment* segmento
 */
uint32_t limite_segmento(segment* segmento);

/**
	* @NAME: numero_pagina
	* @DESC: Retorna el numero de la pagina (div_t.quot) que corresponde a una direccion dada
	* Y el desplazamiento dentro de la misma pagina  (div_t.rem)
	* @PARAMS:
	* 		  segment* segmento, uint32_t direccion
 */

div_t numero_pagina(segment* segmento, uint32_t direccion);

segment* buscar_segmento_dada_una_direccion(t_list* tablaSegmentos, uint32_t direccion);

#endif /* muse-segmentacion_h */
