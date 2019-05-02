#ifndef REGISTER_RAWDATA_H
#define REGISTER_RAWDATA_H

#include <time.h>
#include <sys/types.h>
#include <stdarg.h>

#include "measurement_info.h"

#define NO_SYSFS_FILES      0
#define ONE_SYSFS_FILE      1
#define TWO_SYSFS_FILES     2
#define THREE_SYSFS_FILES   3
#define FOUR_SYSFS_FILES    4
#define FIVE_SYSFS_FILES    5
#define SIX_SYSFS_FILES     6
#define SEVEN_SYSFS_FILES   7
#define EIGHT_SYSFS_FILES   8

// TODO: register_rawdata()
void register_sysfs
    (struct measurement_info *info,
     ssize_t (*read_sysfs_func)(const struct sysfs_stat stat_info, const int rawdata_fd),
     ssize_t (*rawdata_to_stat_func)(const struct sysfs_stat, const int rawdata_fd, const int stat_fd),
     const char *column_name,
     const char *stat_format,
     const int num_sysfs_file, ...);

// TODO: Rename: close_sysfs() -> close_sysfs_files()
void close_sysfs(struct measurement_info info);

// TODO: rawdata_to_powerlog
ssize_t sysfs_to_stat(const int stat_fd, const char *sysfs_filename, const char *stat_format);

// TODO: read_rawdata_1
ssize_t read_sysfs_1(const struct sysfs_stat stat_info, const int rawdata_fd);
// TODO: read_rawdata_2
ssize_t read_sysfs_2(const struct sysfs_stat stat_info, const int rawdata_fd);

// TODO: rawdata_to_powerlog_1
ssize_t rawdata_to_stat_1(const struct sysfs_stat stat_info, const int rawdata_fd, const int stat_fd);
// TODO: rawdata_to_powerlog_util
ssize_t rawdata_to_stat_util(const struct sysfs_stat stat_info, const int rawdata_fd, const int stat_fd);
// TODO: rawdata_to_powerlog_2
ssize_t rawdata_to_stat_2(const struct sysfs_stat stat_info, const int rawdata_fd, const int stat_fd);

#endif   // REGISTER_RAWDATA_H
