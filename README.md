# libMUSE y MUSE
### ¿Como importar un archivo de una carpeta?
Se coloca un nuevo `#include`
```C
#include "/carpeta/archivo.h"
```

### ¿Como compilar si no tenemos el makefile?
```C
gcc muse.c paginacion/paginacion.c paginacion/frames.c segmentacion/segmentacion.c utils/utils.c -o muse -lcommons
```

### Ejemplo de tabla de segmentos
Proceso #1:

```C
  uint32_t x = muse_alloc(5);
  uint32_t y = muse_alloc(5);
  uint32_t z = muse_alloc(1);
  uint32_t i = muse_alloc(10);
  uint32_t p = muse_alloc(40);
  size_t file_tam = 10;
  uint32_t map = muse_map("un_archivo_loco.txt",file_tam,0);
  uint32_t q = muse_alloc(10);
  uint32_t w = muse_alloc(40);
  uint32_t r = muse_alloc(10);
```
![Imagen de tabla de segmentos](https://i.ibb.co/SQqQH2v/adklsjdal-Page-3-2.png)

### TODO list
- [ ] Copiar metadatas en los frames correspondientes (falta hacerlo en muse_alloc y muse_free)
- [ ] Hacer el swap in y swap out de archivo swap y archivo map
- [ ] Hacer al algoritmo clock modificado para swap
- [ ] Hacer tests
- [ ] Asignarle a un archivo las paginas divididas
- [ ] Page fault
- [ ] Hacer procedimiento que separe el archivo en frames/pages
- [ ] Manejo de errores si hace un muse_free de una direccion invalida 


*Colocando una x entre los corchetes se pone como marcado*

