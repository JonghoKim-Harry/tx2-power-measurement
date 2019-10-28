#ifndef GOVERNOR_H
#define GOVERNOR_H

#include <sys/types.h>
#include <stdint.h>
#include "../list.h"
#include "../tx2_sysfs_power.h"
#include "../default_values.h"

void init_gpugovernor();
void select_gpugovernor(const char *gpugov_name, void *data);
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
    void (*print_gpugov)(int fd);
};

extern struct gpugov   *curr_gpugov;
extern struct gpugov   do_nothing;
extern struct gpugov   scale_100;
extern struct gpugov   scale_10;
extern struct gpugov   scale_always;
extern struct gpugov   ondemand8050;
extern struct gpugov   policy_0_or_not;
extern struct gpugov   maxmin_cap_b;
extern struct gpugov   cnngov_a;
extern struct gpugov   cnngov_b;
extern struct gpugov   cnngov_c;
extern struct gpugov   cnngov_c_30;
extern struct gpugov   cnngov_c_40;
extern struct gpugov   cnngov_d;
extern struct gpugov   cnngov_20190919;
extern struct gpugov   cnngov_20190920;
extern struct gpugov   cnngov_dyn_th;
extern struct gpugov   cnngov_dyn_th2_th3;
extern struct gpugov   slow_scale;
extern struct gpugov   feas_test;
extern struct gpugov   cnngov_e;
extern struct gpugov   hilo;
extern struct gpugov   pingpong;
extern struct gpugov   emc_conservative;

/**
 *  Informations for GPU governors
 */
// GPU Frequency
extern int fd_read_gpufreq;
int32_t get_gpufreq();   // Returns in Hz
extern int fd_write_gpufreq;
ssize_t set_gpufreq(int32_t gpufreq);

// GPU Utilization
extern int fd_gpuutil;
int16_t get_gpuutil();   // Returns in x0.1%

// GPU Power
extern int fd_gpupower;
int16_t get_gpupower();

// EMC Utilization
extern int fd_emcutil;
int64_t get_emcutil();   // Returns in x0.0001%

/**
 *  Frequency transformation: Hz <--> frequency level
 */
int32_t gpufreq_level_to_hz(int level);
int     gpufreq_hz_to_level(int32_t gpufreq);

/**
 *  Return frequency equal or greater than n% of given frequency
 *  @gpufreq: Given frequency in Hz
 *  @n: Scale down factor (integer, percent)
 */
int32_t scale_down_by_n(const int32_t gpufreq, int n);

/**
 *  Return frequency equal or less than n% of given frequency
 *  @gpufreq: Given frequency in Hz
 *  @n: Scale up factor (integer, percent)
 */
int32_t scale_up_by_n(const int32_t gpufreq, int n);

/**
 *  Scale down given frequency by n% of maximum frequency.
 *  Return frequency equal or greater than the target frequency
 *  @gpufreq: Given frequency in Hz
 *  @n: Scale down factor (integer, percent)
 */
int32_t scale_down_by_n_of_max(const int32_t gpufreq, int n);

/**
 *  Scale up given frequency by n% of maximum frequency.
 *  Return frequency equal or less than the target frequency
 *  @gpufreq: Given frequency in Hz
 *  @n: Scale up factor (integer, percent)
 */
int32_t scale_up_by_n_of_max(const int32_t gpufreq, int n);


#endif   // GOVERNOR_H
