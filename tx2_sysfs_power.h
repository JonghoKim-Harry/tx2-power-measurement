#ifndef TX2_SYSFS_POWER_H
#define TX2_SYSFS_POWER_H

// In order to stringify number
#define TOSTRING(n) STR(n)
#define STR(n) #n

// Informations which requires root previlege:
//   * TX2_SYSFS_EMC_MINFREQ
//   * TX2_SYSFS_EMC_MAXFREQ
//   * TX2_SYSFS_EMC_FREQ
//   * TX2_SYSFS_APE_FREQ

// I assume that CPU/GPU/MEM power < 100W
#define TX2_SYSFS_GPU_MHZFREQ_MAX_STRLEN  4       // MHz, up to 1300
#define TX2_SYSFS_GPU_FREQ_MAX_STRLEN    10 + 1   // Hz and trailing '\n'
#define TX2_SYSFS_GPU_POWER_MAX_STRLEN    5       // mW
#define TX2_SYSFS_GPU_UTIL_MAX_STRLEN     5       // x0.1%, up to 1000
#define TX2_SYSFS_GPU_TEMP_MAX_STRLEN     7       // x0.001 Celsius degree,
                                                  // up to 1000000
#define TX2_SYSFS_CPU_ONLINE_MAX_STRLEN   1       // Boolean: 0, 1
#define TX2_SYSFS_CPU_FREQ_MAX_STRLEN     7       // kHz, up to 2035200
#define TX2_SYSFS_CPU_POWER_MAX_STRLEN    5       // mW
#define TX2_SYSFS_MEM_POWER_MAX_STRLEN    5       // mW
#define TX2_SYSFS_EMC_UTIL_MAX_STRLEN     7       // x0.0001%, up to 1000000
#define TX2_SYSFS_EMC_FREQ_MAX_STRLEN    10 + 1   // MHz, up to 1300
#define TX2_SYSFS_EMC_MHZFREQ_MAX_STRLEN  4       // MHz, up to 1300

// Thermal Informations (x1000 degree Celsius)
//   * BCPU-therm
//   * MCPU-therm
//   * GPU-therm
//   * PLL-therm
//   * Tboard_tegra
//   * Tdiode_tegra
//   * PMIC-Die
//   * thermal-fan-est
#define TX2_SYSFS_BCPU_TEMP   "/sys/devices/virtual/thermal/thermal_zone0/temp"
#define TX2_SYSFS_MCPU_TEMP   "/sys/devices/virtual/thermal/thermal_zone1/temp"
#define TX2_SYSFS_GPU_TEMP    "/sys/devices/virtual/thermal/thermal_zone2/temp"
#define TX2_SYSFS_PLL_TEMP    "/sys/devices/virtual/thermal/thermal_zone3/temp"
#define TX2_SYSFS_TBOARD_TEMP "/sys/devices/virtual/thermal/thermal_zone4/temp"
#define TX2_SYSFS_TDIODE_TEMP "/sys/devices/virtual/thermal/thermal_zone5/temp"
#define TX2_SYSFS_PMIC_TEMP   "/sys/devices/virtual/thermal/thermal_zone6/temp"
#define TX2_SYSFS_FAN_TEMP    "/sys/devices/virtual/thermal/thermal_zone7/temp"

// Power Measurement I (mW, integer)
#define TX2_SYSFS_GPU_POWER    \
    "/sys/devices/3160000.i2c/i2c-0/0-0040/iio_device/in_power0_input"
#define TX2_SYSFS_SOC_POWER    \
    "/sys/devices/3160000.i2c/i2c-0/0-0040/iio_device/in_power1_input"
#define TX2_SYSFS_WIFI_POWER   \
    "/sys/devices/3160000.i2c/i2c-0/0-0040/iio_device/in_power2_input"

// Power Measurement II (mW, integer)
#define TX2_SYSFS_ALL_POWER    \
    "/sys/devices/3160000.i2c/i2c-0/0-0041/iio_device/in_power0_input"
#define TX2_SYSFS_CPU_POWER    \
    "/sys/devices/3160000.i2c/i2c-0/0-0041/iio_device/in_power1_input"
#define TX2_SYSFS_MEM_POWER    \
    "/sys/devices/3160000.i2c/i2c-0/0-0041/iio_device/in_power2_input"

// CPU Governor Informations
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

// GPU Governor Informations
// Note that GPU utilization is given by x10%, integer
#define TX2_SYSFS_GPU     \
    "/sys/devices/17000000.gp10b"
#define TX2_SYSFS_GPU_GOVERNOR    \
    TX2_SYSFS_GPU "/devfreq/17000000.gp10b/governor"
#define TX2_SYSFS_GPU_RAILGATE   \
    TX2_SYSFS_GPU "/railgate_enable"
#define TX2_SYSFS_NUM_AVAILABLE_GPUFREQ    14
#define TX2_SYSFS_AVAILABLE_GPUFREQ    \
    TX2_SYSFS_GPU "/devfreq/17000000.gp10b/available_frequencies"
#define TX2_SYSFS_GPU_MINFREQ   \
    TX2_SYSFS_GPU "/devfreq/17000000.gp10b/min_freq"
#define TX2_SYSFS_GPU_MAXFREQ   \
    TX2_SYSFS_GPU "/devfreq/17000000.gp10b/max_freq"
#define TX2_SYSFS_GPU_FREQ    \
    TX2_SYSFS_GPU "/devfreq/17000000.gp10b/cur_freq"
#define TX2_SYSFS_GPU_UTIL   \
    TX2_SYSFS_GPU "/load"
#define TX2_SYSFS_GPU_SET_FREQ \
    TX2_SYSFS_GPU "/devfreq/17000000.gp10b/userspace/set_freq"

// EMC Information
// Note that getting EMC frequency informations requires root previlege
#define TX2_SYSFS_EMC_MAXFREQ   \
    "/sys/kernel/debug/bpmp/debug/clk/emc/max_rate"
#define TX2_SYSFS_EMC_MINFREQ   \
    "/sys/kernel/debug/bpmp/debug/clk/emc/min_rate"
#define TX2_SYSFS_EMC_FREQ   \
    "/sys/kernel/debug/clk/emc/clk_rate"
#define TX2_SYSFS_EMC_UTIL \
    "/sys/kernel/actmon_avg_activity/mc_all"

// APE (Audio Processing Engine)
// Note that getting APE frequency informations requires root previlege
// Hz, integer
#define TX2_SYSFS_APE_FREQ \
    "/sys/kernel/debug/clk/ape/clk_rate"

#endif   // TX2_SYSFS_POWER_H
