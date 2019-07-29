#ifndef CAFFELOG_H
#define CAFFELOG_H

#include "measurement_info.h"

typedef struct caffelog_struct {
    struct list_head list;
    struct timespec timestamp;   // GMT

    /*
     *  HMS: Hour, Minute, Second
     *  Note that year, month, day are ignored to be 0
     */
    struct tm calendar_date;   // GMT, HH:MM:SS
//    char event[MAX_COLWIDTH];
    char event[256];

    // Various Events
    int caffe_start;
    int cnn_start;
    int cnn_finish;
    int batch_idx;
    int batch_finish;
} caffelog_struct;

void init_caffelog_parser();
int64_t diff_timestamp(const struct timespec timestamp1, const struct timespec timestamp2);
off_t parse_caffelog(const int caffelog_fd, const off_t offset, const struct tm calendar, caffelog_struct *caffelog);
void free_caffelog_parser();

#endif   // CAFFELOG_H
