#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "governor.h"
#include "../default_values.h"

static void do_nothing_init(void *data);
static int32_t do_nothing_get_target_freq();
static void do_nothing_print_gpugov(int fd);

// Interface
struct gpugov do_nothing = {

    .name = "gpugov-do_nothing",
    .init = do_nothing_init,
    .get_target_freq = do_nothing_get_target_freq,
    .print_gpugov = do_nothing_print_gpugov
};

/**
 *  Governor parameters: NOTHING
 */

static void do_nothing_init(void *data) {
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    set_gpufreq(gpugov_info.max_gpufreq);

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   FINISHED", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR
}

static int32_t do_nothing_get_target_freq() {

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    // Do nothing

    return gpugov_info.max_gpufreq;
}

static void do_nothing_print_gpugov(int fd) {

    char buff[MAX_BUFFLEN];
    size_t buff_len;

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n\nGOVERNOR INFORMATION");
    write(fd, buff, buff_len);

    buff_len = snprintf(buff, MAX_BUFFLEN, "\n - name: %s", do_nothing.name);
    write(fd, buff, buff_len);
}
