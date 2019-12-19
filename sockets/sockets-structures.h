/*
 * sockets-structures.h
 *
 *  Created on: 18 dic. 2019
 *      Author: utnso
 */

#ifndef SOCKETS_STRUCTURES_H_
#define SOCKETS_STRUCTURES_H_

typedef struct proceso t_proceso;
typedef struct buffer t_buffer;
typedef struct paquete t_paquete;

struct buffer{
	int size;
	void* stream;
};

struct paquete{
	int codigo_operacion;
	t_buffer* buffer;
};

struct proceso{
	int id;
	char* ip;
	int totalMemoriaPedida;
	int totalMemoriaLiberada;
	t_list* tablaDeSegmentos;
};

#endif /* SOCKETS_STRUCTURES_H_ */
