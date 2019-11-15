#include "libMuse.c"

int main(){
	int *ptr;
	void* cosas = malloc(50);
	int size= 17;
	void* recibir = malloc(size);
	int id = getpid();
	ptr = &id;

	int error = muse_init(id, "127.0.0.1", 50003);
	printf("Hubo error al iniciar? %d\n", error);

	uint32_t direccion = muse_alloc(60);
	printf("la direccion asignada es %x\n", direccion);

	error = muse_cpy(direccion, "Putitaasfjdk", 13);
	printf("Hubo error en el muse_cpy? %d\n", error);

//	muse_get(recibir, direccion, size);
//	printf("MUSE_GET me devolvio %s\n", (char*)recibir);

//	muse_free(direccion);

//	direccion = muse_map("este es un path re loco", 79014, 0);
//	printf("Muse_map me mappeo en la direccion %x \n", direccion);
//
//	error = muse_sync(direccion, 15);
//	printf("Hubo error en el muse_sync? %d\n", error);
//
//	error = muse_unmap(direccion);
//	printf("Hubo error en el muse_unmap? %d\n", error);
//
	if(getc(stdin) == 'c')
		muse_close();

	free(cosas);
	free(recibir);
	return 0;

}
