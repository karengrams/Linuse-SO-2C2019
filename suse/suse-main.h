/*
 * suse-main.h
 *
 *  Created on: 18 dic. 2019
 *      Author: utnso
 */

#ifndef SUSE_MAIN_H_
#define SUSE_MAIN_H_

#include "suse-structures.h"
#include "suse-server.h"
#include "suse-planificador-largo-plazo.h"
#include <sockets.h>
#include <signal.h>
#include <semaphore.h>

void atenderCliente(void*);

#endif /* SUSE_MAIN_H_ */
