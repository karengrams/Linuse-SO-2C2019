#ifndef UTILS_CLI_H_
#define UTILS_CLI_H_

	#include<stdio.h>
	#include<stdlib.h>
	#include<signal.h>
	#include<unistd.h>
	#include<sys/socket.h>
	#include<netdb.h>
	#include<string.h>

	typedef struct
	{
		int size;
		void* stream;
	} t_buffer;

	typedef struct
	{
		op_code codigo_operacion;
		t_buffer* buffer;
	} t_paquete;

	typedef enum {
		DESCONEXION = 0,
		SUSE_INIT = 1,
		SUSE_SCHEDULE = 2,
		SUSE_JOIN = 3,
		SUSE_CLOSE = 4,
		SUSE_WAIT = 5,
		SUSE_SIGNAL = 6,
	} op_code;


	int crear_conexion(char* ip, char* puerto);
	t_paquete* crear_paquete(op_code operacion);
	void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
	void enviar_paquete(t_paquete* paquete, int socket_cliente);
	void liberar_conexion(int socket_cliente);
	void eliminar_paquete(t_paquete* paquete);


#endif /* UTILS_CLI_H_ */
