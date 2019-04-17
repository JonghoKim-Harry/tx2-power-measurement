#ifndef TX2_SYSFS_POWER_H
#define TX2_SYSFS_POWER_H

// In order to stringify number
#define TOSTRING(n) STR(n)
#define STR(n) #n

// I assume that GPU power < 100W
#define TX2_SYSFS_GPU_FREQ_MAX_STRLEN 10   // Hz
#define TX2_SYSFS_GPU_POWER_MAX_STRLEN 5   // mW
#define TX2_SYSFS_GPU_UTIL_MAX_STRLEN 5    // 100.0
#define TX2_SYSFS_CPU_ONLINE_MAX_STRLEN 1  // Boolean: 0, 1
#define TX2_SYSFS_CPU_FREQ_MAX_STRLEN 7    // kHz
#define TX2_SYSFS_CPU_POWER_MAX_STRLEN 5   // mW

/* Power Measurement I */
#define TX2_SYSFS_POWER_GPU    \
    "/sys/devices/3160000.i2c/i2c-0/0-0040/iio_device/in_power0_input"
#define TX2_SYSFS_POWER_SOC    \
    "/sys/devices/3160000.i2c/i2c-0/0-0040/iio_device/in_power1_input"
#define TX2_SYSFS_POWER_WIFI   \
    "/sys/devices/3160000.i2c/i2c-0/0-0040/iio_device/in_power2_input"

/* Power Measurement II */
#define TX2_SYSFS_POWER_ALL    \
    "/sys/devices/3160000.i2c/i2c-0/0-0041/iio_device/in_power0_input"
#define TX2_SYSFS_POWER_CPU    \
    "/sys/devices/3160000.i2c/i2c-0/0-0041/iio_device/in_power1_input"
#define TX2_SYSFS_POWER_DDR    \
    "/sys/devices/3160000.i2c/i2c-0/0-0041/iio_device/in_power2_input"

/* CPU Governor Informations */
#define TX2_SYSFS_CPUFREQ_GROUP(n)   \
    "/sys/devices/system/cpu/cpufreq/policy" TOSTRING(n)
#define TX2_SYSFS_CPUFREQ_GROUP_REGISTERED_CPUS(n)   \
    TX2_SYSFS_CPUFREQ_GROUP(n) "/related_cpus"
#define TX2_SYSFS_CPUFREQ_GROUP_ACTIVE_CPUS(n)   \
    TX2_SYSFS_CPUFREQ_GROUP(n) "/affected_cpus"
#define TX2_SYSFS_CPUFREQ_GROUP_GOVERNOR(n)   \
    TX2_SYSFS_CPUFREQ_GROUP(n) "/scaling_governor"
#define TX2_SYSFS_CPUFREQ_GROUP_FREQ(n)   \
    TX2_SYSFS_CPUFREQ_GROUP(n) "/scaling_cur_freq"
#define TX2_SYSFS_CPUFREQ_GROUP_MINFREQ(n)   \
    TX2_SYSFS_CPUFREQ_GROUP(n) "/scaling_min_freq"
#define TX2_SYSFS_CPUFREQ_GROUP_MAXFREQ(n)   \
    TX2_SYSFS_CPUFREQ_GROUP(n) "/scaling_max_freq"

#define TX2_SYSFS_CPU(n)      \
    "/sys/devices/system/cpu/cpu" TOSTRING(n)
#define TX2_SYSFS_CPU_GOVERNOR(n) \
    TX2_SYSFS_CPU(n) "/cpufreq/scaling_governor"
#define TX2_SYSFS_CPU_FREQ(n) \
    TX2_SYSFS_CPU(n) "/cpufreq/scaling_cur_freq"
#define TX2_SYSFS_CPU_ONLINE(n) \
    TX2_SYSFS_CPU(n) "/online"

/* GPU Governor Informations */
#define TX2_SYSFS_GPU     \
    "/sys/devices/17000000.gp10b"
#define TX2_SYSFS_GPU_GOVERNOR    \
    TX2_SYSFS_GPU "/devfreq/17000000.gp10b/governor"
#define TX2_SYSFS_GPU_RAILGATE   \
    TX2_SYSFS_GPU "/railgate_enable"
#define TX2_SYSFS_GPU_MINFREQ   \
    TX2_SYSFS_GPU "/devfreq/17000000.gp10b/min_freq"
#define TX2_SYSFS_GPU_MAXFREQ   \
    TX2_SYSFS_GPU "/devfreq/17000000.gp10b/max_freq"
#define TX2_SYSFS_GPU_FREQ    \
    TX2_SYSFS_GPU "/devfreq/17000000.gp10b/cur_freq"
#define TX2_SYSFS_GPU_UTIL   \
    TX2_SYSFS_GPU "/load"

#endif   // TX2_SYSFS_POWER_H
