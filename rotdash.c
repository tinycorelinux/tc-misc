#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv){

	if (argc != 2) return 1;

	const char arr[] = { '/', '-', '\\', '|' };
	unsigned int i = 0;
	chdir("/proc");

	while (access(argv[1],F_OK) == 0) {
		printf("%c\b",arr[i]);
		fflush(stdout);
		i++;
		i %= 4;
		usleep(100000);
	}

	return 0;
}
