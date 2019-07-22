#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "measurement_info.h"
#include "powerlog.h"

#ifdef DEBUG
void print_info(const struct measurement_info_struct info) {

    printf("\n--- info ---");
    printf("\n * num_rawdata: %d", info.num_rawdata);
    printf("\n * num_stat: %d", info.num_stat);
    printf("\n------------\n");

    return;
}

void print_rawdata_info(const rawdata_info_struct rawdata_info) {

    int i;

    printf("\n--- rawdta_info ---");
    printf("\n * num_sysfs_fd: %d", rawdata_info.num_sysfs_fd);
    printf("\n * sysfs file info");
    for(i=0; i<rawdata_info.num_sysfs_fd; i++) {
        printf("\n   - sysfs_fd[%d]: %d", i, rawdata_info.sysfs_fd[i]);
    }
    printf("\n-----------------\n");
}
#endif   // DEBUG

void init_info(measurement_info_struct *info) {

    info->userspace_gpugovernor = 0;
    info->num_rawdata = 0;
    info->num_row  = 0;
    info->num_stat = 0;

    return;
}


void close_sysfs_files(measurement_info_struct info) {

    int i, j;
    rawdata_info_struct *target;
    int num1, num2;

#ifdef DEBUG
    printf("\nclose_sysfs_files()   START");
#endif   // DEBUG

    num1 = info.num_rawdata;

    for(i=0; i<num1; i++) {
    
        target = &info.rawdata_info[i];
        num2 = target->num_sysfs_fd;
        for(j=0; j<num2; j++)
            close(target->sysfs_fd[j]);
    }

#ifdef DEBUG
    printf("\nclose_sysfs_files()   FINISHED");
#endif   // DEBUG
};
