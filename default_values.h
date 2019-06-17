#ifndef DEFAULT_VALUES_H
#define DEFAULT_VALUES_H

#define MIN_TIME_INTERVAL        10000   // 10000 us = 10 ms

// If you want to set owner of result directory, modify these values:
#define DEFAULT_UID   1001   // nvidia
#define DEFAULT_GID   1001   // nvidia

#define MAX_BUFFLEN   256

// sysfs fds
#define MAX_NUM_SYSFS_FD     8
#define  NO_SYSFS_FILE       0
#define ONE_SYSFS_FILE       1
#define TWO_SYSFS_FILES      2
#define THREE_SYSFS_FILES    3
#define FOUR_SYSFS_FILES     4
#define FIVE_SYSFS_FILES     5
#define SIX_SYSFS_FILES      6
#define SEVEN_SYSFS_FILES    7
#define EIGHT_SYSFS_FILES    8

#define MAX_NUM_RAWDATA     20
#define MAX_NUM_STAT        20
#define RAWDATA_BUFFSIZE    64

/* CAUTION: Do NOT decrease this value */
#define MAX_COLWIDTH       128

#define NUM_CPUS             6


#endif   // DEFAULT_VALUES_H
