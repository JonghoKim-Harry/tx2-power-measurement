#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "governor.h"
#include "../default_values.h"

static void cnngov_20191211_init(void *data);
static int32_t cnngov_20191211_get_target_freq();
static void cnngov_20191211_print_gpugov(int fd);

// Interface
struct gpugov cnngov_20191211 = {

    .name = "gpugov-cnngov_20191211",
    .init = cnngov_20191211_init,
    .get_target_freq = cnngov_20191211_get_target_freq,
    .print_gpugov = cnngov_20191211_print_gpugov
};

/**
 *  Governor parameters
 */
static int TH1                        = 996;      // 99.6% (unit: x0.1%)

static int scale_up_factor            = 9;        // 9% (unit: %)
static int scale_down_factor          = 9;        // 9% (unit: %)
static const int SAMPLING_DOWN_FACTOR = 3;
static const int SAMPLING_UP_FACTOR   = 3;
static int sampling_down_counter      = 0;
static int sampling_up_counter        = 0;

static int32_t min_freq, max_freq;

static void cnngov_20191211_init(void *data) {
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    sampling_down_counter = 0;
    sampling_up_counter = 0;

    min_freq = gpugov_info.available_gpufreq[6];
    max_freq = gpugov_info.available_gpufreq[10];
    //max_freq = gpugov_info.available_gpufreq[12];

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   FINISHED", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR
}

static int32_t cnngov_20191211_get_target_freq() {

    int64_t emcutil = get_emcutil();
    int16_t gpuutil = get_gpuutil();
    int32_t freq = get_gpufreq();
    int32_t next_freq;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
    printf("\n___\n%s() in %s:%d   GPU util: %d", __func__, __FILE__, __LINE__, gpuutil);
    printf("\n___\n%s() in %s:%d   EMC util: %d", __func__, __FILE__, __LINE__, emcutil);
#endif   // DEBUG or DEBUG_GOVERNOR

    ++sampling_down_counter;
    ++sampling_up_counter;

    next_freq = freq;

    if(gpuutil >= TH1) {
        if(sampling_up_counter >= SAMPLING_UP_FACTOR)
            next_freq = max_freq;
    }
    else if(sampling_down_counter >= SAMPLING_DOWN_FACTOR) {
        next_freq = scale_down_by_n_of_max(freq, scale_down_factor);
    }

    if(next_freq > max_freq)
        next_freq = max_freq;
    if(next_freq < min_freq)
        next_freq = min_freq;

    set_gpufreq(next_freq);

    if(sampling_down_counter >= SAMPLING_DOWN_FACTOR)
        sampling_down_counter = 0;
    if(sampling_up_counter >= SAMPLING_UP_FACTOR)
        sampling_up_counter = 0;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   RETURNED: %d", __func__, __FILE__, __LINE__, next_freq);
#endif   // DEBUG or DEBUG_GOVERNOR
    return next_freq;
}

static void cnngov_20191211_print_gpugov(int fd) {

    char buff[MAX_BUFFLEN];
    size_t buff_len;

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n\nGOVERNOR INFORMATION");
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - name: %s", cnngov_20191211.name);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - TH1: %d.%d", (TH1 / 10), (TH1 % 10));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SCALE UP FACTOR: %d%", scale_up_factor);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SCALE DOWN FACTOR: %d%", scale_down_factor);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SAMPLING UP FACTOR: %d", SAMPLING_UP_FACTOR);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SAMPLING DOWN FACTOR: %d", SAMPLING_DOWN_FACTOR);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - MAX FREQ: %d", max_freq);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - MIN FREQ: %d", min_freq);
    write(fd, buff, buff_len);
}
