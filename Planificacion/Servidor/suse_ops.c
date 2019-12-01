#include "suse.h"

int suse_create(int tid) {

	t_thread* thread = newThread(tid);
	sem_wait(&sem_disponibleColaNEW);
	list_add(colaNEW,thread);
	sem_post(&sem_disponibleColaNEW);

	return 0;
}

int suse_schedule_next() {
	return 0;
}

int suse_join(int tid) {



	return 0;
}

int suse_wait(int, char*) {

	return 0;
}

int suse_signal(int, char*) {

	return 0;
}
