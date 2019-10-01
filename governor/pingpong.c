#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "governor.h"
#include "../default_values.h"

static void pingpong_init(void *data);
static int32_t pingpong_get_target_freq();
static void pingpong_print_gpugov(int fd);

// Interface
struct gpugov pingpong = {

    .name = "gpugov-pingpong",
    .init = pingpong_init,
    .get_target_freq = pingpong_get_target_freq,
    .print_gpugov = pingpong_print_gpugov
};

/**
 *  Governor parameters
 */
static int high_freq;
static int low_freq;
static int interval      = 15;   // epochs
static int epoch_count;

static void pingpong_init(void *data) {
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    high_freq = gpugov_info.available_gpufreq[10];
    low_freq  = gpugov_info.available_gpufreq[4];

    set_gpufreq(low_freq);

    epoch_count = 0;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   FINISHED", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR
}

static int32_t pingpong_get_target_freq() {

    int64_t emcutil = get_emcutil();
    int16_t gpuutil = get_gpuutil();
    int32_t freq = get_gpufreq();
    int32_t next_freq;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
    printf("\n___\n%s() in %s:%d   GPU util: %d", __func__, __FILE__, __LINE__, gpuutil);
    printf("\n___\n%s() in %s:%d   EMC util: %d", __func__, __FILE__, __LINE__, emcutil);
#endif   // DEBUG or DEBUG_GOVERNOR

    ++epoch_count;

    if(epoch_count >= 2 * interval) {
        next_freq = low_freq;
        epoch_count = 0;
    }
    else if(epoch_count >= interval)
        next_freq = high_freq;
    else
        next_freq = freq;

    set_gpufreq(next_freq);

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   RETURNED: %d", __func__, __FILE__, __LINE__, next_freq);
#endif   // DEBUG or DEBUG_GOVERNOR
    return next_freq;
}

static void pingpong_print_gpugov(int fd) {

    char buff[MAX_BUFFLEN];
    size_t buff_len;

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n\nGOVERNOR INFORMATION");
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - name: %s", pingpong.name);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - High Freq: %d", high_freq);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - Low Freq: %d", low_freq);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - Interval: %d epochs", interval);
    write(fd, buff, buff_len);
}
