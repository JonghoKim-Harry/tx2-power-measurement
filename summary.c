#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "tx2_sysfs_power.h"
#include "constants.h"
#include "summary.h"

void init_summary(summary_struct *summary, const char *_name) {

#ifdef TRACE_CPU
    int i;
#endif   // TRACE_CPU

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->num_powerlog = 0;
    strcpy(summary->name, _name);

    // Maximum values are initialized to INIT_MAX
    summary->max_gpu_util        = INIT_MAX;
    summary->max_gpu_freq        = INIT_MAX;
    summary->max_gpu_power       = INIT_MAX;
    summary->max_board_power       = INIT_MAX;

#ifdef TRACE_CPU
    for(i=0; i<NUM_CPUS; ++i) {
        summary->max_cpu_util[i] = INIT_MAX;
        summary->max_cpu_freq[i] = INIT_MAX;
    }
    summary->max_allcpu_power    = INIT_MAX;
#endif   // TRACE_CPU

#ifdef TRACE_MEM
    summary->max_emc_util        = INIT_MAX;
    summary->max_emc_freq        = INIT_MAX;
    summary->max_mem_power       = INIT_MAX;
#endif   // TRACE_MEM

    // Minimum values are initialized to INIT_MIN
    summary->min_gpu_util        = INIT_MIN;
    summary->min_gpu_freq        = INIT_MIN;
    summary->min_gpu_power       = INIT_MIN;
    summary->min_board_power       = INIT_MIN;

#ifdef TRACE_CPU
    for(i=0; i<NUM_CPUS; ++i) {
        summary->min_cpu_util[i] = INIT_MIN;
        summary->min_cpu_freq[i] = INIT_MIN;
    }
    summary->min_allcpu_power    = INIT_MIN;
#endif   // TRACE_CPU

#ifdef TRACE_MEM
    summary->min_emc_util        = INIT_MIN;
    summary->min_emc_freq        = INIT_MIN;
    summary->min_mem_power       = INIT_MIN;
#endif   // TRACE_MEM

    // Summation values are initialized to INIT_SUM
    summary->psum_gpu_util_e2ms     = INIT_SUM;
    summary->psum_gpu_util_e2ps     = INIT_SUM;
    summary->system_energy_J        = INIT_SUM;
    summary->system_energy_mJ       = INIT_SUM;
    summary->system_energy_uJ       = INIT_SUM;
    summary->system_energy_pJ       = INIT_SUM;
    summary->system_energy_fJ       = INIT_SUM;
    summary->gpu_energy_J           = INIT_SUM;
    summary->gpu_energy_mJ          = INIT_SUM;
    summary->gpu_energy_uJ          = INIT_SUM;
    summary->gpu_energy_pJ          = INIT_SUM;
    summary->gpu_energy_fJ          = INIT_SUM;
    summary->board_energy_J         = INIT_SUM;
    summary->board_energy_mJ        = INIT_SUM;
    summary->board_energy_uJ        = INIT_SUM;
    summary->board_energy_pJ        = INIT_SUM;
    summary->board_energy_fJ        = INIT_SUM;

#ifdef TRACE_CPU
    summary->allcpu_energy_J        = INIT_SUM;
    summary->allcpu_energy_mJ       = INIT_SUM;
    summary->allcpu_energy_uJ       = INIT_SUM;
    summary->allcpu_energy_pJ       = INIT_SUM;
    summary->allcpu_energy_fJ       = INIT_SUM;
#endif   // TRACE_CPU

#ifdef TRACE_MEM
    summary->psum_emc_util_e2us     = INIT_SUM;
    summary->psum_emc_util_e2fs     = INIT_SUM;
    summary->psum_emc_freq_sec      = INIT_SUM;
    summary->psum_emc_freq_ns       = INIT_SUM;
    summary->mem_energy_J           = INIT_SUM;
    summary->mem_energy_mJ          = INIT_SUM;
    summary->mem_energy_uJ          = INIT_SUM;
    summary->mem_energy_pJ          = INIT_SUM;
    summary->mem_energy_fJ          = INIT_SUM;
#endif   // TRACE_MEM

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   FINISH", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_SUMMARY
    return;
}

static inline void print_system_energy(summary_struct summary) {

    printf("\n%s() in %s:%d   SYSTEM energy  J:     %ld", __func__, __FILE__, __LINE__, summary.system_energy_J);
    printf("\n%s() in %s:%d   SYSTEM energy mJ:     %ld", __func__, __FILE__, __LINE__, summary.system_energy_mJ);
    printf("\n%s() in %s:%d   SYSTEM energy uJ:     %ld", __func__, __FILE__, __LINE__, summary.system_energy_uJ);
    printf("\n%s() in %s:%d   SYSTEM energy pJ:     %ld", __func__, __FILE__, __LINE__, summary.system_energy_pJ);
    printf("\n%s() in %s:%d   SYSTEM energy fJ:     %ld", __func__, __FILE__, __LINE__, summary.system_energy_fJ);

    return;
}

static void update_system_energy(summary_struct *summary, const powerlog_struct *powerlog_ptr) {

    int64_t sec, ns;
    int64_t sum_system_power_mW;
    int64_t avg_system_power_mW, avg_system_power_uW;
    int64_t fraction;

    // Calculate average power
    sum_system_power_mW =   powerlog_ptr->allcpu_power
                          + powerlog_ptr->gpu_power
                          + powerlog_ptr->mem_power
                          + summary->last_powerlog.allcpu_power
                          + summary->last_powerlog.gpu_power
                          + summary->last_powerlog.mem_power;
    avg_system_power_mW = sum_system_power_mW / 2;
    avg_system_power_uW = ((sum_system_power_mW % 2) * MILLI_PER_MICRO) / 2;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   avg.SYSTEM power: %ld.%ld (mW)", __func__, __FILE__, __LINE__, avg_system_power_mW, avg_system_power_uW);
#endif   // DEBUG or DEBUG_SUMMARY

    // Calculate elapsed time in: ms, ns
    sec = powerlog_ptr->timestamp.tv_sec  - summary->finish_timestamp.tv_sec;
    ns  = powerlog_ptr->timestamp.tv_nsec - summary->finish_timestamp.tv_nsec;

    if(ns < 0) {
        --sec;
        ns += ONE_PER_NANO;
    }

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   diff sec:  %ld", __func__, __FILE__, __LINE__, sec);
    printf("\n%s() in %s:%d   diff nsec: %ld", __func__, __FILE__, __LINE__, ns);
#endif   // DEBUG or DEBUG_SUMMARY

    // Sum the calculated energy
    summary->system_energy_mJ += sec * avg_system_power_mW;
    summary->system_energy_uJ += sec * avg_system_power_uW;
    summary->system_energy_pJ += ns  * avg_system_power_mW;
    summary->system_energy_fJ += ns  * avg_system_power_uW;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   SYSTEM energy before removing remainder ---|", __func__, __FILE__, __LINE__);
    print_system_energy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of fJ
    fraction  = summary->system_energy_fJ / PICO_PER_FEMTO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction: %ld", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->system_energy_pJ   += fraction;
    summary->system_energy_fJ   -= (fraction * PICO_PER_FEMTO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   SYSTEM energy after removing fJ remainder ---|", __func__, __FILE__, __LINE__);
    print_system_energy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of pJ
    fraction  = summary->system_energy_pJ / MICRO_PER_PICO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction: %ld", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->system_energy_uJ += fraction;
    summary->system_energy_pJ -= (fraction * MICRO_PER_PICO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   SYSTEM energy after removing pJ remainder ---|", __func__, __FILE__, __LINE__);
    print_system_energy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of uJ
    fraction  = summary->system_energy_uJ / MILLI_PER_MICRO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction: %ld", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->system_energy_mJ += fraction;
    summary->system_energy_uJ -= (fraction * MILLI_PER_MICRO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   SYSTEM energy after removing uJ remainder ---|", __func__, __FILE__, __LINE__);
    print_system_energy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of mJ
    fraction  = summary->system_energy_mJ / ONE_PER_MILLI;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction: %ld", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->system_energy_J  += fraction;
    summary->system_energy_mJ -= (fraction * ONE_PER_MILLI);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   SYSTEM energy after removing mJ remainder ---|", __func__, __FILE__, __LINE__);
    print_system_energy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    return;
}


static inline void print_gpuenergy(summary_struct summary) {

    printf("\n%s() in %s:%d   GPU energy  J:     %ld", __func__, __FILE__, __LINE__, summary.gpu_energy_J);
    printf("\n%s() in %s:%d   GPU energy mJ:     %ld", __func__, __FILE__, __LINE__, summary.gpu_energy_mJ);
    printf("\n%s() in %s:%d   GPU energy uJ:     %ld", __func__, __FILE__, __LINE__, summary.gpu_energy_uJ);
    printf("\n%s() in %s:%d   GPU energy pJ:     %ld", __func__, __FILE__, __LINE__, summary.gpu_energy_pJ);
    printf("\n%s() in %s:%d   GPU energy fJ:     %ld", __func__, __FILE__, __LINE__, summary.gpu_energy_fJ);

    return;
}

static void update_gpuenergy(summary_struct *summary, const powerlog_struct *powerlog_ptr) {

    int64_t sec, ns;
    int64_t avg_gpupower_mW, avg_gpupower_uW;
    int64_t fraction;

    // Calculate average power
    avg_gpupower_mW = (powerlog_ptr->gpu_power + summary->last_powerlog.gpu_power) / 2;
    avg_gpupower_uW = (((powerlog_ptr->gpu_power + summary->last_powerlog.gpu_power) % 2) * MILLI_PER_MICRO) / 2;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   given GPU power: %d (mW)", __func__, __FILE__, __LINE__, powerlog_ptr->gpu_power);
    printf("\n%s() in %s:%d   last GPU power: %d (mW)", __func__, __FILE__, __LINE__, summary->last_powerlog.gpu_power);
    printf("\n%s() in %s:%d   avg.GPU power: %ld.%ld (mW)", __func__, __FILE__, __LINE__, avg_gpupower_mW, avg_gpupower_uW);
#endif   // DEBUG or DEBUG_SUMMARY

    // Calculate elapsed time in: ms, ns
    sec = powerlog_ptr->timestamp.tv_sec  - summary->finish_timestamp.tv_sec;
    ns  = powerlog_ptr->timestamp.tv_nsec - summary->finish_timestamp.tv_nsec;

    if(ns < 0) {
        --sec;
        ns += ONE_PER_NANO;
    }

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   diff sec:  %ld", __func__, __FILE__, __LINE__, sec);
    printf("\n%s() in %s:%d   diff nsec: %ld", __func__, __FILE__, __LINE__, ns);
#endif   // DEBUG or DEBUG_SUMMARY

    // Sum the calculated energy
    summary->gpu_energy_mJ += sec * avg_gpupower_mW;
    summary->gpu_energy_uJ += sec * avg_gpupower_uW;
    summary->gpu_energy_pJ += ns  * avg_gpupower_mW;
    summary->gpu_energy_fJ += ns  * avg_gpupower_uW;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   GPU energy before removing remainder ---|", __func__, __FILE__, __LINE__);
    print_gpuenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of fJ
    fraction  = summary->gpu_energy_fJ / PICO_PER_FEMTO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction: %ld", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->gpu_energy_pJ   += fraction;
    summary->gpu_energy_fJ   -= (fraction * PICO_PER_FEMTO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   GPU energy after removing fJ remainder ---|", __func__, __FILE__, __LINE__);
    print_gpuenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of pJ
    fraction  = summary->gpu_energy_pJ / MICRO_PER_PICO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction: %ld", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->gpu_energy_uJ += fraction;
    summary->gpu_energy_pJ -= (fraction * MICRO_PER_PICO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   GPU energy after removing pJ remainder ---|", __func__, __FILE__, __LINE__);
    print_gpuenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of uJ
    fraction  = summary->gpu_energy_uJ / MILLI_PER_MICRO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction: %ld", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->gpu_energy_mJ += fraction;
    summary->gpu_energy_uJ -= (fraction * MILLI_PER_MICRO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   GPU energy after removing uJ remainder ---|", __func__, __FILE__, __LINE__);
    print_gpuenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of mJ
    fraction  = summary->gpu_energy_mJ / ONE_PER_MILLI;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction: %ld", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->gpu_energy_J  += fraction;
    summary->gpu_energy_mJ -= (fraction * ONE_PER_MILLI);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   GPU energy after removing mJ remainder ---|", __func__, __FILE__, __LINE__);
    print_gpuenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    return;
}

static void update_psum_gpuutil(summary_struct *summary, const powerlog_struct *powerlog_ptr) {

    int64_t sec, ns;
    int64_t fraction;

    // Calculate elapsed time in: sec, ns
    sec = powerlog_ptr->timestamp.tv_sec  - summary->finish_timestamp.tv_sec;
    ns  = powerlog_ptr->timestamp.tv_nsec - summary->finish_timestamp.tv_nsec;

    if(ns < 0) {
        --sec;
        ns += ONE_PER_NANO;
    }

    summary->psum_gpu_util_e2ms += (powerlog_ptr->gpu_util * sec);
    summary->psum_gpu_util_e2ps += (powerlog_ptr->gpu_util * ns);

    fraction = summary->psum_gpu_util_e2ps / ONE_PER_NANO;
    if(fraction > 0) {
        summary->psum_gpu_util_e2ms += fraction;
        summary->psum_gpu_util_e2ps -= (fraction * ONE_PER_NANO);
    }

    return;
}

static void update_psum_emcutil(summary_struct *summary, const powerlog_struct *powerlog_ptr) {

    int64_t sec, ns;
    int64_t fraction;

    // Calculate elapsed time in: sec, ns
    sec = powerlog_ptr->timestamp.tv_sec  - summary->finish_timestamp.tv_sec;
    ns  = powerlog_ptr->timestamp.tv_nsec - summary->finish_timestamp.tv_nsec;

    if(ns < 0) {
        --sec;
        ns += ONE_PER_NANO;
    }

    summary->psum_emc_util_e2us += (powerlog_ptr->emc_util * sec);
    summary->psum_emc_util_e2fs += (powerlog_ptr->emc_util * ns);

    fraction = summary->psum_emc_util_e2fs / ONE_PER_NANO;
    if(fraction > 0) {
        summary->psum_emc_util_e2us += fraction;
        summary->psum_emc_util_e2fs -= (fraction * ONE_PER_NANO);
    }

    return;
}

static inline void print_boardenergy(summary_struct summary) {

    printf("\n%s() in %s:%d   BOARD energy  J:     %ld", __func__, __FILE__, __LINE__, summary.board_energy_J);
    printf("\n%s() in %s:%d   BOARD energy mJ:     %ld", __func__, __FILE__, __LINE__, summary.board_energy_mJ);
    printf("\n%s() in %s:%d   BOARD energy uJ:     %ld", __func__, __FILE__, __LINE__, summary.board_energy_uJ);
    printf("\n%s() in %s:%d   BOARD energy pJ:     %ld", __func__, __FILE__, __LINE__, summary.board_energy_pJ);
    printf("\n%s() in %s:%d   BOARD energy fJ:     %ld", __func__, __FILE__, __LINE__, summary.board_energy_fJ);

    return;
}

static void update_boardenergy(summary_struct *summary, const powerlog_struct *powerlog_ptr) {

    int64_t sec, ns;
    int64_t avg_boardpower_mW, avg_boardpower_uW;
    int64_t fraction;

    // Calculate average power
    avg_boardpower_mW = (powerlog_ptr->board_power + summary->last_powerlog.board_power) / 2;
    avg_boardpower_uW = (((powerlog_ptr->board_power + summary->last_powerlog.board_power) % 2) * MILLI_PER_MICRO) / 2;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   given BOARD power: %d (mW)", __func__, __FILE__, __LINE__, powerlog_ptr->board_power);
    printf("\n%s() in %s:%d   last BOARD power: %d (mW)", __func__, __FILE__, __LINE__, summary->last_powerlog.board_power);
    printf("\n%s() in %s:%d   avg.BOARD power: %ld.%ld (mW)", __func__, __FILE__, __LINE__, avg_boardpower_mW, avg_boardpower_uW);
#endif   // DEBUG or DEBUG_SUMMARY

    // Calculate elapsed time in: sec, ns
    sec = powerlog_ptr->timestamp.tv_sec  - summary->finish_timestamp.tv_sec;
    ns  = powerlog_ptr->timestamp.tv_nsec - summary->finish_timestamp.tv_nsec;
    if(ns < 0) {
        --sec;
        ns += ONE_PER_NANO;
    }

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   diff sec:  %ld", __func__, __FILE__, __LINE__, sec);
    printf("\n%s() in %s:%d   diff nsec: %ld", __func__, __FILE__, __LINE__, ns);
#endif   // DEBUG or DEBUG_SUMMARY

    // Sum the calculated energy
    summary->board_energy_mJ += sec * avg_boardpower_mW;
    summary->board_energy_uJ += sec * avg_boardpower_uW;
    summary->board_energy_pJ += ns  * avg_boardpower_mW;
    summary->board_energy_fJ += ns  * avg_boardpower_uW;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   BOARD energy before removing remainder ---|", __func__, __FILE__, __LINE__);
    print_boardenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of fJ
    fraction  = summary->board_energy_fJ / PICO_PER_FEMTO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction: %ld", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->board_energy_pJ   += fraction;
    summary->board_energy_fJ   -= (fraction * PICO_PER_FEMTO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   BOARD energy after removing fJ remainder ---|", __func__, __FILE__, __LINE__);
    print_boardenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of pJ
    fraction  = summary->board_energy_pJ / MICRO_PER_PICO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction: %ld", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->board_energy_uJ += fraction;
    summary->board_energy_pJ -= (fraction * MICRO_PER_PICO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   BOARD energy after removing pJ remainder ---|", __func__, __FILE__, __LINE__);
    print_boardenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of uJ
    fraction  = summary->board_energy_uJ / MILLI_PER_MICRO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction: %ld", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->board_energy_mJ += fraction;
    summary->board_energy_uJ -= (fraction * MILLI_PER_MICRO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   BOARD energy after removing uJ remainder ---|", __func__, __FILE__, __LINE__);
    print_boardenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of mJ
    fraction  = summary->board_energy_mJ / ONE_PER_MILLI;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction: %ld", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->board_energy_J  += fraction;
    summary->board_energy_mJ -= (fraction * ONE_PER_MILLI);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   BOARD energy after removing mJ remainder ---|", __func__, __FILE__, __LINE__);
    print_boardenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    return;
}

#ifdef TRACE_MEM
static inline void print_memenergy(summary_struct summary) {

    printf("\n%s() in %s:%d   MEM energy  J:     %ld", __func__, __FILE__, __LINE__, summary.mem_energy_J);
    printf("\n%s() in %s:%d   MEM energy mJ:     %ld", __func__, __FILE__, __LINE__, summary.mem_energy_mJ);
    printf("\n%s() in %s:%d   MEM energy uJ:     %ld", __func__, __FILE__, __LINE__, summary.mem_energy_uJ);
    printf("\n%s() in %s:%d   MEM energy pJ:     %ld", __func__, __FILE__, __LINE__, summary.mem_energy_pJ);
    printf("\n%s() in %s:%d   MEM energy fJ:     %ld", __func__, __FILE__, __LINE__, summary.mem_energy_fJ);

    return;
}

static void update_memenergy(summary_struct *summary, const powerlog_struct *powerlog_ptr) {

    int64_t sec, ns;
    int64_t avg_mempower_mW, avg_mempower_uW;
    int64_t fraction;

    // Calculate average power
    avg_mempower_mW = (powerlog_ptr->mem_power + summary->last_powerlog.mem_power) / 2;
    avg_mempower_uW = (((powerlog_ptr->mem_power + summary->last_powerlog.mem_power) % 2) * MILLI_PER_MICRO) / 2;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   given MEM power: %d (mW)", __func__, __FILE__, __LINE__, powerlog_ptr->mem_power);
    printf("\n%s() in %s:%d   last MEM power: %d (mW)", __func__, __FILE__, __LINE__, summary->last_powerlog.mem_power);
    printf("\n%s() in %s:%d   avg.MEM power: %ld.%ld (mW)", __func__, __FILE__, __LINE__, avg_mempower_mW, avg_mempower_uW);
#endif   // DEBUG or DEBUG_SUMMARY

    // Calculate elapsed time in: sec, ns
    sec = powerlog_ptr->timestamp.tv_sec  - summary->finish_timestamp.tv_sec;
    ns  = powerlog_ptr->timestamp.tv_nsec - summary->finish_timestamp.tv_nsec;
    if(ns < 0) {
        --sec;
        ns += ONE_PER_NANO;
    }

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   diff sec:  %ld", __func__, __FILE__, __LINE__, sec);
    printf("\n%s() in %s:%d   diff nsec: %ld", __func__, __FILE__, __LINE__, ns);
#endif   // DEBUG or DEBUG_SUMMARY

    // Sum the calculated energy
    summary->mem_energy_mJ += sec * avg_mempower_mW;
    summary->mem_energy_uJ += sec * avg_mempower_uW;
    summary->mem_energy_pJ += ns  * avg_mempower_mW;
    summary->mem_energy_fJ += ns  * avg_mempower_uW;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   MEM energy before removing remainder ---|", __func__, __FILE__, __LINE__);
    print_memenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of fJ
    fraction  = summary->mem_energy_fJ / PICO_PER_FEMTO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction: %ld", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->mem_energy_pJ   += fraction;
    summary->mem_energy_fJ   -= (fraction * PICO_PER_FEMTO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   MEM energy after removing fJ remainder ---|", __func__, __FILE__, __LINE__);
    print_memenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of pJ
    fraction  = summary->mem_energy_pJ / MICRO_PER_PICO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction: %ld", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->mem_energy_uJ += fraction;
    summary->mem_energy_pJ -= (fraction * MICRO_PER_PICO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   MEM energy after removing pJ remainder ---|", __func__, __FILE__, __LINE__);
    print_memenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of uJ
    fraction  = summary->mem_energy_uJ / MILLI_PER_MICRO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction: %ld", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->mem_energy_mJ += fraction;
    summary->mem_energy_uJ -= (fraction * MILLI_PER_MICRO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   MEM energy after removing uJ remainder ---|", __func__, __FILE__, __LINE__);
    print_memenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of mJ
    fraction  = summary->mem_energy_mJ / ONE_PER_MILLI;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction: %ld", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->mem_energy_J  += fraction;
    summary->mem_energy_mJ -= (fraction * ONE_PER_MILLI);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   MEM energy after removing mJ remainder ---|", __func__, __FILE__, __LINE__);
    print_memenergy(*summary);
#endif   // DEBUG or DEBUG_SUMMARY

    return;
}
#endif   // TRACE_MEM

void update_summary(summary_struct *summary, const powerlog_struct *powerlog_ptr) {

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_SUMMARY

    if(!summary->num_powerlog) {
        summary->start_timestamp = powerlog_ptr->timestamp;
        summary->finish_timestamp = powerlog_ptr->timestamp;
        summary->last_powerlog = *powerlog_ptr;
    }

    // Update maximum values
    if(summary->max_gpu_util < powerlog_ptr->gpu_util)
        summary->max_gpu_util = powerlog_ptr->gpu_util;
    if(summary->max_gpu_freq < powerlog_ptr->gpu_freq)
        summary->max_gpu_freq = powerlog_ptr->gpu_freq;
    if(summary->max_gpu_power < powerlog_ptr->gpu_power)
        summary->max_gpu_power = powerlog_ptr->gpu_power;

    // Update minimum values.
    // Note that we initialized minimum values to maximum possitive number.
    // Thus, correct values are always less than initialized minimum values
    if(summary->min_gpu_util > powerlog_ptr->gpu_util)
        summary->min_gpu_util = powerlog_ptr->gpu_util;
    if(summary->min_gpu_freq > powerlog_ptr->gpu_freq)
        summary->min_gpu_freq = powerlog_ptr->gpu_freq;
    if(summary->min_gpu_power > powerlog_ptr->gpu_power)
        summary->min_gpu_power = powerlog_ptr->gpu_power;

    update_system_energy(summary, powerlog_ptr);
    update_gpuenergy(summary, powerlog_ptr);
    update_psum_gpuutil(summary, powerlog_ptr);
    update_psum_emcutil(summary, powerlog_ptr);
    update_boardenergy(summary, powerlog_ptr);
#ifdef TRACE_MEM
    update_memenergy(summary, powerlog_ptr);
#endif   // TRACE_MEM

    // Count number of powerlogs
    ++(summary->num_powerlog);

    // Store timestamp
    summary->finish_timestamp = powerlog_ptr->timestamp;

    // AT LAST, Copy the contents of the last powerlog
    summary->last_powerlog = *powerlog_ptr;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   FINISH", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_SUMMARY
    return;
}

void print_summary(int fd, const summary_struct *summary) {

    char buff[MAX_BUFFLEN];
    size_t buff_len;

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n--- summary ---");
    write(fd, buff, buff_len);

    // Address of the summary struct
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - address: %p", (void *)summary);
    write(fd, buff, buff_len);

    // First and last timestamp
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - @start_timestamp: %ld.%ld\n - @finish_timestamp: %ld.%ld", summary->start_timestamp.tv_sec, summary->start_timestamp.tv_nsec, summary->finish_timestamp.tv_sec, summary->finish_timestamp.tv_nsec);
    write(fd, buff, buff_len);

    // Number of powerlogs
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - @num_powerlog: %d", summary->num_powerlog);
    write(fd, buff, buff_len);

    //
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - @min_gpu_util: %d", summary->min_gpu_util);
    write(fd, buff, buff_len);
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - @max_gpu_util: %d", summary->max_gpu_util);
    write(fd, buff, buff_len);
}

struct timespec elapsed_time(const summary_struct summary) {

    struct timespec ret;
    time_t diff_sec;
    int64_t diff_nsec;

    diff_sec  = summary.finish_timestamp.tv_sec  - summary.start_timestamp.tv_sec;
    diff_nsec = summary.finish_timestamp.tv_nsec - summary.start_timestamp.tv_nsec;

    if(diff_nsec < 0) {
        --diff_sec;
        diff_nsec += ONE_PER_NANO;
    }

    ret.tv_sec  = diff_sec;
    ret.tv_nsec = diff_nsec;

    return ret;
}


/* Print functions */
void print_summary_name(int fd, const summary_struct *summary) {

    char buff[MAX_BUFFLEN];
    ssize_t buff_len;

    write(fd, "\n\n", 2);

    buff_len = snprintf(buff, MAX_BUFFLEN, summary->name);
    write(fd, buff, buff_len);
}

void print_summary_runtime(int fd, const summary_struct *summary) {

    char buff[MAX_BUFFLEN];
    ssize_t buff_len;

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n   * Runtime: %*ld.%09ld seconds", 9, elapsed_time(*summary).tv_sec, elapsed_time(*summary).tv_nsec);
    write(fd, buff, buff_len);
}

void print_summary_gpu_util_range(int fd, const summary_struct *summary) {

    char buff[MAX_BUFFLEN];
    ssize_t buff_len;

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n   * GPU-utilization range: %*d.%*d %c - %*d.%*d %c", (TX2_SYSFS_GPU_UTIL_MAX_STRLEN - 1), (summary->min_gpu_util / 10), 1, (summary->min_gpu_util % 10), '%', (TX2_SYSFS_GPU_UTIL_MAX_STRLEN - 1), (summary->max_gpu_util / 10), 1, (summary->max_gpu_util % 10), '%');
    write(fd, buff, buff_len);
}

void print_summary_emc_util_range(int fd, const summary_struct *summary) {
#ifdef TRACE_MEM

    char buff[MAX_BUFFLEN];
    ssize_t buff_len;

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n   * EMC-utilization range: %*d.%0*d %c - %*d.%0*d %c", (TX2_SYSFS_EMC_UTIL_MAX_STRLEN - 4), (summary->min_emc_util / 10000), 4, (summary->min_emc_util % 10000), '%', (TX2_SYSFS_GPU_UTIL_MAX_STRLEN - 4), (summary->max_emc_util / 10000), 4, (summary->max_emc_util % 10000), '%');
    write(fd, buff, buff_len);

#endif   // TRACE_MEM
}

void print_summary_gpu_freq_range(int fd, const summary_struct *summary) {

    char buff[MAX_BUFFLEN];
    ssize_t buff_len;

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n   * GPU-frequency range:   %*d MHz - %*d MHz", TX2_SYSFS_GPU_MHZFREQ_MAX_STRLEN, summary->min_gpu_freq, TX2_SYSFS_GPU_MHZFREQ_MAX_STRLEN, summary->max_gpu_freq);
    write(fd, buff, buff_len);
}

void print_summary_gpu_power_range(int fd, const summary_struct *summary) {

    char buff[MAX_BUFFLEN];
    ssize_t buff_len;

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n   * GPU-power range:       %*d mW - %*d mW", TX2_SYSFS_GPU_POWER_MAX_STRLEN, summary->min_gpu_power, TX2_SYSFS_GPU_UTIL_MAX_STRLEN, summary->max_gpu_power);
    write(fd, buff, buff_len);
}


