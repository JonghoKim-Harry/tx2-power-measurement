#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "governor.h"
#include "../default_values.h"

static void feas_test_init(void *data);
static int32_t feas_test_get_target_freq();
static void feas_test_print_gpugov(int fd);

// Interface
struct gpugov feas_test = {

    .name = "gpugov-feas_test",
    .init = feas_test_init,
    .get_target_freq = feas_test_get_target_freq,
    .print_gpugov = feas_test_print_gpugov
};

/**
 *  Governor parameters
 */
// Scale-down when GPU UTIL < TH1 and EMC UTIL > TH4
static int TH1                        = 962;      // 96.2% (unit: x0.1%)
static int TH4                        = 500000;   // 80.0% (unit: x0.0001%)

// Scale-down when GPU UTIL > TH1 and EMC UTIL > TH3
static int TH3                        = 590000;   // 45.0% (unit: x0.0001%)

// Scale-up when GPU UTIL > TH1 and EMC UTIL < TH2
static int TH2                        = 565000;   // 35.0% (unit: x0.0001%)

static int scale_up_factor            = 9;        // 9% (unit: %)
static int scale_down_factor          = 9;        // 9% (unit: %)
static const int SAMPLING_DOWN_FACTOR = 7;
static const int SAMPLING_UP_FACTOR   = 7;
static int sampling_down_counter      = 0;
static int sampling_up_counter        = 0;

static int32_t max_freq, min_freq;

static void feas_test_init(void *data) {
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    sampling_down_counter = 0;
    sampling_up_counter = 0;

    min_freq = gpugov_info.available_gpufreq[8];
    max_freq = gpugov_info.available_gpufreq[10];

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   FINISHED", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR
}

static int32_t feas_test_get_target_freq() {

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

    if(gpuutil < TH1) {
        if(sampling_down_counter >= SAMPLING_DOWN_FACTOR && emcutil > TH4)
            next_freq = scale_down_by_n_of_max(freq, scale_up_factor);
        else
            next_freq = freq;
    }
    else {
        if(sampling_down_counter >= SAMPLING_DOWN_FACTOR && emcutil > TH3)
            next_freq = scale_down_by_n_of_max(freq, scale_down_factor);
        else if(sampling_up_counter >= SAMPLING_UP_FACTOR && emcutil < TH2)
            next_freq = scale_up_by_n_of_max(freq, scale_up_factor);
        else
            next_freq = freq;
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

static void feas_test_print_gpugov(int fd) {

    char buff[MAX_BUFFLEN];
    size_t buff_len;

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n\nGOVERNOR INFORMATION");
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - name: %s", feas_test.name);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - TH1: %d.%d", (TH1 / 10), (TH1 % 10));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - TH2: %d.%d", (TH2 / 10000), (TH2 % 10000));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - TH3: %d.%d", (TH3 / 10000), (TH3 % 10000));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - TH4: %d.%d", (TH4 / 10000), (TH4 % 10000));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SCALE UP FACTOR: %d%", scale_up_factor);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SCALE DOWN FACTOR: %d%", scale_down_factor);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SAMPLING UP FACTOR: %d", SAMPLING_UP_FACTOR);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SAMPLING DOWN FACTOR: %d", SAMPLING_DOWN_FACTOR);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - MIN FREQ: %d", min_freq);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - MAX FREQ: %d", max_freq);
    write(fd, buff, buff_len);
}
