#ifndef GOVERNOR_H
#define GOVERNOR_H

#include <sys/types.h>
#include <stdint.h>
#include "../tx2_sysfs_power.h"

extern const int32_t available_gpufreq[TX2_SYSFS_NUM_AVAILABLE_GPUFREQ];

extern int max_gpufreq;
extern int min_gpufreq;
extern int fd_write_gpufreq;
extern int fd_gpufreq;
extern int fd_gpuutil;
extern int fd_gpupower;

void start_gpugovernor();
int32_t determine_gpufreq();        // You should implement this function
void finish_gpugovernor();

int32_t get_gpufreq();
int16_t get_gpuutil();
int16_t get_gpupower();

ssize_t set_gpufreq(const int32_t gpufreq);

#endif   // GOVERNOR_H
