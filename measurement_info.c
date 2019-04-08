#include <stdio.h>
#include <errno.h>
#include "measurement_info.h"

#ifdef DEBUG
void print_stat_info(const struct sysfs_stat stat_info) {

    int i;

    printf("\n--- stat_info ---");
    printf("\n * num_sysfs_fd: %d", stat_info.num_sysfs_fd);
    printf("\n * sysfs file info");
    for(i=0; i<stat_info.num_sysfs_fd; i++) {
        printf("\n   - max_strlen[%d]: %d", i, stat_info.max_strlen[i]);
        printf("\n   - sysfs_fd[%d]: %d", i, stat_info.sysfs_fd[i]);
    }
    printf("\n-----------------\n");
}

void print_info(const struct measurement_info info) {

    printf("\n--- info ---");
    printf("\n * rawdata_fd: %d", info.rawdata_fd);
    printf("\n * num_sysfs_data: %d", info.num_sysfs_data);
    printf("\n * header raw: %s", info.header_raw);
    printf("\n------------\n");

    return;
}
#endif   // DEBUG
