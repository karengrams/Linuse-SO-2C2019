#include "suse.h"

typedef enum {
	MENSAJE,
	PAQUETE
}op_code;

int max_multi;

t_list* colaNEW;
t_list* colaREADY;
t_list* listaEXEC;
t_list* listaEXIT;
t_list* listaBLOCKED;

t_log* logger;

sem_t sem_manejoThreads;
sem_t sem_disponibleColaNEW;
sem_t sem_disponibleColaREADY;
sem_t sem_multiprocesamiento;
sem_t sem_refreshConfig;



int main(void)
{

	colaNEW = list_create();
	colaREADY = list_create();
	listaEXEC = list_create();
	listaEXIT = list_create();
	listaBLOCKED = list_create();

	t_config* config = config_create(DIRCONFIG);
	int max_multi = config_get_int_value(config,"MAX_MULTIPROG");

	void iterator(char* value)
	{
		printf("%s\n", value);
	}

	logger = log_create("logger.log", "Servidor", 1, LOG_LEVEL_DEBUG);

	int server_fd = iniciar_servidor(
		config_get_string_value(config, "IP"),
		config_get_string_value(config, "LISTEN_PORT")
	);

	log_info(logger, "Servidor listo para recibir al cliente");
	int cliente_fd = esperar_cliente(server_fd);

	t_list* lista;
	while(1)
	{
		int cod_op = recibir_operacion(cliente_fd);
		switch(cod_op)
		{
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			break;
		case PAQUETE:
			lista = recibir_paquete(cliente_fd);
			printf("Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_SUCCESS;
}
