#include "libMuse.c"

int main(){
	int *ptr;
	int id = getpid();
	ptr = &id;
	muse_init(id, "127.0.0.1", 44444);

	muse_alloc(30);
	muse_free(ptr);
	if(getc(stdin) == 'c')
		muse_close();
	return 0;

}
