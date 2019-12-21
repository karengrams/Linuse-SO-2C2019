#include "libmuse.h"

int main(){
	int *ptr;
	void* cosas = malloc(50);
	void* recibir = malloc(265);
	void* recibir2 = malloc(265);
	int id = getpid();
	muse_init(id, "10.0.2.15", 9172);
	uint32_t my_memory = muse_alloc(100);
	uint32_t map = muse_map("stephen.txt",50,MAP_SHARED);
	printf("my_memory=%lu\n",my_memory);
	printf("map=%lu\n",map);
	muse_cpy(map,"Me da miedo el amor eso fue lo que te dije yo no .", 50);
	muse_get(recibir2,map,50);
	printf("muse_get(recibir2,map,50)=%s\n",(char*)recibir2);
	muse_sync(map,50);
	muse_unmap(map);
	muse_close();
	return 0;
}