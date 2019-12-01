#include "suse.h"

int getCreateTime(int time) {
	int current_time = clock();

	return current_time - clock();
}



