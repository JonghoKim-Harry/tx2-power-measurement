#ifndef RAWDATA_TO_POWERLOG_H
#define RAWDATA_TO_POWERLOG_H

#include "measurement_info.h"

ssize_t timestamp_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);
ssize_t gpupower_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);
ssize_t gpufreq_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);
ssize_t gpuutil_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);

#endif   // RAWDATA_TO_POWERLOG_H
