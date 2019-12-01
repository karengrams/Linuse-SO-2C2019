#include "libMuse.h"
#include <unistd.h>

int main(){
	int *ptr;
	void* cosas = malloc(50);
	void* recibir = malloc(265);
	int id = getpid();
	muse_init(id, "127.0.0.1",9157);
//	uint32_t my_memory = muse_alloc(35);
//	uint32_t my_other_memory = muse_alloc(5);
//	uint32_t my_other_other_memory = muse_alloc(10);
//
//	printf("my_memory = %lu\n",my_memory);
//	printf("my_other_memory = %lu\n",my_other_memory);
//	printf("my_other_other_memory = %lu",my_other_other_memory);s

	uint32_t map = muse_map("holi.txt",90,MAP_SHARED);
	muse_sync(map,90);

//	sleep(240);
	//printf("map = %lu\n",map);
	//muse_get(recibir,115,1);
	//muse_sync(map,265);
	//printf("MUSE_GET me devolvio %s\n", (char*)recibir);
	muse_close();
	return 0;
}
