#include <stdio.h>
#include "constants.h"
#include "summary.h"

void init_summary(powerlog_summary_struct *summary) {

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_SUMMARY

    summary->last_powerlog = NULL;
    summary->num_powerlog = 0;

    summary->gpu_energy_mJ = 0;
    summary->gpu_energy_pJ = 0;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   FINISH", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_SUMMARY
    return;
}

void update_summary(powerlog_summary_struct *summary, const powerlog_struct *powerlog_ptr) {

    int32_t sec, nsec;
    double avg_gpupower;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_SUMMARY

    if(!summary->num_powerlog) {
        summary->start_timestamp = powerlog_ptr->timestamp;
        summary->finish_timestamp = powerlog_ptr->timestamp;
        summary->last_powerlog = powerlog_ptr;
    }

    // Calculate average power
    avg_gpupower = (powerlog_ptr->gpu_power + summary->last_powerlog->gpu_power) / 2.0;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   avg.GPU power: %lf (mW)", __func__, __FILE__, __LINE__, avg_gpupower);
#endif   // DEBUG or DEBUG_SUMMARY

    // Calculate elapsed time
    sec = powerlog_ptr->timestamp.tv_sec - summary->finish_timestamp.tv_sec;
    nsec = powerlog_ptr->timestamp.tv_nsec - summary->finish_timestamp.tv_nsec;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   elapsed sec:  %d", __func__, __FILE__, __LINE__, sec);
    printf("\n%s() in %s:%d   elapsed nsec: %d", __func__, __FILE__, __LINE__, nsec);
#endif   // DEBUG or DEBUG_SUMMARY

    // Sum the calculated energy
    summary->gpu_energy_mJ +=  sec * avg_gpupower;
    summary->gpu_energy_pJ += nsec * avg_gpupower;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   GPU energy before removing remainder ---|", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   GPU energy mJ:  %lf", __func__, __FILE__, __LINE__, summary->gpu_energy_mJ);
    printf("\n%s() in %s:%d   GPU energy pJ:  %lf", __func__, __FILE__, __LINE__, summary->gpu_energy_pJ);
#endif   // DEBUG or DEBUG_SUMMARY

    // Remove remainder
    while(summary->gpu_energy_pJ < 0) {
        --(summary->gpu_energy_mJ);
        summary->gpu_energy_pJ += MILLI_PER_PICO;
    }

    while(summary->gpu_energy_pJ > MILLI_PER_PICO) {
        ++(summary->gpu_energy_mJ);
        summary->gpu_energy_pJ -= MILLI_PER_PICO;
    }

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   GPU energy after removing remainder ---|", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   GPU energy mJ:  %lf", __func__, __FILE__, __LINE__, summary->gpu_energy_mJ);
    printf("\n%s() in %s:%d   GPU energy pJ:  %lf", __func__, __FILE__, __LINE__, summary->gpu_energy_pJ);
#endif   // DEBUG or DEBUG_SUMMARY

    // Count number of powerlogs
    ++(summary->num_powerlog);

    // Store timestamp
    summary->finish_timestamp = powerlog_ptr->timestamp;

    // AT LAST, Store the pointer to last powerlog
    summary->last_powerlog = powerlog_ptr;

#if defined(DEBUG) || defined(DEBUG_SUMMARY)
    printf("\n%s() in %s:%d   FINISH", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_SUMMARY
    return;
}
