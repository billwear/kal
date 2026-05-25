#include <stdio.h>
#include <stdlib.h>

int main(void) {
	FILE *fp;
	char path[512];

	char *home = getenv("HOME");
	if (home == NULL) {
		fprintf(stderr, "Error: $HOME not set.\n");
		return 1;
	}

	snprintf(path, sizeof(path), "%s/.kalendar", home);

	fp = fopen(path,"a");
	if(fp == NULL) {
		printf("unable to open calendar file at %s\n", path);
		return 1;
	}

	fclose(fp);
}
