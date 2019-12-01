#ifndef SUSE_CLI_H_
    #define SUSE_CLI_H_

    #include<stdio.h>
    #include<stdlib.h>
    #include<commons/log.h>
    #include<commons/string.h>
    #include<commons/config.h>
    #include<readline/readline.h>

    #include "utils_cli.h"

    t_log* iniciar_logger(void);
    t_config* leer_config(void);
    void leer_consola(t_log* logger);
    t_paquete* armar_paquete();

    void _leer_consola_haciendo(void(*accion)(char*));
#endif /* SUSE_CLI_H_ */
