/*
 ============================================================================
 Name        : Suse3.c
 Author      : Andre Malpica
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "suse_cli.h"

int main(void) {
	t_log* logger = iniciar_logger();
	log_info(logger, "Soy un log");

	t_config* config = leer_config();
	char* valor = config_get_string_value(config, "CLAVE");
	log_info(logger, valor);

	leer_consola(logger);

	int conexion = crear_conexion(
		config_get_string_value(config, "IP"),
		config_get_string_value(config, "LISTEN_PORT")
	);

	enviar_mensaje(valor, conexion);

	t_paquete* paquete = armar_paquete();

	enviar_paquete(paquete, conexion);

	eliminar_paquete(paquete);
	log_destroy(logger);
	config_destroy(config);
	close(conexion);
}

t_log* iniciar_logger() {
	return log_create("suse.log", "SUSE", 1, LOG_LEVEL_INFO);
}

t_config* leer_config() {
	return config_create("suse.config");
}

void leer_consola(t_log* logger) {
	void loggear(char* leido) {
		log_info(logger, leido);
	}

	_leer_consola_haciendo((void*) loggear);
}

t_paquete* armar_paquete() {
	t_paquete* paquete = crear_paquete();

	void _agregar(char* leido) {
		agregar_a_paquete(paquete, leido, strlen(leido) + 1);
	}

	_leer_consola_haciendo((void*) _agregar);

	return paquete;
}

void _leer_consola_haciendo(void(*accion)(char*)) {
	char* leido = readline(">");

	while(strncmp(leido, "", 1) != 0) {
		accion(leido);
		free(leido);
		leido = readline(">");
	}

	free(leido);
}
