#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define PATH_BUF_SIZE 512
#define LINE_BUF_SIZE 512
#define DESIG_BUF_SIZE 64
#define HEADER_BUF_SIZE 64

FILE* open_calendar_file(void) {
    char path[PATH_BUF_SIZE];
    char *home = getenv("HOME");
    if (home == NULL) {
        fprintf(stderr, "Error: HOME environment variable not set.\n");
        return NULL;
    }
    snprintf(path, sizeof(path), "%s/.kalendar", home);
    return fopen(path, "r");
}

int match_designator(const char *designator, struct tm *current_time) {
    if (strcmp(designator, "*") == 0) {
        return 1;
    }

    char today_stub[32];
    strftime(today_stub, sizeof(today_stub), "%b %e", current_time);
    
    if (strcasecmp(designator, today_stub) == 0) {
        return 1;
    }

    return 0; 
}

void read_calendar_stream(FILE *fp, int verbose) {
    char line_buffer[LINE_BUF_SIZE];
    char current_designator[DESIG_BUF_SIZE] = {0}; 
    char header_buffer[HEADER_BUF_SIZE];
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    // 1. Format and print the day being evaluated
    // %A = Full weekday name (e.g., Monday)
    // %B = Full month name (e.g., May)
    // %d = Day of the month as a decimal number (01-31)
    strftime(header_buffer, sizeof(header_buffer), "%A, %B %d", t);
    printf("=== Events for %s ===\n", header_buffer);

    // 2. Loop through the file stream
    while (fgets(line_buffer, sizeof(line_buffer), fp) != NULL) {
        if (line_buffer[0] == '#') {
            if (verbose) {
                printf("%s", line_buffer);
            }
            continue;
        }

        char *tab_ptr = strchr(line_buffer, '\t');
        if (tab_ptr == NULL) {
            continue; 
        }

        if (line_buffer[0] != '\t') {
            size_t desig_len = tab_ptr - line_buffer;
            if (desig_len >= DESIG_BUF_SIZE) {
                desig_len = DESIG_BUF_SIZE - 1;
            }
            strncpy(current_designator, line_buffer, desig_len);
            current_designator[desig_len] = '\0'; 
            
            while (desig_len > 0 && current_designator[desig_len - 1] == ' ') {
                current_designator[--desig_len] = '\0';
            }
        }

        if (match_designator(current_designator, t)) {
            printf("%s", tab_ptr + 1);
        }
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

    read_calendar_stream(fp, verbose);

    fclose(fp);
    return 0;
}
