#ifndef LOG_TO_STAT_H
#define LOG_TO_STAT_H

#include "measurement_info.h"

// Powerlog to Statistics
ssize_t timestamp_to_stat(const int stat_fd, const int colwidth, const powerlog_struct powerlog);
ssize_t gpupower_to_stat (const int stat_fd, const int colwidth, const powerlog_struct powerlog);
ssize_t gpufreq_to_stat  (const int stat_fd, const int colwidth, const powerlog_struct powerlog);
ssize_t gpuutil_to_stat  (const int stat_fd, const int colwidth, const powerlog_struct powerlog);

// Powerlog Summary to Statistics
ssize_t elapsedtime_to_stat(const int stat_fd, const int colwidth, const powerlog_summary_struct powerlog_summary);
ssize_t gpuenergy_to_stat  (const int stat_fd, const int colwidth, const powerlog_summary_struct powerlog_summary);

// TODO: Caffelog to Statistics

#endif   // LOG_TO_STAT_H
