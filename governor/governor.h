#ifndef GOVERNOR_H
#define GOVERNOR_H

#include <sys/types.h>
#include <stdint.h>
#include "../list.h"
#include "../tx2_sysfs_power.h"
#include "../default_values.h"

void init_gpugovernor();
void start_gpugovernor(const char *gpugov_name);
void finish_gpugovernor();

/**
 * struct gpugov_info_struct
 * @num_available_gpufreq: the number of available gpu frequencies
 * @available_gpufreq: available gpu frequencies in Hz
 * @max_gpufreq: maximum gpu frequency the governor uses
 * @min_gpufreq: minimum gpu frequency the governor uses
 */
struct gpugov_info_struct {
    size_t num_available_gpufreq;
    int32_t *available_gpufreq;
    int32_t max_gpufreq;
    int32_t min_gpufreq;
};

extern struct gpugov_info_struct   gpugov_info;

/**
 * struct gpugov
 * @node: list node - contains registered userspace gpu governors
 * @name: gpu governor's name
 * @init: initialization function
 * @get_target_freq: function which decides gpu frequency
 */
struct gpugov {
    struct list_head node;

    const char name[GPU_GOVERNOR_NAME_LEN];
    void (*init)(void *);
    int32_t (*get_target_freq)();
};

extern struct gpugov   ondemand8050;
extern struct gpugov   *curr_gpugov;

// Helper functions and variables for GPU governor
extern int fd_write_gpufreq;
extern int fd_read_gpufreq;
extern int fd_gpuutil;
extern int fd_gpupower;
int32_t gpufreq_i(int level);
int32_t level(int32_t gpufreq);
int32_t get_gpufreq();
int16_t get_gpuutil();
int16_t get_gpupower();
ssize_t set_gpufreq(int32_t gpufreq);

#endif   // GOVERNOR_H
