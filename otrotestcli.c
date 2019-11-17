#include "libMuse.h"

int main(){
	int id = getpid();
	muse_init(id, "127.0.0.1", 50101);
	muse_close();
	return 0;
}
