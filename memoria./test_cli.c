#include "libmuse.h"
#include <unistd.h>

int main(){
	int *ptr;
	void* cosas = malloc(50);
	void* recibir = malloc(265);
	int id = getpid();
	muse_init(id, "127.0.0.1",9172);
	uint32_t my_memory = muse_alloc(35);
	uint32_t my_other_memory = muse_alloc(5);
	uint32_t my_other_other_memory = muse_alloc(50);
	uint32_t my_other_other_other_memory = muse_alloc(10);
	muse_free(my_other_other_memory);
	muse_free(my_other_other_other_memory);
	muse_free(my_other_memory);
	muse_free(my_memory);
	uint32_t my_other_other_other_other_memory = muse_alloc(100);
//	printf("my_memory = %lu\n",my_memory);
//	printf("my_other_memory = %lu\n",my_other_memory);
	uint32_t map = muse_map("stephen.txt",279,MAP_PRIVATE);
	printf("map = %lu\n",map);
	char * fragmento = (char*) malloc(120);
	muse_get(fragmento,map,120);
	printf("fragmento=%s\n",fragmento);
	muse_cpy(map,"holi",4);
	muse_get(fragmento,map,120);
	printf("fragmento=%s\n",fragmento);
//	muse_sync(map,120);
//	uint32_t my_other_other_memory = muse_alloc(10);
//	printf("my_other_other_memory = %lu",my_other_other_memory);
//	muse_unmap(map);
//	muse_cpy(my_memory, "Hola perri, como va?", 21);
//	muse_get(recibir,my_memory,10);
//	printf("MUSE_GET me devolvio %s\n", (char*)recibir);
	muse_close();
	return 0;
}