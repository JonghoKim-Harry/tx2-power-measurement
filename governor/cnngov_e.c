#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "governor.h"
#include "../default_values.h"

static void cnngov_e_init(void *data);
static int32_t cnngov_e_get_target_freq();
static void cnngov_e_print_gpugov(int fd);

// Interface
struct gpugov cnngov_e = {

    .name = "gpugov-cnngov_e",
    .init = cnngov_e_init,
    .get_target_freq = cnngov_e_get_target_freq,
    .print_gpugov = cnngov_e_print_gpugov
};

/**
 *  Governor parameters
 */
static int gpu_up_threshold          = 800;      // 80% (unit: x0.1%)
static int gpu_down_threshold        = 700;      // 70% (unit: x0.1%)
static int emc_up_threshold          = 200000;   // 20% (unit: x0.0001%)
static int scale_up_factor           = 9;        // 9% (unit: %)
static int scale_down_factor         = 9;        // 9% (unit: %)

static void cnngov_e_init(void *data) {
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    // Do nothing

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   FINISHED", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR
}

static int32_t cap_min_freq(int32_t next_freq, int64_t emcutil, int16_t gpuutil) {

    int32_t capped_min_freq;

    // Decide capped minimum frequency
    capped_min_freq = gpugov_info.available_gpufreq[4];

    // Apply capped minimum frequency
    if(next_freq < capped_min_freq)
        return capped_min_freq;
    else
        return next_freq;
}

static int32_t cnngov_e_get_target_freq() {

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
        if(emcutil < emc_up_threshold)
            next_freq = scale_up_by_n_of_max(freq, scale_up_factor);
        else
            next_freq = freq;
    }
    else if(gpuutil < gpu_down_threshold) {
        next_freq = scale_down_by_n_of_max(freq, scale_down_factor);
        next_freq = cap_min_freq(next_freq, emcutil, gpuutil);
    }
    else
        next_freq = freq;

    set_gpufreq(next_freq);

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   RETURNED: %d", __func__, __FILE__, __LINE__, next_freq);
#endif   // DEBUG or DEBUG_GOVERNOR
    return next_freq;
}

static void cnngov_e_print_gpugov(int fd) {

    char buff[MAX_BUFFLEN];
    size_t buff_len;

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n\nGOVERNOR INFORMATION");
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - name: %s", cnngov_e.name);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - GPU UP THRESHOLD: %d.%d", (gpu_up_threshold / 10), (gpu_up_threshold % 10));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - GPU DOWN THRESHOLD: %d.%d", (gpu_down_threshold / 10), (gpu_down_threshold % 10));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - EMC UP THRESHOLD: %d.%d", (emc_up_threshold / 10000), (emc_up_threshold % 10000));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SCALE UP FACTOR: %d%", scale_up_factor);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SCALE DOWN FACTOR: %d%", scale_down_factor);
    write(fd, buff, buff_len);
}
