#include <stdio.h>
#include "governor.h"

#define   UP_THRESHOLD   800
#define DOWN_THRESHOLD   500

static int32_t reduce_gpufreq_by20(const int32_t gpufreq) {

    int32_t reduced_gpufreq;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   get input: %d", __func__, __FILE__, __LINE__, gpufreq);
#endif   // DEBUG or DEBUG_GOVERNOR

    switch(gpufreq) {

        case 140250000:
        case 229500000:
            reduced_gpufreq = 140250000;
            break;

        case 318750000:
            reduced_gpufreq = 229500000;
            break;

        case 408000000:
            reduced_gpufreq = 318750000;
            break;

        case 497250000:
        case 586500000:
            reduced_gpufreq = 408000000;
            break;

        case 675750000:
        case 765000000:
            reduced_gpufreq = 586500000;
            break;

        case 854250000:
            reduced_gpufreq = 675750000;
            break;

        case 943500000:
            reduced_gpufreq = 765000000;
            break;

        case 1032750000:
        case 1122000000:
            reduced_gpufreq = 854250000;
            break;

        case 1211250000:
            reduced_gpufreq = 943500000;
            break;

        case 1300500000:
            reduced_gpufreq = 1032750000;
            break;

        default:
            reduced_gpufreq = -1;
            break;
    }

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n%s() in %s:%d   RETURN: %d\n---", __func__, __FILE__, __LINE__, reduced_gpufreq);
#endif   // DEBUG or DEBUG_GOVERNOR

    return reduced_gpufreq;
}

int32_t determine_gpufreq() {

    int16_t gpuutil;
    int32_t new_gpufreq;
    
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    gpuutil = get_gpuutil();

    if(gpuutil > UP_THRESHOLD)
        new_gpufreq = max_gpufreq;
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
