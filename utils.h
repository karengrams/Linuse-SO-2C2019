#ifndef UTILS_H_
#define UTILS_H_

#include <string.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>

typedef enum{
	HEAP,
	MMAP,
} segment_type;

typedef struct{
	bool ocupado;
	int bytes;
} t_metadata;

typedef struct{
	int id;
	char* ip;
	t_list* tablaDeSegmentos;
} t_proceso;

typedef struct{
	segment_type tipo;
	uint32_t baseLogica; //void* o uint32_t ???
	int tamanio; //list_size(tablaDePaginas) * tamanioPagina
	t_list* tablaDePaginas;
} t_segmento;

typedef struct{
	bool bit_presencia;
	int numero_frame;
}t_pagina;

/*
 * Divide la memoria en marcos
 * Devuelve una lista con punteros al inicio de cada marco
 */
t_list* dividir_memoria(void* memoria, int tamanioPagina, int tamanioMemoria);

/*
 * Crea el bitmap de los marcos de la memoria
 * a partir de la lista de marcos
 */
t_bitarray* crear_bitmap(t_list* listaDeMarcos);

/*
 * Reserva memoria e inicializa los structs para luego agregarlos
 * a sus listas/tablas
 */
t_segmento* crear_segmento(int type, void* baseLogica, int tamanio);
t_proceso* crear_proceso(int id, char* ip);

/*
 * Busca al proceso que nos esta solicitando algo en la tabla de procesos de Muse
 * Si se encuentra retorna su direccion en memoria, sino retorna NULL
 */
t_proceso* buscar_proceso(t_list* paqueteRecibido, t_list* tablaDeProcesos, char* ipCliente);

/*
 * Dado un proceso devuelve su posicion en la lista_de_procesos
 * Si el elemento esta repedito devuelve la primera posicion donde aparece (no deberia estar repetido tho)
 * Si el elemento no existe devuelve -1
 */
int posicion_en_lista_proceso(t_proceso* proceso, t_list* lista);


/*
 * Dado un segmento devuelve si tiene cierta cantidad de espacio pedida disponible
 */
bool segmento_tiene_espacio(t_segmento* segmento, int tamanio, t_list* listaDeMarcos);

/*
 * Hace una copia de un segmento de forma continua
 */
void mappear_segmento(t_segmento* segmento, void* segmentoMappeado, t_list* listaDeMarcos);
bool tiene_espacio(void* punteroAMemoria, int valorPedido);




void liberar_proceso(t_proceso* proceso, t_list* tablaProcesos);
void liberar_segmentos(t_list* segmentos);

#endif /* UTILS_H_ */
