#include "libmuse.h"

void grabar_archivo(uint32_t arch, char* palabra)
{
	uint32_t offset;
	//hilolay_wait(revolucion_recibida);
	muse_get(&offset, arch, sizeof(uint32_t));
	muse_cpy(arch + offset, palabra, strlen(palabra) + 1);
	offset += strlen(palabra) + 1;
	muse_cpy(arch, &offset, sizeof(uint32_t));
	//hilolay_signal(revolucion_emitida);
	sleep(1);
}

uint32_t leer_archivo(uint32_t arch, uint32_t leido)
{
	uint32_t offset;
	char * palabra = malloc(100);
	//hilolay_wait(presion_emitida);
	muse_get(&offset, arch, sizeof(uint32_t));
	uint32_t len = offset - leido;
	muse_get(palabra, arch + leido, len);
	offset += strlen(palabra) + 1;
	muse_cpy(arch, &offset, sizeof(uint32_t));
	//hilolay_signal(presion_recibida);
	puts(palabra);
	free(palabra);
	return offset;
}

int main(){
int *ptr;
void* cosas = malloc(50);
void* recibir = malloc(265);
void* recibir2 = malloc(265);
int id = getpid();
muse_init(id, "10.0.2.15", 9172);

uint32_t arch = muse_map("archivito-loco-de-4096", 4096, MAP_SHARED);
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