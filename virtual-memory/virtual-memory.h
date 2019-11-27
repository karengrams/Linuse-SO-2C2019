/*
 * virtual-memory.h
 *
 *  Created on: 27 nov. 2019
 *      Author: utnso
 */

#ifndef VIRTUAL_MEMORY_VIRTUAL_MEMORY_H_
#define VIRTUAL_MEMORY_VIRTUAL_MEMORY_H_
#include "../paginacion/paginacion.h"
#include "../paginacion/frames.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>


t_bitarray *BIT_ARRAY_SWAP;
void* VIRTUAL_MEMORY;

void* crear_archivo_swap(int);
void llenar_archivo(int,int);
int posicion_libre_en_swap();
void inicializar_bitmap_swap(int,int);
void asignar_marco_en_swap(page*);
void swap_pages(page*,page*);


#endif /* VIRTUAL_MEMORY_VIRTUAL_MEMORY_H_ */
