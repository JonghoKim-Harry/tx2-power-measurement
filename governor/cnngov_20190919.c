#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "governor.h"
#include "../default_values.h"

static void cnngov_20190919_init(void *data);
static int32_t cnngov_20190919_get_target_freq();
static void cnngov_20190919_print_gpugov(int fd);

// Interface
struct gpugov cnngov_20190919 = {

    .name = "gpugov-cnngov_20190919",
    .init = cnngov_20190919_init,
    .get_target_freq = cnngov_20190919_get_target_freq,
    .print_gpugov = cnngov_20190919_print_gpugov
};

/**
 *  Governor parameters
 */
static int TH1                       = 800;      // 80% (unit: x0.1%)
static int TH2                       = 250000;   // 25% (unit: x0.0001%)
static int TH3                       = 200000;   // 20% (unit: x0.0001%)
static int TH4                       = 350000;   // 35% (unit: x0.0001%)
static int TH5                       = 300000;   // 30% (unit: x0.0001%)
static int scale_up_factor           = 9;        // 9% (unit: %)
static int scale_down_factor         = 9;        // 9% (unit: %)

static void cnngov_20190919_init(void *data) {
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    // Do nothing

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   FINISHED", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR
}

static int32_t cnngov_20190919_get_target_freq() {

    int64_t emcutil = get_emcutil();
    int16_t gpuutil = get_gpuutil();
    int32_t freq = get_gpufreq();
    int32_t next_freq;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
    printf("\n___\n%s() in %s:%d   GPU util: %d", __func__, __FILE__, __LINE__, gpuutil);
    printf("\n___\n%s() in %s:%d   EMC util: %d", __func__, __FILE__, __LINE__, emcutil);
#endif   // DEBUG or DEBUG_GOVERNOR

    if(gpuutil < TH1) {
    
        if(emcutil > TH2)
            next_freq = scale_down_by_n_of_max(freq, scale_down_factor);
        else if(emcutil < TH3)
            next_freq = scale_up_by_n_of_max(freq, scale_up_factor);
        else
            next_freq = freq;
    }
    else {
        if(emcutil > TH4)
            next_freq = scale_down_by_n_of_max(freq, scale_down_factor);
        else if(emcutil < TH5)
            next_freq = scale_up_by_n_of_max(freq, scale_up_factor);
        else
            next_freq = freq;
    }

    set_gpufreq(next_freq);

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   RETURNED: %d", __func__, __FILE__, __LINE__, next_freq);
#endif   // DEBUG or DEBUG_GOVERNOR
    return next_freq;
}

static void cnngov_20190919_print_gpugov(int fd) {

    char buff[MAX_BUFFLEN];
    size_t buff_len;

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n\nGOVERNOR INFORMATION");
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - name: %s", cnngov_20190919.name);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - TH1: %d.%d", (TH1 / 10), (TH1 % 10));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - TH2: %d.%d", (TH2 / 10000), (TH2 % 10000));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - TH3: %d.%d", (TH3 / 10000), (TH3 % 10000));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - TH4: %d.%d", (TH4 / 10000), (TH4 % 10000));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - TH5: %d.%d", (TH5 / 10000), (TH5 % 10000));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SCALE UP FACTOR: %d%", scale_up_factor);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SCALE DOWN FACTOR: %d%", scale_down_factor);
    write(fd, buff, buff_len);
}
