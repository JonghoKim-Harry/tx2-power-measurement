#ifndef STAT_H
#define STAT_H

#include <stdio.h>
#include "default_values.h"

// Do NOT include measurement_info.h
struct measurement_info_struct;

enum logtype_t {
    LOGTYPE_NA = 0,
    LOGTYPE_TIME,
    LOGTYPE_TIMESTAMP,
    LOGTYPE_POWERLOG,
    LOGTYPE_SUMMARY,
    LOGTYPE_CAFFELOG,
    LOGTYPE_TEGRALOG,
    NUM_LOGTYPES
};

typedef struct stat_info_struct {

    char colname[MAX_COLWIDTH];
    int  colwidth;
    enum logtype_t logtype;
    ssize_t (*func_log_to_stat)(const int stat_fd, const int colwidth, ...);
} stat_info_struct;


struct row_info_struct {
    char message[MAX_ROWWIDTH];
    ssize_t (*func_log_to_stat)(const int stat_fd, const int colwidth, ...);
    size_t num_data;
    void *data1, *data2;
    int colwidth;
    char unit[MAX_UNIT_STRLEN];
} row_info;

// For registering rows
//extern const struct row_info_struct row_minmax_gpu_util;
//extern const struct row_info_struct row_minmax_gpu_freq;
//extern const struct row_info_struct row_minmax_gpu_power;
extern const struct row_info_struct row_avg_gpu_util;
extern const struct row_info_struct row_gpu_energy;
extern const struct row_info_struct row_board_energy;

void register_row_message(
        struct measurement_info_struct *info,
        const char *message
);

void register_row1(
     struct measurement_info_struct *info,
     struct row_info_struct row_info,
     void *data1
);

void register_row2(
     struct measurement_info_struct *info,
     struct row_info_struct row_info,
     void *data1,
     void *data2
);

void print_registered_rows(const int stat_fd, const struct measurement_info_struct info);

void register_stat(
     struct measurement_info_struct *info,
     const char *colname,
     const int colwidth,
     enum logtype_t logtype,
     ssize_t (*func_log_to_stat)(const int stat_fd, const int colwidth, ...)
);

off_t print_expinfo(const int stat_fd, const struct measurement_info_struct info);
ssize_t print_header_row(const int stat_fd, const struct measurement_info_struct info);

#endif   // STAT_H
