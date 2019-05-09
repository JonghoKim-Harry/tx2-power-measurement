#include <stdio.h>

#include "stat.h"
    
static const char separation_line1[256] = "\n__________________________________________________________________________________________________________________________________________________________________________";
static const char separation_line2[256] = "\n--------------------------------------------------------------------------------------------------------------------------------------------------------------------------";


ssize_t print_header_raw(const int stat_fd, const measurement_info_struct info) {

    ssize_t num_written_bytes, total_written_bytes;
    char buff[MAX_COLWIDTH];
    int i;
    int buff_len;

    total_written_bytes = 0;

    num_written_bytes = write(stat_fd, separation_line1, strlen(separation_line1));
    total_written_bytes += num_written_bytes;

    num_written_bytes = write(stat_fd, "\n", 1);
    total_written_bytes += num_written_bytes;

    for(i=0; i<info.num_stat; i++) {

        num_written_bytes = write(stat_fd, "  ", 2);
        total_written_bytes += num_written_bytes;

        buff_len = snprintf(buff, MAX_COLWIDTH, "%*s", info.stat_info[i].colwidth, info.stat_info[i].colname);
        num_written_bytes = write(stat_fd, buff, buff_len);
        total_written_bytes += num_written_bytes;
    }

    num_written_bytes = write(stat_fd, separation_line2, strlen(separation_line2));
    total_written_bytes += num_written_bytes;

    return total_written_bytes;
}
