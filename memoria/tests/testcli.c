#include "libMuse.h"
#include <unistd.h>

int main(){
	void* recibir = malloc(265);
	void* recibir2 = malloc(265);
	int id = getpid();
	muse_init(id, "127.0.0.1",9172);
	uint32_t my_memory = muse_alloc(30);
	uint32_t map = muse_map("stephen.txt",277,MAP_SHARED);
	printf("map = %lu\n",map);
	printf("my_memory = %lu\n",my_memory);
	muse_cpy(my_memory, "Hola loquui, como va?", 21);
	muse_get(recibir,my_memory,10);
	muse_cpy(map, "Habia una vaca re loca y pirada que queria aprobar sistemas operativos y no pudo ", 81); 
	muse_get(recibir2,map,81);
	muse_sync(map+10,10);
	printf("muse_get(recibir,my_memory,10)=%s\n", (char*)recibir);
	printf("muse_get(recibir2,map,81)=%s\n",(char*)recibir2);
	printf("POR DIOSOSSSSNKASNDKLASDFANF");
//	muse_free(my_memory);
	muse_close();
	return 0;
}
