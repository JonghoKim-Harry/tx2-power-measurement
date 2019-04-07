#ifndef SYSFS_STAT_H
#define SYSFS_STAT_H

#include <time.h>
#include <sys/types.h>
#include <stdarg.h>

#define MAX_NUM_SYSFS_FD    8
#define ONE_SYSFS_FILE      1
#define TWO_SYSFS_FILES     2
#define THREE_SYSFS_FILES   3
#define FOUR_SYSFS_FILES    4
#define FIVE_SYSFS_FILES    5
#define SIX_SYSFS_FILES     6
#define SEVEN_SYSFS_FILES   7
#define EIGHT_SYSFS_FILES   8

struct sysfs_stat {

#ifdef DEBUG
    char column_name[256];
#endif   // DEBUG

    //
    ssize_t (*read_sysfs_func)(const struct sysfs_stat stat_info, const int rawdata_fd);
    int num_sysfs_fd;
    int max_strlen[MAX_NUM_SYSFS_FD];
    int sysfs_fd[MAX_NUM_SYSFS_FD];

    //
    ssize_t (*rawdata_to_stat_func)(const struct sysfs_stat info, const int rawdata_fd, const int stat_fd);
    int column_width;
    char stat_format[64];
};

#ifdef DEBUG
void print_stat_info(const struct sysfs_stat stat_info);
#endif   // DEBUG


#endif   // SYSFS_STAT_H
