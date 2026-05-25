#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_BUF_SIZE 512
#define LINE_BUF_SIZE 512

FILE* open_calendar_file(void) {
    char path[PATH_BUF_SIZE];
    
    char *home = getenv("HOME");
    if (home == NULL) {
        fprintf(stderr, "Error: HOME environment variable not set.\n");
        return NULL;
    }

    snprintf(path, sizeof(path), "%s/.kalendar", home);

    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "unable to open calendar file at %s\n", path);
        return NULL;
    }

    return fp;
}

/* * Subroutine: read_calendar_stream
 * Reads the file line-by-line into a local buffer and prints it.
 */
void read_calendar_stream(FILE *fp) {
    char line_buffer[LINE_BUF_SIZE];

    // fgets reads up to newline or LINE_BUF_SIZE - 1, keeping it safe
    while (fgets(line_buffer, sizeof(line_buffer), fp) != NULL) {
        // For now, we dump the raw line buffer directly to stdout
        printf("%s", line_buffer);
    }
}

int main(int argc, char *argv[]) {
    int verbose = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        }
    }

    FILE *fp = open_calendar_file();
    if (fp == NULL) {
        return 1; 
    }

    if (verbose) {
        printf("Success! Calendar file opened smoothly via subroutine.\n\n");
    }

    // Call our new streaming subroutine
    read_calendar_stream(fp);

    fclose(fp);
    return 0;
}
