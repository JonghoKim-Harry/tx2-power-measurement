#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include "parse_caffelog.h"
#include "constants.h"

#define NO_REGEX_EFLAGS   0

typedef struct caffelog_parser_struct {
    regex_t caffelog_basic_regex;
    regex_t first_batch_start_regex;
    regex_t batch_finish_regex;
    int batch_idx;
    int num_batch;
    uint8_t flag_cnn;
} caffelog_parser_struct;

static caffelog_parser_struct caffelog_parser = {};

#define CNN_FLAG_INITIAL_VALUE          0x0
#define CNN_FLAG_START                    (1)
#define CNN_FLAG_FINISH                   (1 << 1)
#define CNN_STARTED(flag)        (flag & CNN_FLAG_START)  == CNN_FLAG_START
#define CNN_NOT_STARTED(flag)    (flag & CNN_FLAG_START)  != CNN_FLAG_START
#define CNN_FINISHED(flag)       (flag & CNN_FLAG_FINISH) == CNN_FLAG_FINISH
#define CNN_NOT_FINISHED(flag)   (flag & CNN_FLAG_FINISH) != CNN_FLAG_FINISH

// match[0]: The whole match. Do not care about it
// match[1]: Timestamp
// match[2]: Event
#define CAFFELOG_BASIC_REGEX \
    "[[:alpha:][:space:]]*" \
    "([[:digit:]]{2}:[[:digit:]]{2}:[[:digit:]]{2}[.][[:digit:]]{6})" \
    "[^\]]*\][[:space:]]*" \
    "([^[:space:]].*)"

// For example,
// I0617 14:59:31.418915 23665 caffe.cpp:281] Running for 50 iterations.
//
// match[1]: Number of batches
#define FIRST_BATCH_START_REGEX \
    "[[:alpha:][:space:][:digit:]]*" \
    "Running for ([[:digit:]]+) iteration" \
    ".*"

// For example,
// I0617 14:59:33.499068 23665 caffe.cpp:304] Batch 37, accuracy = 0.79
//
// match[1]: (Batch# - 1), because raw caffelog is zero-indexed
#define BATCH_FINISH_REGEX \
    "[[:alpha:][:space:][:digit:]]*" \
    "Batch[[:space:]]*([[:digit:]]+), accuracy = [[:digit:]][.][[:digit:]]+" \
    ".*"

void init_caffelog_parser() {

    caffelog_parser.batch_idx = -2;
    caffelog_parser.flag_cnn = CNN_FLAG_INITIAL_VALUE;

    /*
     *   Ignore the compilation warning message:
     *
     *      warning: unknown escape sequence: '\]'
     *
     *   REG_EXTENDED supports the escape sequence '\]', thus just ignore it
     */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
    // Produce special data structure for fast regex execution
    regcomp(&caffelog_parser.caffelog_basic_regex, CAFFELOG_BASIC_REGEX, REG_EXTENDED);
    regcomp(&caffelog_parser.first_batch_start_regex, FIRST_BATCH_START_REGEX, REG_EXTENDED);
    regcomp(&caffelog_parser.batch_finish_regex, BATCH_FINISH_REGEX, REG_EXTENDED);
#pragma GCC diagnostic pop

    return;
}

int64_t diff_timestamp(const struct timespec timestamp1, const struct timespec timestamp2) {

    /*
     *   This function compares two timestamps.
     *   Like strcmp() function, this function returns positive number
     *   if timestamp1 is larger, zero if both timestamp is identical,
     *   or negative number if timestamp2 is larger.
     *
     *   This function returns the difference of two timestamps in nanosecond
     *   scale. If the difference is more than range of int64_t, this function
     *   returns INT64_MIN or INT64_MAX. However, there are enuogh room
     *   because the range covers about 292 years
     */

    int64_t diff_sec, diff_nsec, diff_ns;

    diff_sec   = timestamp1.tv_sec  - timestamp2.tv_sec;
    diff_nsec  = timestamp1.tv_nsec - timestamp2.tv_nsec;

    diff_ns   = ONE_SECOND_TO_NANOSECOND * diff_sec + diff_nsec;
    return diff_ns;
}

off_t parse_caffelog(const int caffelog_fd, const off_t offset, const struct tm calendar, caffelog_struct *caffelog) {

    /*
     *  This function returns end of line of parsed caffelog.
     *  If -1 is returned, the result should be ignored
     */
    off_t new_offset = offset;

    // Timestamp
    const size_t max_regexmatch = 2 + 1;   // 0th is whole match
    regmatch_t matched_regex[max_regexmatch];
    char timebuff[7];
    const char *start_ptr;

    // Line
    char buff[256];
    const char *eol;
    ssize_t read_bytes;

    char event_buff[256];

    // Flag
    int detect_something;
    int detect_batch_finish;

#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   START");
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

    if(offset < 0) {
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   FINISHED: Maybe end of file reached");
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

        return -1;
    }

    // Copy calendar informations
    caffelog->calendar_date.tm_isdst = calendar.tm_isdst;
    caffelog->calendar_date.tm_yday  = calendar.tm_yday;
    caffelog->calendar_date.tm_wday  = calendar.tm_wday;
    caffelog->calendar_date.tm_year  = calendar.tm_year;
    caffelog->calendar_date.tm_mon   = calendar.tm_mon;
    caffelog->calendar_date.tm_mday  = calendar.tm_mday;

read_a_line:
    read_bytes = pread(caffelog_fd, buff, 256, new_offset);

    if(read_bytes <= 0) {
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
        perror("pread() FAIL");
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

        return -1;
    }

    for(eol=buff; (*eol)!='\n'; eol++) {/* Just finding line */}
    buff[eol-buff] = '\0';

    new_offset += (eol-buff+1);

#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   Line: %s", buff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

    if(regexec(&caffelog_parser.caffelog_basic_regex, buff, (2 + 1), matched_regex, NO_REGEX_EFLAGS))
        goto read_a_line;

#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   matched_regex[0].rm_so: %u", matched_regex[0].rm_so);
    printf("\nparse_caffelog()   matched_regex[0].rm_eo: %u", matched_regex[0].rm_eo);
    printf("\nparse_caffelog()   matched_regex[1].rm_so: %u", matched_regex[1].rm_so);
    printf("\nparse_caffelog()   matched_regex[1].rm_eo: %u", matched_regex[1].rm_eo);
    printf("\nparse_caffelog()   matched_regex[2].rm_so: %u", matched_regex[2].rm_so);
    printf("\nparse_caffelog()   matched_regex[2].rm_eo: %u", matched_regex[2].rm_eo);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

    if(matched_regex[0].rm_so == matched_regex[0].rm_eo) {
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   No Match");
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG
        goto read_a_line;
    }

    // Hour
    start_ptr = buff + matched_regex[1].rm_so;
    strncpy(timebuff, start_ptr, 2);
    timebuff[2] = '\0';
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   timebuff hour: %s", timebuff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG
    caffelog->calendar_date.tm_hour = atoi(timebuff);

    // Minute
    start_ptr += 3;
    strncpy(timebuff, start_ptr, 2);
    timebuff[2] = '\0';
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   timebuff min: %s", timebuff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG
    caffelog->calendar_date.tm_min = atoi(timebuff);

    // Second
    start_ptr += 3;
    strncpy(timebuff, start_ptr, 2);
    timebuff[2] = '\0';
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   timebuff sec: %s", timebuff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG
    caffelog->calendar_date.tm_sec = atoi(timebuff);
    (&caffelog->timestamp)->tv_sec = mktime(&caffelog->calendar_date);


    // Nanosecond
    start_ptr += 3;
    strncpy(timebuff, start_ptr, 6);
    timebuff[6] = '\0';
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   timebuff ns: %s", timebuff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG
    (&caffelog->timestamp)->tv_nsec = MICROSECOND_TO_NANOSECOND * atoi(timebuff);

    // Event
    // Note that putting caffelog message in " " makes MS Excel to recognize
    // it as a single string
    strcpy(caffelog->event, "\"[Caffe] ");
    strcat(caffelog->event, buff + matched_regex[2].rm_so);
    strcat(caffelog->event, "\"");
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   event: %s", caffelog->event);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

    // Batch Index
    // Note that (batch idx) <- (detected_batch_idx + 1)
    strcpy(event_buff, caffelog->event);

    detect_something = 0;
    detect_batch_finish = 0;
    caffelog->cnn_start = -1;
    caffelog->cnn_finish = -1;

    if(CNN_NOT_STARTED(caffelog_parser.flag_cnn)) {
        if(!regexec(&caffelog_parser.first_batch_start_regex, event_buff, (1 + 1), matched_regex, NO_REGEX_EFLAGS)) {

            detect_something = 1;
            caffelog->cnn_start = 100;

            // Parse the number of batches
            strncpy(buff, event_buff + matched_regex[1].rm_so, (matched_regex[1].rm_eo - matched_regex[1].rm_so + 1));
            caffelog_parser.num_batch = atoi(buff);
            caffelog->batch_idx = 1;
            caffelog_parser.flag_cnn |= CNN_FLAG_START;
        }
    }
    else if(CNN_NOT_FINISHED(caffelog_parser.flag_cnn)) {
        if(!regexec(&caffelog_parser.batch_finish_regex, event_buff, (1 + 1), matched_regex, NO_REGEX_EFLAGS)) {

            detect_something = 1;
            detect_batch_finish = 1;

            // Parse batch index by detecting batch finishes
            strncpy(buff, event_buff + matched_regex[1].rm_so, (matched_regex[1].rm_eo - matched_regex[1].rm_so + 1));
            caffelog->batch_idx = atoi(buff) + 1;
            caffelog_parser.batch_idx = caffelog->batch_idx + 1;

            if(caffelog_parser.batch_idx > caffelog_parser.num_batch) {
                caffelog->cnn_finish = 100;
                caffelog_parser.flag_cnn |= CNN_FLAG_FINISH;
                caffelog_parser.batch_idx = -1;
            }
        }
    }

    if(!detect_something) // Nothing detected, thus guess batch number from previous parsing results
       caffelog->batch_idx = caffelog_parser.batch_idx;

    if(detect_batch_finish)
        caffelog->batch_finish = 100;
    else
        caffelog->batch_finish = -1;

#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   FINISHED");
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

    return new_offset;
}

void free_caffelog_parser() {

    // Free objects
    regfree(&caffelog_parser.caffelog_basic_regex);
    regfree(&caffelog_parser.first_batch_start_regex);
    regfree(&caffelog_parser.batch_finish_regex);

    return;
}

#undef CAFFELOG_BASIC_REGEX
#undef FIRST_BATCH_START_REGEX
#undef BATCH_FINISH_REGEX
