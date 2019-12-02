
#ifndef hilolay_alumnos_h__
#define hilolay_alumnos_h__

#include "utils-cli.c"
#include <commons/config.h>

	typedef struct hilolay_operations {
		int (*suse_create) (int);
		int (*suse_schedule_next) (void);
		int (*suse_join) (int);
		int (*suse_close) (int);
		int (*suse_wait) (int, char *);
		int (*suse_signal) (int, char *);
	} hilolay_operations;

	struct hilolay_operations *main_ops;

	void init_internal(struct hilolay_operations*);

#endif // hilolay_alumnos_h__
