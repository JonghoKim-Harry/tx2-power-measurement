#include "constants.h"
#include "summary.h"

void init_summary(powerlog_summary_struct *summary) {

    summary->last_powerlog = NULL;
    summary->num_powerlog = 0;

    summary->gpu_energy_mJ = 0;
    summary->gpu_energy_pJ = 0;

    return;
}

void update_summary(powerlog_summary_struct *summary, const powerlog_struct *powerlog_ptr) {

    int32_t sec, nsec;
    double avg_gpupower;

    // Timestamp
    summary->finish_timestamp = powerlog_ptr->timestamp;
    if(!summary->num_powerlog) {
        summary->start_timestamp = powerlog_ptr->timestamp;
        summary->last_powerlog = powerlog_ptr;
    }

    // Calculate energy
    avg_gpupower = (powerlog_ptr->gpu_power + summary->last_powerlog->gpu_power) / 2.0;

    sec = summary->finish_timestamp.tv_sec - summary->start_timestamp.tv_sec;
    nsec = summary->finish_timestamp.tv_nsec - summary->start_timestamp.tv_nsec;
    summary->gpu_energy_mJ +=  sec * avg_gpupower;
    summary->gpu_energy_pJ += nsec * avg_gpupower;

    while(summary->gpu_energy_pJ < 0) {
        --(summary->gpu_energy_mJ);
        summary->gpu_energy_pJ += MILLI_PER_PICO;
    }

    // Count
    ++(summary->num_powerlog);

    // AT LAST, Store the pointer to last powerlog
    summary->last_powerlog = powerlog_ptr;

    return;
}
