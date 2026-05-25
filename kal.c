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
 * Current rules: * , MMM * , M * , M / * , M-* , MMM DD, M DD, M/DD, M-DD, MMM DD YYYY, WDAYS, MACROS, ORDINALS
 */
int match_designator(const char *designator, struct tm *current_time) {
    // 1. Global daily wildcard
    if (strcmp(designator, "*") == 0) {
        return 1;
    }

    // Extract current time contexts
    int current_mon = current_time->tm_mon + 1; // 1-12
    int current_mday = current_time->tm_mday;
    int current_year = current_time->tm_year + 1900;
    int wday = current_time->tm_wday;           // 0=Sun, 1=Mon, ..., 6=Sat

    // Create a local, lowercase copy of the designator for safe matching
    char clean_desig[DESIG_BUF_SIZE];
    int i = 0;
    while (designator[i] && i < (DESIG_BUF_SIZE - 1)) {
        clean_desig[i] = tolower((unsigned char)designator[i]);
        i++;
    }
    clean_desig[i] = '\0';

    // 2. Specific Day-of-the-Week named patterns
    if (strcmp(clean_desig, "sun") == 0 || strcmp(clean_desig, "sunday") == 0)    return (wday == 0);
    if (strcmp(clean_desig, "mon") == 0 || strcmp(clean_desig, "monday") == 0)    return (wday == 1);
    if (strcmp(clean_desig, "tue") == 0 || strcmp(clean_desig, "tuesday") == 0)   return (wday == 2);
    if (strcmp(clean_desig, "wed") == 0 || strcmp(clean_desig, "wednesday") == 0) return (wday == 3);
    if (strcmp(clean_desig, "thu") == 0 || strcmp(clean_desig, "thursday") == 0)  return (wday == 4);
    if (strcmp(clean_desig, "fri") == 0 || strcmp(clean_desig, "friday") == 0)    return (wday == 5);
    if (strcmp(clean_desig, "sat") == 0 || strcmp(clean_desig, "saturday") == 0)  return (wday == 6);

    // 3. Compound Macro-Days
    if (strcmp(clean_desig, "weekday") == 0) return (wday >= 1 && wday <= 5); // Mon-Fri
    if (strcmp(clean_desig, "weekend") == 0) return (wday == 0 || wday == 6); // Sun or Sat

    // 4. NEW STEP: Relative Ordinal Weekdays (e.g., "4th mon", "4th monday")
    int parsed_ordinal = 0;
    char parsed_suffix[4] = {0};
    char parsed_wday[32] = {0};
    
    // Attempt parsing something like "4th mon" or "4th monday"
    // %1d grabs the digit, %2s grabs "th"/"st"/"nd"/"rd", and %31s grabs the day string
    if (sscanf(clean_desig, "%1d%2s %31s", &parsed_ordinal, parsed_suffix, parsed_wday) == 3) {
        int today_ordinal = ((current_mday - 1) / 7) + 1;
        
        if (parsed_ordinal == today_ordinal) {
            if ((wday == 0 && (strcmp(parsed_wday, "sun") == 0 || strcmp(parsed_wday, "sunday") == 0)) ||
                (wday == 1 && (strcmp(parsed_wday, "mon") == 0 || strcmp(parsed_wday, "monday") == 0)) ||
                (wday == 2 && (strcmp(parsed_wday, "tue") == 0 || strcmp(parsed_wday, "tuesday") == 0)) ||
                (wday == 3 && (strcmp(parsed_wday, "wed") == 0 || strcmp(parsed_wday, "wednesday") == 0)) ||
                (wday == 4 && (strcmp(parsed_wday, "thu") == 0 || strcmp(parsed_wday, "thursday") == 0)) ||
                (wday == 5 && (strcmp(parsed_wday, "fri") == 0 || strcmp(parsed_wday, "friday") == 0)) ||
                (wday == 6 && (strcmp(parsed_wday, "sat") == 0 || strcmp(parsed_wday, "saturday") == 0))) {
                return 1;
            }
        }
    }

    // 5. Pure Numeric Month Wildcards (e.g., "5 *", "5/*", "5-*")
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

    // 6. Exact Pure Numeric Dates (e.g., "5 25", "5/25", "5-25")
    int m = 0, d = 0;
    if (sscanf(clean_desig, "%d/%d", &m, &d) == 2 ||
        sscanf(clean_desig, "%d-%d", &m, &d) == 2 ||
        sscanf(clean_desig, "%d %d", &m, &d) == 2) {
        if (m == current_mon && d == current_mday) {
            return 1;
        }
    }

    // 7. Textual Month Parsing Rules (e.g., "may *", "September 25", "sept 25, 2026")
    char month_token[32];
    char remainder[32] = {0};
    int scan_count = sscanf(clean_desig, "%31s %31[^\n]", month_token, remainder);
    
    if (scan_count >= 1) {
        if (strcmp(month_token, "sept") == 0) {
            strcpy(month_token, "sep");
        }

        struct tm parsed_tm = {0};
        char *strptime_result = strptime(month_token, "%b", &parsed_tm);
        if (strptime_result == NULL || *strptime_result != '\0') {
            strptime_result = strptime(month_token, "%B", &parsed_tm);
        }

        if (strptime_result != NULL && *strptime_result == '\0') {
            int parsed_mon = parsed_tm.tm_mon + 1; // 1-12

            if (scan_count == 2 && strcmp(remainder, "*") == 0) {
                if (parsed_mon == current_mon) return 1;
            }

            int parsed_day = 0;
            int parsed_year = 0;
            if (scan_count == 2) {
                int day_scan = sscanf(remainder, "%d , %d", &parsed_day, &parsed_year);
                if (day_scan != 2) {
                    day_scan = sscanf(remainder, "%d %d", &parsed_day, &parsed_year);
                }

                if (day_scan >= 1) {
                    if (parsed_mon == current_mon && parsed_day == current_mday) {
                        if (day_scan == 2 && parsed_year != current_year) {
                            return 0; 
                        }
                        return 1;
                    }
                }
            }
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
