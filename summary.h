#ifndef UPDATE_SUMMARY_H
#define UPDATE_SUMMARY_H

#include "measurement_info.h"

void init_summary(summary_struct *summary);
void update_summary(summary_struct *summary, const powerlog_struct *powerlog_ptr);
struct timespec elapsed_time(const summary_struct summary);
#endif   // UPDATE_SUMMARY_H
