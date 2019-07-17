#ifndef UPDATE_SUMMARY_H
#define UPDATE_SUMMARY_H

#include "measurement_info.h"

typedef struct summary_struct {

    struct timespec start_timestamp, finish_timestamp;
    struct powerlog_struct last_powerlog;
    int                     num_powerlog;

    // GPU Utilization
    int16_t min_gpu_util;        // x0.1%
    int16_t max_gpu_util;        // x0.1%

    // Area of time-utilization graph
    // in order to calculate average utilization
    int64_t area_gpu_util_sec;   // 0.1 % * sec
    int64_t area_gpu_util_ns;    // 0.1 % * ns

    // GPU Frequency
    int16_t min_gpu_freq;        // MHz
    int16_t max_gpu_freq;        // MHz

    // GPU Power
    int16_t min_gpu_power;       // mW
    int16_t max_gpu_power;       // mW

    // GPU Energy
    int64_t gpu_energy_J;            // joule = Watt * second
    int64_t gpu_energy_uJ;           // micro: 10^(-6)
    int64_t gpu_energy_pJ;           // pico:  10^(-12)
    int64_t gpu_energy_dotone_pJ;    // 0.1 pJ for remainder calculation

#ifdef TRACE_CPU
    int32_t allcpu_energy_Wh;
    int64_t allcpu_energy_pWh;
    int32_t avg_allcpu_power;    // mW
#endif   // TRACE_CPU

#ifdef TRACE_DDR
    int32_t mem_energy_Wh;
    int64_t mem_energy_pWh;
    int32_t avg_mem_power;       // mW
#endif   // TRACE_DDR
} summary_struct;

void init_summary(summary_struct *summary);
void update_summary(summary_struct *summary, const powerlog_struct *powerlog_ptr);
struct timespec elapsed_time(const summary_struct summary);
#endif   // UPDATE_SUMMARY_H
