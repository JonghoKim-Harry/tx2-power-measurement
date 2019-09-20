#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "governor.h"
#include "../default_values.h"

static void policy_0_or_not_init(void *data);
static int32_t policy_0_or_not_get_target_freq();
static void policy_0_or_not_print_gpugov(int fd);

// Interface
struct gpugov policy_0_or_not = {

    .name = "gpugov-policy_0_or_not",
    .init = policy_0_or_not_init,
    .get_target_freq = policy_0_or_not_get_target_freq,
    .print_gpugov = policy_0_or_not_print_gpugov
};

/**
 *  Governor parameters
 */
static int32_t high_freq;
static int32_t low_freq;

static void policy_0_or_not_init(void *data) {
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    high_freq = gpugov_info.available_gpufreq[4];
    low_freq  = gpugov_info.available_gpufreq[10];

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   FINISHED", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR
}

static int32_t policy_0_or_not_get_target_freq() {

    int64_t emcutil = get_emcutil();
    int16_t gpuutil = get_gpuutil();
    int32_t freq = get_gpufreq();
    int32_t next_freq;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
    printf("\n___\n%s() in %s:%d   GPU util: %d", __func__, __FILE__, __LINE__, gpuutil);
    printf("\n___\n%s() in %s:%d   EMC util: %d", __func__, __FILE__, __LINE__, emcutil);
#endif   // DEBUG or DEBUG_GOVERNOR

    if(gpuutil == 0)
        next_freq = low_freq;
    else
        next_freq = high_freq;

    set_gpufreq(next_freq);

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   RETURNED: %d", __func__, __FILE__, __LINE__, next_freq);
#endif   // DEBUG or DEBUG_GOVERNOR
    return next_freq;
}

static void policy_0_or_not_print_gpugov(int fd) {

    char buff[MAX_BUFFLEN];
    size_t buff_len;

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n\nGOVERNOR INFORMATION");
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - name: %s", policy_0_or_not.name);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - GPU LOW FREQ: %d Hz", low_freq);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - GPU HIGH FREQ: %d Hz", high_freq);
    write(fd, buff, buff_len);
}
