#ifndef DEFAULT_VALUES_H
#define DEFAULT_VALUES_H

#include <stdint.h>
#define MIN_TIME_INTERVAL        10000   // 10000 us = 10 ms

// If you want to set owner of result directory, modify these values:
#define DEFAULT_UID   1001   // nvidia
#define DEFAULT_GID   1001   // nvidia

#define MAX_BUFFLEN   256
#define GPU_GOVERNOR_NAME_LEN   64

// For initialization
#define INIT_MAX            -1   // Maximum values are simply initialized to (-1)
#define INIT_MIN     INT16_MAX   // Minimum values are initialized to maximum possitive numbers
#define INIT_SUM             0   // Summation values should be initialized to 0

// For MS Excel
#define INFINITE           100

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
