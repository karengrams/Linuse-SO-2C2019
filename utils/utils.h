#ifndef UTILS_H_
#define UTILS_H_

#include <string.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>

typedef struct{
	int id;
	char* ip;
	t_list* tablaDeSegmentos;
} t_proceso;s

typedef struct{
	bool ocupado;
	int bytes;
} t_metadata;

/*
 * Crea el bitmap de los marcos de la memoria
 * a partir de la lista de marcos
 */
t_bitarray* crear_bitmap(t_list* listaDeMarcos);

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

void liberar_proceso(t_proceso* proceso, t_list* tablaProcesos);

#endif /* UTILS_H_ */
