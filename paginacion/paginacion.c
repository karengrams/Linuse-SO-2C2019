#include "../paginacion/paginacion.h"

t_list* crear_lista_paginas(int cantidadDePaginas){
	t_list *tabla_de_paginas = list_create();
	page *pagina;

	for(int i = 0; i<cantidadDePaginas; i++){
		pagina = crear_pagina();
		list_add(tabla_de_paginas, pagina);
	}
	free(pagina);
	return tabla_de_paginas;
}

int paginas_necesarias(int valorPedido){
	div_t aux = div(valorPedido, tamanio_paginas());
	if (aux.rem == 0){
		return aux.quot;
	}else{
		return aux.quot + 1;
	}
}

page* crear_pagina(){
	page* pagina = malloc(sizeof(page));
	pagina->bit_presencia = 0;
	return pagina;
}

void dividir_tabla_de_pags(int cantidadDePaginas, t_list *tabla_de_pags){
	page* pagina;

	for(int i = 0; i<cantidadDePaginas; i++){
		pagina = crear_pagina();
		list_add(tabla_de_pags, pagina);
	}
}



