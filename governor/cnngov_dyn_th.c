#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "governor.h"
#include "../default_values.h"

static void cnngov_dyn_th_init(void *data);
static int32_t cnngov_dyn_th_get_target_freq();
static void cnngov_dyn_th_print_gpugov(int fd);

// Interface
struct gpugov cnngov_dyn_th = {

    .name = "gpugov-cnngov_dyn_th",
    .init = cnngov_dyn_th_init,
    .get_target_freq = cnngov_dyn_th_get_target_freq,
    .print_gpugov = cnngov_dyn_th_print_gpugov
};

/**
 *  Governor parameters
 */
// Scale-down when GPU UTIL < TH1 and EMC UTIL > TH2
static int TH1                       =   962;      // 96.2% (unit: x0.1%)
static int TH2                       =   421000;   // 42.1% (unit: x0.0001%)

// Scale-down when GPU UTIL > TH1 and EMC UTIL > TH3
static int TH3;

const static int TH3_ARRAY[]         = { 539000,   // 53.9% for  114 MHz
                                         539000,   // 53.9% for  216 MHz
                                         539000,   // 53.9% for  318 MHz
                                         539000,   // 53.9% for  420 MHz
                                         539000,   // 53.9% for  522 MHz
                                         539000,   // 53.9% for  624 MHz
                                         539000,   // 53.9% for  726 MHz
                                         513000,   // 51.3% for  828 MHz
                                         487000,   // 48.7% for  930 MHz
                                         461000,   // 46.1% for 1032 MHz
                                         435000,   // 43.5% for 1134 MHz
                                         409000,   // 40.3% for 1236 MHz
                                         383000    // 38.3% for 1300 Mhz
                                       };

// Scale-up when GPU UTIL > TH1 and EMC UTIL < TH4
static int TH4                       =   800000;   // 80.0% (unit: x0.0001%)
static int scale_up_factor           =   9;        // 9%    (unit: %)
static int scale_down_factor         =   9;        // 9%    (unit: %)

static const int SAMPLING_FACTOR     =   7;
static int sampling_counter          =   0;

static void cnngov_dyn_th_init(void *data) {
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    // Do nothing

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n%s() in %s:%d   FINISHED\n---", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR
}

static int32_t cnngov_dyn_th_get_target_freq() {

    int64_t emcutil;
    int16_t gpuutil;
    int32_t freq;
    int32_t next_freq;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    emcutil = get_emcutil();
    gpuutil = get_gpuutil();
    freq = get_gpufreq();
    next_freq = freq;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n%s() in %s:%d   GPU util: %d", __func__, __FILE__, __LINE__, gpuutil);
    printf("\n%s() in %s:%d   EMC util: %ld", __func__, __FILE__, __LINE__, emcutil);

    printf("\n%s() in %s:%d   TH1: %d", __func__, __FILE__, __LINE__, TH1);
    printf("\n%s() in %s:%d   TH2: %d", __func__, __FILE__, __LINE__, TH2);
    printf("\n%s() in %s:%d   TH3: (variable)", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   TH4: %d", __func__, __FILE__, __LINE__, TH4);
#endif   // DEBUG or DEBUG_GOVERNOR

    ++sampling_counter;

    if(sampling_counter >= SAMPLING_FACTOR) {

        sampling_counter = 0;

        if(gpuutil > TH1) {

            TH3 = TH3_ARRAY[gpufreq_hz_to_level(freq)];

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   TH3: %d", __func__, __FILE__, __LINE__, TH3);
#endif   // DEBUG or DEBUG_GOVERNOR

            if(emcutil > TH3)
                next_freq = scale_down_by_n_of_max(freq, scale_down_factor);
            else if(TH2 < TH3 && emcutil < TH2)
                next_freq = scale_up_by_n_of_max(freq, scale_up_factor);
            else
                next_freq = freq;
        }
        else {
            if(emcutil > TH4)
                next_freq = scale_down_by_n_of_max(freq, scale_up_factor);
            else
                next_freq = freq;
        }
    }

    set_gpufreq(next_freq);

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n%s() in %s:%d   RETURNED: %d\n---", __func__, __FILE__, __LINE__, next_freq);
#endif   // DEBUG or DEBUG_GOVERNOR
    return next_freq;
}

static void cnngov_dyn_th_print_gpugov(int fd) {

    char buff[MAX_BUFFLEN];
    size_t buff_len;

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n\nGOVERNOR INFORMATION");
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - name: %s", cnngov_dyn_th.name);
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

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SAMPLING FACTOR: %d", SAMPLING_FACTOR);
    write(fd, buff, buff_len);
}
