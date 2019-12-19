/*
 * muse-main.h
 *
 *  Created on: 4 dic. 2019
 *      Author: utnso
 */

#ifndef MUSE_MAIN_H_
#define MUSE_MAIN_H_
#include "muse-server.h"
#include <sockets.h>
#include "virtual-memory/virtual-memory.h"
#include <signal.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <pthread.h>
#include "muse-structures.h"
#include "../sockets/sockets-structures.h"

void _destruir_paquete(void*);
void atender_cliente_select(fd_set* master, int socketCli);
void* atender_cliente(void *);

#endif /* MUSE_MAIN_H_ */
