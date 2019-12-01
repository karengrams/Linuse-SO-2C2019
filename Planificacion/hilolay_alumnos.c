#include "hilolay_alumnos.h"


void hilolay_init(){

	//iniciar conexion con suse

	init_internal(main_ops); //esta funcion crea el hilo del programa padre y llama a suse_create

}
