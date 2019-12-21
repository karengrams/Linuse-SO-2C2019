#include "libmuse.h"
int main(){
int *ptr;
void* cosas = malloc(50);
void* recibir = malloc(265);
void* recibir2 = malloc(265);
int id = getpid();
muse_init(id, "10.0.2.15", 9172);
uint32_t my_memory = muse_alloc(100);
//muse_cpy(50, "POR DIOS DONDE SORONGO ROMPEEEEEEE, SINCRO TE ODIO", 50); // Todo: para mi es aca donde muere
uint32_t map = muse_map("stephen.txt",50,MAP_SHARED);
/*
	muse_cpy(map, "POR DIOS DONDE SORONGO ROMPEEEEEEE, SINCRO TE ODIO", 50); // Todo: para mi es aca donde muere
	muse_get(recibir2,map,50);
	muse_sync(map,10);
	muse_unmap(map);
	printf("my_memory=%lu\n",my_memory);
	printf("map=%lu\n",map);
	printf("muse_get(recibir2,map,50)=%s\n",(char*)recibir2);
	muse_free(my_memory);
	printf("POR DIOSOSSSSNKASNDKLASDFANF");*/
muse_close();
return 0;
}