#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "governor.h"
#include "../default_values.h"

static void emc_conservative_init(void *data);
static int32_t emc_conservative_get_target_freq();
static void emc_conservative_print_gpugov(int fd);

// Interface
struct gpugov emc_conservative = {

    .name = "gpugov-emc_conservative",
    .init = emc_conservative_init,
    .get_target_freq = emc_conservative_get_target_freq,
    .print_gpugov = emc_conservative_print_gpugov
};

/**
 *  Governor parameters
 */
static int up_threshold      = 300000;     // 30% (unit: x0.0001%)
static int down_threshold    = 500000;     // 50% (unit: x0.0001%)
static int scale_up_factor   = 9;          // 9% (unit: %)
static int scale_down_factor = 9;          // 9% (unit: %)

static void emc_conservative_init(void *data) {
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    // Do nothing

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   FINISHED", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR
}

static int32_t emc_conservative_get_target_freq() {

    int16_t emcutil = get_emcutil();
    int32_t freq = get_gpufreq();
    int32_t next_freq;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    if(emcutil>down_threshold)
        next_freq = scale_down_by_n_of_max(freq, scale_down_factor);
    else if(emcutil<up_threshold)
        next_freq = scale_up_by_n_of_max(freq, scale_up_factor);
    else
        next_freq = freq;

    set_gpufreq(next_freq);

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   RETURNED: %d", __func__, __FILE__, __LINE__, next_freq);
#endif   // DEBUG or DEBUG_GOVERNOR
    return next_freq;
}


static void emc_conservative_print_gpugov(int fd) {

    char buff[MAX_BUFFLEN];
    size_t buff_len;

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n\nGOVERNOR INFORMATION");
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - name: %s", emc_conservative.name);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - UP THRESHOLD: %d.%d", (up_threshold / 10000), (up_threshold % 10000));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - DOWN THRESHOLD: %d.%d", (down_threshold / 10000), (down_threshold % 10000));
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SCALE UP FACTOR: %d%", scale_up_factor);
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - SCALE DOWN FACTOR: %d%", scale_down_factor);
    write(fd, buff, buff_len);
}
