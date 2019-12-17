#include <stdio.h>
#include <stdlib.h>
#include "libmuse.h"
#include <unistd.h>
#include <string.h>

#define RUTA_ARCHIVO "un_archivo.txt"
#define MAP_SHARED 1
/*
void grabar_archivo(uint32_t arch, char* palabra)
{
	uint32_t offset;
	muse_get(&offset, arch, sizeof(uint32_t));
	muse_cpy(arch + offset, palabra, strlen(palabra) + 1);
	offset += strlen(palabra) + 1;
	muse_cpy(arch, &offset, sizeof(uint32_t));
	sleep(1);
}

void *revolucionar()
{
	uint32_t arch = muse_map(RUTA_ARCHIVO, 4096, MAP_SHARED);
	uint32_t offset = 0;

	offset = sizeof(uint32_t);

	for(int i = 0; i<6;i++)
		offset = leer_archivo(arch, offset);

	offset = sizeof(uint32_t);
	muse_cpy(arch, &offset, sizeof(uint32_t));

	grabar_archivo(arch, "Es hora de levantarse y aprobar el TP\n");

	grabar_archivo(arch, "Es hora de aprobar y festejar\n");

	grabar_archivo(arch, "Saben que la entrega se esta por acabar\n");

	grabar_archivo(arch, "Asi que hagamoslo!\n");

	arch += 5000;

	muse_get(NULL, arch, 1);

	muse_sync(arch, 4096);

	muse_unmap(arch);

	return 0;
}
*/

uint32_t leer_archivo(uint32_t arch, uint32_t leido)
{
	uint32_t offset;
	char * palabra = malloc(100);
	muse_get(&offset, arch, sizeof(uint32_t));
	uint32_t len = offset - leido;
	muse_get(palabra, arch + offset, len);
	offset += strlen(palabra) + 1;
	muse_cpy(arch, &offset, sizeof(uint32_t));
	puts(palabra);
	free(palabra);
	return offset;
}

void revolucionar(){

	uint32_t arch = muse_map(RUTA_ARCHIVO, 4096, MAP_SHARED);
	uint32_t offset = 0;

	offset = sizeof(uint32_t);

	for(int i = 0; i<6;i++)
		offset = leer_archivo(arch, offset);

}


int main(void)
{
	muse_init(2, "127.0.0.1", 9172);
	revolucionar();
	muse_close();
	return 0;
}
