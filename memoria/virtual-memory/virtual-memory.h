/*
 * virtual-memory.h
 *
 *  Created on: 27 nov. 2019
 *      Author: utnso
 */

#ifndef VIRTUAL_MEMORY_VIRTUAL_MEMORY_H_
#define VIRTUAL_MEMORY_VIRTUAL_MEMORY_H_
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <commons/bitarray.h>
#include <semaphore.h>
#include "../muse-structures.h"

int crear_archivo_swap(int);
void llenar_archivo(int,int);
int posicion_libre_en_swap();
void inicializar_bitmap_swap(int,int);

#endif /* VIRTUAL_MEMORY_VIRTUAL_MEMORY_H_ */
