#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "governor.h"
#include "../default_values.h"

static void scale_10_init(void *data);
static int32_t scale_10_get_target_freq();
static void scale_10_print_gpugov(int fd);

// Interface
struct gpugov scale_10 = {

    .name = "gpugov-scale_10",
    .init = scale_10_init,
    .get_target_freq = scale_10_get_target_freq,
    .print_gpugov = scale_10_print_gpugov
};

/**
 *  Governor parameters
 */
static int scale_down_factor         = 9;        // 9% (unit: %)
static int max_flag;
static int32_t max_freq;
static int32_t epoch_count;
static const int period = 10;

static void scale_10_init(void *data) {
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    max_flag = 1;
    max_freq = gpugov_info.max_gpufreq;
    epoch_count = 0;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   FINISHED", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR
}

static int32_t scale_10_get_target_freq() {

    int32_t next_freq;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    if(epoch_count % period == 0) {

        if(max_flag) {
            next_freq = max_freq;
            max_flag = 0;
        }
        else {
            next_freq = scale_down_by_n_of_max(max_freq, scale_down_factor);
            max_flag = 1;
        }
        set_gpufreq(next_freq);
    }

    ++epoch_count;
    if(epoch_count >= 1e6)
        epoch_count -= 1e6;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   RETURNED: %d", __func__, __FILE__, __LINE__, next_freq);
#endif   // DEBUG or DEBUG_GOVERNOR
    return next_freq;
}

static void scale_10_print_gpugov(int fd) {

    char buff[MAX_BUFFLEN];
    size_t buff_len;

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n\nGOVERNOR INFORMATION");
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - name: %s", scale_10.name);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SCALE DOWN FACTOR: %d%", scale_down_factor);
    write(fd, buff, buff_len);
}
