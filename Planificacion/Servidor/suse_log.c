void metrics(int loggear) {

	t_list* allThreads;

	if (loggear) {

		log_info(logger, "----METRICS----");

		sem_wait(&sem_manejoThreads);

		for (int i = 0; i < list_size(allThreads); i++) {
			t_thread* aThread = list_get(allThreads, i);


		}

		sem_post(&sem_manejoThreads);

	} else {
		printf("\n\n----METRICS----");

		printf("\n\n");

	}
}
