#ifndef LOG_TO_STAT_H
#define LOG_TO_STAT_H

#include "measurement_info.h"
#include "powerlog.h"
#include "summary.h"
#include "caffelog.h"

// Timestamp to Statistics
ssize_t timestamp_to_stat(const int stat_fd, const int colwidth, const struct timespec timestamp);
ssize_t elapsedtime_to_stat(const int stat_fd, const int colwidth, const struct timespec timestamp, const struct timespec baseline);

// Powerlog to Statistics
#ifdef TRACE_POWER
ssize_t boardpower_to_stat (const int stat_fd, const int colwidth, const powerlog_struct powerlog);
ssize_t socpower_to_stat (const int stat_fd, const int colwidth, const powerlog_struct powerlog);
ssize_t wifipower_to_stat (const int stat_fd, const int colwidth, const powerlog_struct powerlog);
#endif   // TRACE_POWER

ssize_t gpupower_to_stat (const int stat_fd, const int colwidth, const powerlog_struct powerlog);
ssize_t gpufreq_to_stat  (const int stat_fd, const int colwidth, const powerlog_struct powerlog);
ssize_t gpuutil_to_stat  (const int stat_fd, const int colwidth, const powerlog_struct powerlog);

#ifdef TRACE_CPU
ssize_t allcpu_power_to_stat (const int stat_fd, const int colwidth, const powerlog_struct powerlog);
#endif   // TRACE_CPU

#ifdef TRACE_MEM
ssize_t mempower_to_stat (const int stat_fd, const int colwidth, const powerlog_struct powerlog);
ssize_t emcfreq_to_stat  (const int stat_fd, const int colwidth, const powerlog_struct powerlog);
ssize_t emcutil_to_stat  (const int stat_fd, const int colwidth, const powerlog_struct powerlog);
#endif   // TRACE_MEM

// Powerlog Summary to Statistics
ssize_t gpuenergy_to_stat  (const int stat_fd, const int colwidth, const summary_struct summary);
ssize_t psum_gpuutil_to_stat (const int stat_fd, const int colwidth, const summary_struct summary);
ssize_t avg_gpuutil_to_stat (const int stat_fd, const int colwidth, const summary_struct summary);
ssize_t gpuenergy_to_stat  (const int stat_fd, const int colwidth, const summary_struct summary);
ssize_t boardenergy_to_stat  (const int stat_fd, const int colwidth, const summary_struct summary);

#ifdef TRACE_MEM
ssize_t memenergy_to_stat  (const int stat_fd, const int colwidth, const summary_struct summary);
#endif   // TRACE_MEM

//
ssize_t powerevent_to_stat(const int stat_fd, const int colwidth, const summary_struct summary);

// Caffelog to Statistics
ssize_t caffeevent_to_stat(const int stat_fd, const int colwidth, const caffelog_struct caffelog);
ssize_t cnn_event_to_stat(const int stat_fd, const int colwidth, const caffelog_struct caffelog);
ssize_t batch_idx_to_stat(const int stat_fd, const int colwidth, const caffelog_struct caffelog);
ssize_t batch_finish_to_stat(const int stat_fd, const int colwidth, const caffelog_struct caffelog);

#endif   // LOG_TO_STAT_H
