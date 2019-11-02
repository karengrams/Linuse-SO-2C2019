#ifndef UTILS_H_
#define UTILS_H_

#include <string.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include "../paginacion/paginacion.h"
#include "../paginacion/frames.h"
#include "../segmentacion/segmentacion.h"

t_list* PROCESS_TABLE;


typedef struct{
	int id;
	char* ip;
	t_list* tablaDeSegmentos;
} t_proceso;

//int minimo(int, int);
t_proceso* crear_proceso(int , char*);
/*
 * Busca al proceso que nos esta solicitando algo en la tabla de procesos de Muse
 * Si se encuentra retorna su direccion en memoria, sino retorna NULL
 */
t_proceso* buscar_proceso(t_list*,char* );
/*
 * Dado un proceso devuelve su posicion en la lista_de_procesos
 * Si el elemento esta repedito devuelve la primera posicion donde aparece (no deberia estar repetido tho)
 * Si el elemento no existe devuelve -1
 */
int posicion_en_lista_proceso(t_proceso* elemento);
uint32_t magia_muse_alloc(t_proceso* , int);
void liberar_proceso(t_proceso* proceso);
int magia_muse_cpy(t_proceso* proceso, t_list* paqueteRecibido);
int magia_muse_init(t_proceso* cliente_a_atender, char* ipCliente, int id);
void* magia_muse_get(t_proceso* , t_list*);
uint32_t magia_muse_alloc(t_proceso* proceso, int tam);
#endif /* UTILS_H_ */
