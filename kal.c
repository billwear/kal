#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define PATH_BUF_SIZE 512
#define LINE_BUF_SIZE 512
#define DESIG_BUF_SIZE 64
#define HEADER_BUF_SIZE 64

// --- Function Prototypes ---
FILE* open_calendar_file(void);
int match_designator(const char *designator, struct tm *current_time);
void read_calendar_stream(FILE *fp, int verbose);

// ============================================================================
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
// ============================================================================

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

/* * Subroutine: match_designator
 * Current rules: * , MMM * , M * , M / * , M-* , MMM DD, M DD, M/DD, M-DD
 */
int match_designator(const char *designator, struct tm *current_time) {
    // 1. Global daily wildcard
    if (strcmp(designator, "*") == 0) {
        return 1;
    }

    // Extract current time integers (tm_mon is 0-11, so add 1 for 1-12 tracking)
    int current_mon = current_time->tm_mon + 1;
    int current_mday = current_time->tm_mday;

    // Create a local, lowercase copy of the designator for safe matching
    char clean_desig[DESIG_BUF_SIZE];
    int i = 0;
    while (designator[i] && i < (DESIG_BUF_SIZE - 1)) {
        clean_desig[i] = tolower((unsigned char)designator[i]);
        i++;
    }
    clean_desig[i] = '\0';

    // Fetch lowercase short month name (e.g., "may")
    char current_month_lower[16];
    strftime(current_month_lower, sizeof(current_month_lower), "%b", current_time);
    for (int j = 0; current_month_lower[j]; j++) {
        current_month_lower[j] = tolower((unsigned char)current_month_lower[j]);
    }

    // 2. Month Wildcard check ("maybe *")
    char target_pattern[32];
    snprintf(target_pattern, sizeof(target_pattern), "%s *", current_month_lower);
    if (strcmp(clean_desig, target_pattern) == 0) {
        return 1;
    }

    // 3. Numeric Month Wildcards (e.g., "5 *", "5/*", "5-*")
    int parsed_num_month = 0;
    char wildcard_char = '\0';

    if (sscanf(clean_desig, "%d *", &parsed_num_month) == 1) {
        if (parsed_num_month == current_mon) return 1;
    }
    if (sscanf(clean_desig, "%d/%c", &parsed_num_month, &wildcard_char) == 2 && wildcard_char == '*') {
        if (parsed_num_month == current_mon) return 1;
    }
    if (sscanf(clean_desig, "%d-%c", &parsed_num_month, &wildcard_char) == 2 && wildcard_char == '*') {
        if (parsed_num_month == current_mon) return 1;
    }

    // 4. Exact Text Date Match (e.g., "may 25")
    snprintf(target_pattern, sizeof(target_pattern), "%s %d", current_month_lower, current_mday);
    if (strcmp(clean_desig, target_pattern) == 0) {
        return 1;
    }

    // 5. Exact Numeric Date Matrix (e.g., "5 25", "5/25", "5-25")
    int m = 0, d = 0;
    if (sscanf(clean_desig, "%d/%d", &m, &d) == 2 ||
        sscanf(clean_desig, "%d-%d", &m, &d) == 2 ||
        sscanf(clean_desig, "%d %d", &m, &d) == 2) {
        if (m == current_mon && d == current_mday) {
            return 1;
        }
    }

    return 0; 
}

/* * Subroutine: read_calendar_stream
 * Handles comments, invalid lines, tabs, continuation logic, and sanitizing.
 */
void read_calendar_stream(FILE *fp, int verbose) {
    char line_buffer[LINE_BUF_SIZE];
    char current_designator[DESIG_BUF_SIZE] = {0}; 
    char header_buffer[HEADER_BUF_SIZE];
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    strftime(header_buffer, sizeof(header_buffer), "%A, %B %d", t);
    printf("=== Events for %s ===\n", header_buffer);

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
            
            while (desig_len > 0 && isspace((unsigned char)current_designator[--desig_len])) {
                current_designator[desig_len] = '\0';
            }
        }

        if (match_designator(current_designator, t)) {
            printf("%s", tab_ptr + 1);
        }
    }
}
