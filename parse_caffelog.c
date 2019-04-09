#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parse_caffelog.h"

#define SECOND_TO_NANOSECOND        1000000000
#define MICROSECOND_TO_NANOSECOND   1000
#define HOUR_TO_SECOND              3600
#define MIN_TO_SECOND               60
#define NO_REGEX_EFLAGS   0

int64_t compare_timestamp_hms(const struct tm timestamp1, const struct tm timestamp2) {

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
     *
     *   This function only compares HMS (Hour, Minute, Second).
     *   Note that year, month, day informations are ignored
     */

    int diff_hour, diff_min, diff_sec;
    int64_t diff_ns;

    diff_hour = timestamp1.tm_hour - timestamp2.tm_hour;
    diff_min  = timestamp1.tm_min  - timestamp2.tm_min;
    diff_sec  = timestamp1.tm_sec  - timestamp2.tm_sec;

    diff_ns   = SECOND_TO_NANOSECOND *
                (HOUR_TO_SECOND * diff_hour
                + MIN_TO_SECOND * diff_min
                + diff_sec);

    return diff_ns;
}

off_t parse_caffelog(const int caffelog_fd, const regex_t timestamp_pattern, const off_t offset, struct caffe_event *event) {

    off_t new_offset = offset;

    // Timestamp
    const size_t num_regexmatch = 2 + 1;   // 0th is whole match
    regmatch_t matched_regex[num_regexmatch];
    char timebuff[7];
    const char *start_ptr;

    // Line
    char buff[256];
    const char *eol;
    ssize_t read_bytes;

#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   START");
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

    if(offset < 0) {
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   FINISHED: Maybe end of file reached");
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

        return -1;
    }

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

    if(regexec(&timestamp_pattern, buff, num_regexmatch, matched_regex, NO_REGEX_EFLAGS))
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
    event->gmt_date_hms.tm_hour = atoi(timebuff);

    // Minute
    start_ptr += 3;
    strncpy(timebuff, start_ptr, 2);
    timebuff[2] = '\0';
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   timebuff min: %s", timebuff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG
    event->gmt_date_hms.tm_min = atoi(timebuff);

    // Second
    start_ptr += 3;
    strncpy(timebuff, start_ptr, 2);
    timebuff[2] = '\0';
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   timebuff sec: %s", timebuff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG
    event->gmt_date_hms.tm_sec = atoi(timebuff);
    (&event->gmt_timestamp)->tv_sec = mktime(&event->gmt_date_hms);


    // Nanosecond
    start_ptr += 3;
    strncpy(timebuff, start_ptr, 6);
    timebuff[6] = '\0';
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   timebuff ns: %s", timebuff);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG
    (&event->gmt_timestamp)->tv_nsec = MICROSECOND_TO_NANOSECOND * atoi(timebuff);

    // Event
    strcpy(event->event, buff + matched_regex[2].rm_so);
#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   event: %s", event->event);
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

#if defined(DEBUG) || defined(DEBUG_PARSE_CAFFELOG)
    printf("\nparse_caffelog()   FINISHED");
#endif   // DEBUG or DEBUG_PARSE_CAFFELOG

    return new_offset;
}
