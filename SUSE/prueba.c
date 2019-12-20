#include <stdio.h>
#include <stdlib.h>
#include <hilolay/hilolay.h>
#include <unistd.h>
#include <string.h>

#define RUTA_ARCHIVO "una/ruta"
#define MAP_SHARED 1

struct hilolay_sem_t *presion_emitida;
struct hilolay_sem_t *presion_recibida;
struct hilolay_sem_t *revolucion_emitida;
struct hilolay_sem_t *revolucion_recibida;
/*
void grabar_archivo(uint32_t arch, char* palabra)
{
	uint32_t offset;
	hilolay_wait(revolucion_recibida);
	muse_get(&offset, arch, sizeof(uint32_t));
	muse_cpy(arch + offset, palabra, strlen(palabra) + 1);
	offset += strlen(palabra) + 1;
	muse_cpy(arch, &offset, sizeof(uint32_t));
	hilolay_signal(revolucion_emitida);
	sleep(1);
}

uint32_t leer_archivo(uint32_t arch, uint32_t leido)
{
	uint32_t offset;
	char * palabra = malloc(100);
	hilolay_wait(presion_emitida);
	muse_get(&offset, arch, sizeof(uint32_t));
	uint32_t len = offset - leido;
	muse_get(palabra, arch + leido, len);
	offset += strlen(palabra) + 1;
	muse_cpy(arch, &offset, sizeof(uint32_t));
	hilolay_signal(presion_recibida);
	puts(palabra);
	free(palabra);
	return offset;
}*/

void *revolucionar()
{
	puts("blablabla");
	/*uint32_t arch = muse_map(RUTA_ARCHIVO, 4096, MAP_SHARED);
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

	muse_unmap(arch);*/

	return 0;
}


int main(void)
{
	struct hilolay_t revolucion;

	hilolay_init();
	//muse_init(2, "127.0.0.1", 3306);

	presion_emitida = hilolay_sem_open("presion_emitida");
	presion_recibida = hilolay_sem_open("presion_recibida");
	revolucion_emitida = hilolay_sem_open("revolucion_emitida");
	revolucion_recibida = hilolay_sem_open("revolucion_recibida");


	hilolay_create(&revolucion, NULL, &revolucionar, NULL);


	hilolay_join(&revolucion);

	hilolay_sem_close(presion_emitida);
	hilolay_sem_close(presion_recibida);
	hilolay_sem_close(revolucion_emitida);
	hilolay_sem_close(revolucion_recibida);


	return hilolay_return(0);
}