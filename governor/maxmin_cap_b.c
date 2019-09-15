#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "governor.h"
#include "../default_values.h"

static void maxmin_cap_b_init(void *data);
static int32_t maxmin_cap_b_get_target_freq();
static void maxmin_cap_b_print_gpugov(int fd);

// Interface
struct gpugov maxmin_cap_b = {

    .name = "gpugov-maxmin_cap_b",
    .init = maxmin_cap_b_init,
    .get_target_freq = maxmin_cap_b_get_target_freq,
    .print_gpugov = maxmin_cap_b_print_gpugov
};

/**
 *  Governor parameters
 */
static int gpu_up_threshold          = 800;      // 80% (unit: x0.1%)
static int gpu_down_threshold        = 700;      // 70% (unit: x0.1%)
static int emc_maxcap_threshold      = 200000;   // 20% (unit: x0.0001%)
static int emc_mincap_threshold      = 200000;   // 20% (unit: x0.0001%)
static int scale_up_factor           = 9;        // 9% (unit: %)
static int scale_down_factor         = 9;        // 9% (unit: %)

static void maxmin_cap_b_init(void *data) {
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    // Do nothing

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   FINISHED", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR
}

static int32_t cap_max_freq(int32_t next_freq, int64_t emcutil) {

    int32_t capped_max_freq;

    // Decide capped maximum frequency
    if(emcutil > emc_maxcap_threshold)
        capped_max_freq = gpugov_info.available_gpufreq[10];
    else
        capped_max_freq = gpugov_info.available_gpufreq[12];

    // Apply capped maximum frequency
    if(next_freq > capped_max_freq)
        return capped_max_freq;
    else
        return next_freq;
}

static int32_t cap_min_freq(int32_t next_freq, int64_t emcutil) {

    int32_t capped_min_freq;

    // Decide capped minimum frequency
    if(emcutil < emc_mincap_threshold)
        capped_min_freq = gpugov_info.available_gpufreq[2];
    else
        capped_min_freq = gpugov_info.available_gpufreq[0];

    // Apply capped minimum frequency
    if(next_freq < capped_min_freq)
        return capped_min_freq;
    else
        return next_freq;
}

static int32_t maxmin_cap_b_get_target_freq() {

    int64_t emcutil = get_emcutil();
    int16_t gpuutil = get_gpuutil();
    int32_t freq = get_gpufreq();
    int32_t next_freq;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
    printf("\n___\n%s() in %s:%d   GPU util: %d", __func__, __FILE__, __LINE__, gpuutil);
    printf("\n___\n%s() in %s:%d   EMC util: %d", __func__, __FILE__, __LINE__, emcutil);
#endif   // DEBUG or DEBUG_GOVERNOR

    if(gpuutil > gpu_up_threshold) {
        next_freq = scale_up_by_n_of_max(freq, scale_up_factor);
        next_freq = cap_max_freq(next_freq, emcutil);
    }
    else if(gpuutil < gpu_down_threshold) {
        next_freq = scale_down_by_n_of_max(freq, scale_down_factor);
        next_freq = cap_min_freq(next_freq, emcutil);
    }
    else
        next_freq = freq;

    set_gpufreq(next_freq);

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   RETURNED: %d", __func__, __FILE__, __LINE__, next_freq);
#endif   // DEBUG or DEBUG_GOVERNOR
    return next_freq;
}

static void maxmin_cap_b_print_gpugov(int fd) {

    char buff[MAX_BUFFLEN];
    size_t buff_len;

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n\nGOVERNOR INFORMATION");
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - name: %s", maxmin_cap_b.name);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - GPU UP THRESHOLD: %d.%d", (gpu_up_threshold / 10), (gpu_up_threshold % 10));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - GPU DOWN THRESHOLD: %d.%d", (gpu_down_threshold / 10), (gpu_down_threshold % 10));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - EMC MAX-CAP THRESHOLD: %d.%d", (emc_maxcap_threshold / 10000), (emc_maxcap_threshold % 10000));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - EMC MIN-CAP THRESHOLD: %d.%d", (emc_mincap_threshold / 10000), (emc_mincap_threshold % 10000));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SCALE UP FACTOR: %d%", scale_up_factor);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SCALE DOWN FACTOR: %d%", scale_down_factor);
    write(fd, buff, buff_len);
}
