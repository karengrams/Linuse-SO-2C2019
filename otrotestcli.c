/*
 * otrotestcli.c
 *
 *  Created on: 16 nov. 2019
 *      Author: utnso
 */


#include "libMuse.h"

int main(){
	int id = getpid();

	muse_init(id, "127.0.1.1", 50003);
	muse_close();
	return 0;
}
