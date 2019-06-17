#include <stdio.h>
#include <stdint.h>
#include "constants.h"
#include "summary.h"

void init_summary(summary_struct *summary) {

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->num_powerlog = 0;

    // Maximum values are simply initialized to -1
    summary->max_gpu_util  = -1;
    summary->max_gpu_freq  = -1;
    summary->max_gpu_power = -1;

    // Minimum values are initialized to maximum possitive numbers
    summary->min_gpu_util  = INT16_MAX;
    summary->min_gpu_freq  = INT16_MAX;
    summary->min_gpu_power = INT16_MAX;

    summary->gpu_energy_J          = 0;
    summary->gpu_energy_uJ         = 0;
    summary->gpu_energy_pJ         = 0;
    summary->gpu_energy_dotone_pJ  = 0;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   FINISH", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_SUMMARY
    return;
}

void update_summary(summary_struct *summary, const powerlog_struct *powerlog_ptr) {

    int64_t sec, ms, ns;
    int64_t avg_gpupower_mW, avg_gpupower_dotone_mW;
    int64_t fraction, remainder;

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

    // Calculate average power
    avg_gpupower_mW = (powerlog_ptr->gpu_power + summary->last_powerlog.gpu_power) / 2;
    avg_gpupower_dotone_mW = ((powerlog_ptr->gpu_power + summary->last_powerlog.gpu_power) % 2) * 5;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   given GPU power: %d (mW)", __func__, __FILE__, __LINE__, powerlog_ptr->gpu_power);
    printf("\n%s() in %s:%d   last GPU power: %d (mW)", __func__, __FILE__, __LINE__, summary->last_powerlog.gpu_power);
    printf("\n%s() in %s:%d   avg.GPU power: %d.%d (mW)", __func__, __FILE__, __LINE__, avg_gpupower_mW, avg_gpupower_dotone_mW);
#endif   // DEBUG or DEBUG_SUMMARY

    // Calculate elapsed time in: ms, ns
    sec = powerlog_ptr->timestamp.tv_sec  - summary->finish_timestamp.tv_sec;
    ns  = powerlog_ptr->timestamp.tv_nsec - summary->finish_timestamp.tv_nsec;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   diff sec:  %d", __func__, __FILE__, __LINE__, sec);
    printf("\n%s() in %s:%d   diff nsec: %d", __func__, __FILE__, __LINE__, ns);
#endif   // DEBUG or DEBUG_SUMMARY

    // Note that negative numbers are also OK
    fraction  = ns / MILLI_PER_NANO;
    remainder = ns % MILLI_PER_NANO;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction:  %d", __func__, __FILE__, __LINE__, fraction);
    printf("\n%s() in %s:%d   remainder: %d", __func__, __FILE__, __LINE__, remainder);
#endif   // DEBUG or DEBUG_SUMMARY

    ms = sec * ONE_PER_MILLI + fraction;
    ns = remainder;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   elapsed ms: %d", __func__, __FILE__, __LINE__, ms);
    printf("\n%s() in %s:%d   elapsed ns: %d", __func__, __FILE__, __LINE__, ns);
#endif   // DEBUG or DEBUG_SUMMARY

    // Sum the calculated energy
    summary->gpu_energy_uJ += ms * avg_gpupower_mW;
    summary->gpu_energy_pJ += ns * avg_gpupower_mW;
    summary->gpu_energy_dotone_pJ += ms * avg_gpupower_dotone_mW * MICRO_PER_PICO;
    summary->gpu_energy_dotone_pJ += ns * avg_gpupower_dotone_mW;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   GPU energy before removing remainder ---|", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   GPU energy uJ:     %ld", __func__, __FILE__, __LINE__, summary->gpu_energy_uJ);
    printf("\n%s() in %s:%d   GPU energy pJ:     %ld.%ld", __func__, __FILE__, __LINE__, summary->gpu_energy_pJ, summary->gpu_energy_dotone_pJ);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of 0.1 pJ
    fraction  = summary->gpu_energy_dotone_pJ / 10;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction:  %d", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->gpu_energy_pJ         += fraction;
    summary->gpu_energy_dotone_pJ  -= (fraction * 10);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   GPU energy after removing 0.1pJ remainder ---|", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   GPU energy uJ:     %ld", __func__, __FILE__, __LINE__, summary->gpu_energy_uJ);
    printf("\n%s() in %s:%d   GPU energy pJ:     %ld.%ld", __func__, __FILE__, __LINE__, summary->gpu_energy_pJ, summary->gpu_energy_dotone_pJ);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder of pJ
    fraction  = summary->gpu_energy_pJ / MICRO_PER_PICO;
    if(fraction < 0) --fraction;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   fraction:  %d", __func__, __FILE__, __LINE__, fraction);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->gpu_energy_uJ += fraction;
    summary->gpu_energy_pJ -= (fraction * MICRO_PER_PICO);

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   GPU energy after removing pJ remainder ---|", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   GPU energy uJ:     %ld", __func__, __FILE__, __LINE__, summary->gpu_energy_uJ);
    printf("\n%s() in %s:%d   GPU energy pJ:     %ld.%ld", __func__, __FILE__, __LINE__, summary->gpu_energy_pJ, summary->gpu_energy_dotone_pJ);
#endif   // DEBUG or DEBUG_SUMMARY

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
