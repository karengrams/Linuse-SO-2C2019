#include "libMuse.h"
#include <unistd.h>

int main(){
	int *ptr;
	void* cosas = malloc(50);
	void* recibir = malloc(265);
	void* recibir2 = malloc(265);

	int id = getpid();
	muse_init(id, "127.0.0.1",9168);
	uint32_t my_memory = muse_alloc(30);
//	uint32_t my_other_memory = muse_alloc(5);
//	uint32_t my_other_other_memory = muse_alloc(50);
//	uint32_t my_other_other_other_memory = muse_alloc(10);
//	uint32_t my_other_other_other_other_memory = muse_alloc(100);
//	muse_free(my_other_other_other_other_memory);
//	muse_free(my_other_other_other_memory);
//	muse_free(my_other_other_memory);
//	muse_free(my_other_memory);
//	muse_free(my_memory);

	uint32_t map = muse_map("holi.txt",265,MAP_SHARED);
	printf("map = %lu\n",map);
//	muse_sync(map,120);
	printf("my_memory = %lu\n",my_memory);
//	printf("my_other_memory = %lu\n",my_other_memory);
//	printf("my_other_other_memory = %lu\n",my_other_other_memory);
//	printf("my_other_other_other_memory = %lu\n",my_other_other_other_memory);
	sleep(3);
//	muse_unmap(map);
	muse_cpy(my_memory, "Hola perri, como va?", 21);
	muse_get(recibir,my_memory,10);
	muse_get(recibir2,map,115);
	printf("muse_get(recibir,my_memory,10)=%s\n", (char*)recibir);
	printf("muse_get(recibir2,map,115)=%s\n",(char*)recibir2);
	muse_close();
	return 0;
}
