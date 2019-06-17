#ifndef LOG_TO_STAT_H
#define LOG_TO_STAT_H

#include "measurement_info.h"

// Timestamp to Statistics
ssize_t timestamp_to_stat(const int stat_fd, const int colwidth, const struct timespec timestamp);
ssize_t elapsedtime_to_stat(const int stat_fd, const int colwidth, const struct timespec timestamp, const struct timespec baseline);

// Powerlog to Statistics
ssize_t gpupower_to_stat (const int stat_fd, const int colwidth, const powerlog_struct powerlog);
ssize_t gpufreq_to_stat  (const int stat_fd, const int colwidth, const powerlog_struct powerlog);
ssize_t gpuutil_to_stat  (const int stat_fd, const int colwidth, const powerlog_struct powerlog);

// Powerlog Summary to Statistics
ssize_t gpuenergy_to_stat  (const int stat_fd, const int colwidth, const powerlog_summary_struct powerlog_summary);

// TODO
ssize_t powerevent_to_stat(const int stat_fd, const int colwidth, const powerlog_summary_struct powerlog_summary);

// Caffelog to Statistics
ssize_t caffeevent_to_stat(const int stat_fd, const int colwidth, const caffelog_struct caffelog);
ssize_t cnn_event_to_stat(const int stat_fd, const int colwidth, const caffelog_struct caffelog);
ssize_t batch_idx_to_stat(const int stat_fd, const int colwidth, const caffelog_struct caffelog);
ssize_t batch_finish_to_stat(const int stat_fd, const int colwidth, const caffelog_struct caffelog);

#endif   // LOG_TO_STAT_H
