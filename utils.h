#ifndef UTILS_H_
#define UTILS_H_

#include <string.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>

typedef enum{
	HEAP,
	MUSE_MAP,
} segment_type;

typedef struct{
	int id;
	char* ip;
	t_list* tablaDeSegmentos;
} t_proceso;

typedef struct{
	segment_type tipo;
	void* baseLogica;
	int tamanio;
	t_list* tablaDePaginas;
} t_segmento;


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
 * a sus listas
 */
t_segmento* crear_segmento(int type, void* baseLogica, int tamanio);
t_proceso* crear_proceso(int id, char* ip);


#endif /* UTILS_H_ */
