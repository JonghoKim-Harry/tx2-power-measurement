#ifndef UPDATE_SUMMARY_H
#define UPDATE_SUMMARY_H

#include "measurement_info.h"
#include "powerlog.h"

typedef struct summary_struct {

    struct timespec start_timestamp, finish_timestamp;
    struct powerlog_struct last_powerlog;
    int                     num_powerlog;

    // GPU utilization and product-sum of utilization-time
    int16_t min_gpu_util;        // x0.1%
    int16_t max_gpu_util;        // x0.1%
    int64_t psum_gpu_util_sec;   // % * sec
    int64_t psum_gpu_util_ns;    // % * ns

    // GPU frequency
    int16_t min_gpu_freq;        // MHz
    int16_t max_gpu_freq;        // MHz

    // GPU power and energy
    int16_t min_gpu_power;             // mW
    int16_t max_gpu_power;             // mW
    int64_t gpu_energy_J;              // joule = Watt * second
    int64_t gpu_energy_uJ;             // micro: 10^(-6)
    int64_t gpu_energy_pJ;             // pico:  10^(-12)
    int64_t gpu_energy_dotone_pJ;      // 0.1 pJ for remainder calculation

    int32_t min_board_power;           // mW
    int32_t max_board_power;           // mW
    int64_t all_energy_J;
    int64_t all_energy_mJ;
    int64_t all_energy_uJ;
    int64_t all_energy_pJ;
    int64_t all_energy_fJ;

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
    int32_t allcpu_energy_J;
    int64_t allcpu_energy_pJ;
#endif   // TRACE_CPU

#ifdef TRACE_MEM
    // EMC utilization and product-sum of utilization-time
    int32_t min_emc_util;        // x0.0001%
    int32_t max_emc_util;        // x0.0001%
    int64_t psum_emc_util_ms;    //
    int64_t psum_emc_util_fs;    //

    // EMC frequency and product-sum of frequency-time
    int16_t min_emc_freq;        // MHz
    int16_t max_emc_freq;        // MHz
    int64_t psum_emc_freq_sec;   //
    int64_t psum_emc_freq_ns;    //

    // Memory power consumption
    int16_t min_mem_power;       // mW
    int16_t max_mem_power;       // mW
    int32_t mem_energy_J ;
    int32_t mem_energy_mJ;       // milli: 10^(-3)
    int32_t mem_energy_uJ;       // micro: 10^(-6)
    int64_t mem_energy_pJ;       // pico:  10^(-12)
    int64_t mem_energy_fJ;       // femto: 10^(-15)
#endif   // TRACE_MEM
} summary_struct;

void init_summary(summary_struct *summary);
void update_summary(summary_struct *summary, const powerlog_struct *powerlog_ptr);
struct timespec elapsed_time(const summary_struct summary);
#endif   // UPDATE_SUMMARY_H
