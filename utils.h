#ifndef UTILS_H_
#define UTILS_H_

#include <commons/collections/list.h>
#include <commons/bitarray.h>

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




#endif /* UTILS_H_ */
