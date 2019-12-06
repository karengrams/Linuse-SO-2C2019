# SUSE
### Para compilar:

```C
gcc suse.c utils.c -lcommons -lpthread
```

### En la consola de las pruebas para que tome el puerto como variable de entorno:

```C
export PUERTO="valor"
```

### Para compilar la prueba::
```C
gcc prueba.c utils-cli.c hilolay_alumnos.c -lhilolay
```

### TODO list:
-  [ ] Hacer test mas exigentes
-  [ ] Liberar recursos 
-  [ ] Hacer makefile?
