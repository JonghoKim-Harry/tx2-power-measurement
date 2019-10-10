#ifndef SUMMARY_H
#define SUMMARY_H

#include "measurement_info.h"
#include "powerlog.h"
#include "default_values.h"

typedef struct summary_struct {

    struct timespec          start_timestamp, finish_timestamp;
    struct powerlog_struct   last_powerlog;
    int                      num_powerlog;
    char                     name[MAX_BUFFLEN];

    // GPU utilization and product-sum of utilization-time
    int16_t min_gpu_util;              // e-1%
    int16_t max_gpu_util;              // e-1%
    int64_t psum_gpu_util_e2ms;        // e2% x ms = e-1% x sec
    int64_t psum_gpu_util_e2ps;        // e2% x ps = e-1% x ns

    // GPU frequency
    int16_t min_gpu_freq;              // MHz
    int16_t max_gpu_freq;              // MHz

    // System power and energy
    int32_t min_system_power;          // mW
    int32_t max_system_power;          // mW
    int64_t system_energy_J;
    int64_t system_energy_mJ;
    int64_t system_energy_uJ;
    int64_t system_energy_pJ;
    int64_t system_energy_fJ;

    // GPU power and energy
    int16_t min_gpu_power;             // mW
    int16_t max_gpu_power;             // mW
    int64_t gpu_energy_J;              // joule = Watt x second
    int64_t gpu_energy_mJ;             // milli: 10^(-3)            
    int64_t gpu_energy_uJ;             // micro: 10^(-6)
    int64_t gpu_energy_pJ;             // pico:  10^(-12)
    int64_t gpu_energy_fJ;             // femto: 10^(-15)

    // Board power and energy
    int32_t min_board_power;           // mW
    int32_t max_board_power;           // mW
    int64_t board_energy_J;            // joule = Watt x second
    int64_t board_energy_mJ;           // milli: 10^(-3)
    int64_t board_energy_uJ;           // micro: 10^(-6)
    int64_t board_energy_pJ;           // pico:  10^(-12)
    int64_t board_energy_fJ;           // femto: 10^(-15)

#ifdef TRACE_CPU
    // Note that CPUs can be power-gated; therefore,
    // avgerage utilization/frequency should be calculated carefully
    int16_t min_cpu_util[NUM_CPUS];    // x0.1%
    int16_t max_cpu_util[NUM_CPUS];    // x0.1%
    int32_t min_cpu_freq[NUM_CPUS];    // MHz
    int32_t max_cpu_freq[NUM_CPUS];    // MHz

    // The power consumptions of CPUs are measured together
    int16_t min_allcpu_power;          // mW
    int16_t max_allcpu_power;          // mW
    int64_t allcpu_energy_J;           // joule = Watt x second
    int64_t allcpu_energy_mJ;          // milli: 10^(-3)
    int64_t allcpu_energy_uJ;          // micro: 10^(-6)
    int64_t allcpu_energy_pJ;          // pico:  10^(-12)
    int64_t allcpu_energy_fJ;          // femto: 10^(-15)
#endif   // TRACE_CPU

#ifdef TRACE_MEM
    // EMC utilization and product-sum of utilization-time
    int32_t min_emc_util;              // e-4%
    int32_t max_emc_util;              // e-4%
    int64_t psum_emc_util_e2us;        // % * e2us = % * e-4  sec
    int64_t psum_emc_util_e2fs;        // % * e2fs = % * e-13 sec

    // EMC frequency and product-sum of frequency-time
    int16_t min_emc_freq;              // MHz
    int16_t max_emc_freq;              // MHz
    int64_t psum_emc_freq_sec;         //
    int64_t psum_emc_freq_ns;          //

    // Memory power consumption
    int16_t min_mem_power;             // mW
    int16_t max_mem_power;             // mW
    int64_t mem_energy_J ;             // joule = Watt x second
    int64_t mem_energy_mJ;             // milli: 10^(-3)
    int64_t mem_energy_uJ;             // micro: 10^(-6)
    int64_t mem_energy_pJ;             // pico:  10^(-12)
    int64_t mem_energy_fJ;             // femto: 10^(-15)
#endif   // TRACE_MEM
} summary_struct;

/**/
void init_summary(summary_struct *summary, const char *_name);
void update_summary(summary_struct *summary, const powerlog_struct *powerlog_ptr);
void print_summary(int fd, const summary_struct *summary);
struct timespec summary_runtime(const summary_struct summary);

/* Print functions */
void print_summary_name(int fd, const summary_struct *summary);
void print_summary_runtime(int fd, const summary_struct *summary);
void print_summary_gpu_util_range(int fd, const summary_struct *summary);
void print_summary_emc_util_range(int fd, const summary_struct *summary);
void print_summary_gpu_freq_range(int fd, const summary_struct *summary);
void print_summary_gpu_power_range(int fd, const summary_struct *summary);

#endif   // SUMMARY_H
