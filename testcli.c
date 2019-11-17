#include "libMuse.h"

int main(){
	int *ptr;
	int id = getpid();
	ptr = &id;
	int error = muse_init(id, "127.0.0.1", 50003);
	printf("Hubo error al iniciar? %d\n", error);
	return 0;
}
