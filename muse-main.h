/*
 * muse-main.h
 *
 *  Created on: 4 dic. 2019
 *      Author: utnso
 */

#ifndef MUSE_MAIN_H_
#define MUSE_MAIN_H_
#include "muse.h"
#include "sockets/sockets.h"
#include "virtual-memory/virtual-memory.h"
#include "structures.h"
#include <signal.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <pthread.h>

void atender_cliente_select(fd_set* master, int socketCli);
void atender_cliente(void *);

#endif /* MUSE_MAIN_H_ */
