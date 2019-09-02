#include <stdio.h>
#include <stdlib.h>
#include "governor.h"

#define   UP_THRESHOLD     800
#define DOWN_THRESHOLD     500
#define SCALE_DOWN_RATIO   0.8

static void ondemand8050_init(void *data);
static int32_t ondemand8050_get_target_freq();

// Global
struct gpugov ondemand8050 = {

    .name = "gpugov-ondemand8050",
    .init = ondemand8050_init,
    .get_target_freq = ondemand8050_get_target_freq,
};

static size_t *scale_down_table;

static int32_t reduce_gpufreq_by20(const int32_t gpufreq) {

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   get input: %d", __func__, __FILE__, __LINE__, gpufreq);
    printf("\n___\n%s() in %s:%d   RETURN: ", __func__, __FILE__, __LINE__, scale_down_table[gpufreq_hz_to_level(gpufreq)]);
#endif   // DEBUG or DEBUG_GOVERNOR

    return scale_down_table[gpufreq_hz_to_level(gpufreq)];
}

static void ondemand8050_init(void *data) {

    const int32_t min_freq = gpugov_info.min_gpufreq;
    const int32_t *freq = gpugov_info.available_gpufreq;
    const size_t num_freq = gpugov_info.num_available_gpufreq;
    size_t level, target_level;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR


    // Build scale-down table
    scale_down_table = malloc(num_freq * sizeof(size_t));

    for(level = (num_freq - 1); level > 0; --level) {

        if(freq[level] >= min_freq) {
            target_level = level;
            goto fill_up_table;
        }

        for(target_level = (level-1); target_level >= 0; --target_level) {
            if(freq[target_level] < SCALE_DOWN_RATIO * freq[level])
                goto fill_up_table;
        }
fill_up_table:
        scale_down_table[level] = target_level;
    }

    scale_down_table[0] = 0;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n%s() in %s:%d   FINISH", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    return;
}

static int32_t ondemand8050_get_target_freq() {

    int16_t gpuutil;
    int32_t new_gpufreq;
    
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    gpuutil = get_gpuutil();

    if(gpuutil > UP_THRESHOLD)
        new_gpufreq = gpugov_info.max_gpufreq;
    else if(gpuutil < DOWN_THRESHOLD)
        new_gpufreq = reduce_gpufreq_by20(get_gpufreq());
    else
        new_gpufreq = get_gpufreq();

    set_gpufreq(new_gpufreq);


#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n%s() in %s:%d   gpuutil: %d -> new_gpufreq: %d", __func__, __FILE__, __LINE__, gpuutil, new_gpufreq);
    printf("\n%s() in %s:%d   RETURN: %d\n---", __func__, __FILE__, __LINE__, new_gpufreq);
#endif   // DEBUG or DEBUG_GOVERNOR
    return new_gpufreq;
}
