#include "stats.h"

/*
ssize_t sysfs_to_stat(const int stat_fd, const char *sysfs_filename, const char *stat_format) {

    ssize_t num_read_bytes;
    int fd;
    char sysfs_str[64], buff[128];
    int buff_len;
    if(!access(sysfs_filename, F_OK)) {

        fd = open(sysfs_filename, O_RDONLY | O_NONBLOCK);
        lseek(fd, 0, SEEK_SET);
        num_read_bytes = read(fd, sysfs_str, 64);
        if(num_read_bytes > 0) {
            sysfs_str[num_read_bytes - 1] = '\0';   // Removing '\n'
            buff_len = snprintf(buff, 128, stat_format, sysfs_str);
            write(stat_fd, buff, buff_len);

            buff_len = snprintf(buff, 128, "   (%s)", sysfs_filename);
            write(stat_fd, buff, buff_len);
        }
        close(fd);
    }

    return num_read_bytes;
}

ssize_t read_sysfs_1(const struct sysfs_stat rawdata_info, const int rawdata_fd) {

    ssize_t num_read_bytes;
    const int max_strlen = rawdata_info.max_strlen[0];
    const int sysfs_fd = rawdata_info.sysfs_fd[0];
    int i;
    char buff[128];

#ifdef DEBUG
    printf("\nread_sysfs_1()   START");
#endif   // DEBUG

    lseek(sysfs_fd, 0, SEEK_SET);
    num_read_bytes = read(sysfs_fd, buff, max_strlen);

#ifdef DEBUG
    if(num_read_bytes == -1)
        printf("\nERROR in read_sysfs_1(): %s", strerror(errno));
#endif   // DEBUG

    if(buff[num_read_bytes-1] == '\n' || buff[num_read_bytes-1] == EOF || buff[num_read_bytes-1] == ' ') {
        buff[num_read_bytes-1] = '\0';
        --num_read_bytes;
    }

    for(i=0; i<(max_strlen - num_read_bytes ); i++)
        write(rawdata_fd, " ", 1);
    write(rawdata_fd, buff, num_read_bytes);

#ifdef DEBUG
    printf("\nread_sysfs_1()   num_read_bytes: %ld", num_read_bytes);
    printf("\nread_sysfs_1()   max_strlen: %d", max_strlen);
    printf("\nread_sysfs_1()   sysfs_fd: %d", sysfs_fd);
    printf("\nread_sysfs_1()   buff[num_read_bytes-1] ASCII CODE: %d", buff[num_read_bytes-1]);
    printf("\nread_sysfs_1()   FINISHED");
#endif   // DEBUG

    return num_read_bytes;
}

ssize_t read_sysfs_2(const struct sysfs_stat rawdata_info, const int rawdata_fd) {

    ssize_t num_read_bytes;
    const int max_strlen1 = rawdata_info.max_strlen[0];
    const int max_strlen2 = rawdata_info.max_strlen[1];
    const int sysfs_fd1 = rawdata_info.sysfs_fd[0];
    const int sysfs_fd2 = rawdata_info.sysfs_fd[1];
    int i;
    char buff[128];

#ifdef DEBUG
    printf("\nread_sysfs_2()   START");
#endif   // DEBUG

    lseek(sysfs_fd1, 0, SEEK_SET);
    num_read_bytes = read(sysfs_fd1, buff, max_strlen1);

#ifdef DEBUG
    if(num_read_bytes == -1)
        printf("\nERROR in read_sysfs_1(): %s", strerror(errno));
#endif   // DEBUG

    if(buff[0] == '1') {
        lseek(sysfs_fd2, 0, SEEK_SET);
        num_read_bytes = read(sysfs_fd2, buff, max_strlen2);

#ifdef DEBUG
        if(num_read_bytes == -1)
            printf("\nERROR in read_sysfs_1(): %s", strerror(errno));
#endif   // DEBUG

        if(buff[num_read_bytes-1] == '\n' || buff[num_read_bytes-1] == EOF || buff[num_read_bytes-1] == ' ') {
            buff[num_read_bytes-1] = '\0';
            --num_read_bytes;
        }
    }
    else {
        strcpy(buff, "OFFLINE");
        num_read_bytes = strlen("OFFLINE");
    }

    for(i=0; i<(max_strlen2 - num_read_bytes ); i++)
        write(rawdata_fd, " ", 1);
    write(rawdata_fd, buff, num_read_bytes);

#ifdef DEBUG
    printf("\nread_sysfs_2()   column_name: %s", rawdata_info.column_name);
    printf("\nread_sysfs_2()   num_read_bytes: %ld", num_read_bytes);
    printf("\nread_sysfs_2()   max_strlen1: %d", max_strlen1);
    printf("\nread_sysfs_2()   max_strlen2: %d", max_strlen2);
    printf("\nread_sysfs_2()   sysfs_fd1: %d", sysfs_fd1);
    printf("\nread_sysfs_2()   sysfs_fd2: %d", sysfs_fd2);
    printf("\nread_sysfs_2()   buff: %s", buff);
    printf("\nread_sysfs_2()   buff[num_read_bytes-1] ASCII CODE: %d", buff[num_read_bytes-1]);
    printf("\nread_sysfs_2()   FINISHED");
#endif   // DEBUG

    return num_read_bytes;
}
*/

/*
ssize_t rawdata_to_stat_1(const struct sysfs_stat rawdata_info, const int rawdata_fd, const int stat_fd) {

    ssize_t num_read_bytes;
    const char *stat_format = rawdata_info.stat_format;
    const int max_strlen = rawdata_info.max_strlen[0];

    char rawdata_buff[256], stat_buff[256];
    int buff_len;

#ifdef DEBUG
    printf("\nrawdata_to_stat_1()   START");
#endif   // DEBUG

    num_read_bytes = read(rawdata_fd, rawdata_buff, max_strlen);

    // Two whitespace between columns
    write(stat_fd, "  ", 2);

    if(num_read_bytes > 0) {
        rawdata_buff[num_read_bytes] = '\0';
        buff_len = snprintf(stat_buff, 256, stat_format, max_strlen, rawdata_buff);
        write(stat_fd, WHITESPACE, rawdata_info.column_width - buff_len);
        write(stat_fd, stat_buff, buff_len);
    }

#ifdef DEBUG
    printf("\nrawdata_to_stat_1()   rawdata_fd: %d", rawdata_fd);
    if(num_read_bytes == -1) {
        printf("\nERROR in rawdata_to_stat_1(): %s", strerror(errno));
    }
    printf("\nrawdata_to_stat_1()   FINISHED");
#endif   // DEBUG

    return num_read_bytes;
}

ssize_t rawdata_to_stat_util(const struct sysfs_stat rawdata_info, const int rawdata_fd, const int stat_fd) {

    ssize_t num_read_bytes;
    const int max_strlen = rawdata_info.max_strlen[0];

    char rawdata_buff[256], rawdata_buff2[2], stat_buff[256];
    int buff_len;
    int percentage_int;

#ifdef DEBUG
    printf("\nrawdata_to_stat_util()   START");
#endif   // DEBUG

    num_read_bytes = read(rawdata_fd, rawdata_buff, max_strlen);

    // Two whitespace between columns
    write(stat_fd, "  ", 2);

    if(num_read_bytes > 0) {
        rawdata_buff[num_read_bytes] = '\0';
        strcpy(rawdata_buff2, &rawdata_buff[num_read_bytes-1]);
        rawdata_buff[num_read_bytes-1] = '\0';

        percentage_int = atoi(rawdata_buff);

        if(percentage_int == 100)
            buff_len = snprintf(stat_buff, 256, "100");
        else
            buff_len = snprintf(stat_buff, 256, "%02d.%s", percentage_int, rawdata_buff2);
        write(stat_fd, WHITESPACE, rawdata_info.column_width - buff_len);
        write(stat_fd, stat_buff, buff_len);
    }

#ifdef DEBUG
    printf("\nrawdata_to_stat_util()   rawdata_fd: %d", rawdata_fd);
    if(num_read_bytes == -1) {
        printf("\nERROR in rawdata_to_stat_util(): %s", strerror(errno));
    }
    printf("\nrawdata_to_stat_util()   FINISHED");
#endif   // DEBUG

    return num_read_bytes;

}

ssize_t rawdata_to_stat_2(const struct sysfs_stat rawdata_info, const int rawdata_fd, const int stat_fd) {

    ssize_t num_read_bytes;
    const char *stat_format = rawdata_info.stat_format;
    const int max_strlen = rawdata_info.max_strlen[1];

    char rawdata_buff[256], stat_buff[256];
    int buff_len;

#ifdef DEBUG
        printf("\nrawdata_to_stat_2()   START");
#endif   // DEBUG

    num_read_bytes = read(rawdata_fd, rawdata_buff, max_strlen);

    // Two whitespace between columns
    write(stat_fd, "  ", 2);

    if(num_read_bytes > 0) {
        rawdata_buff[num_read_bytes] = '\0';
        buff_len = snprintf(stat_buff, 256, stat_format, max_strlen, rawdata_buff);

        if(strncmp(stat_buff, "OFFLINE", 7)) {
            strcat(stat_buff, "kHz");
            buff_len += 3;
        }

        write(stat_fd, WHITESPACE, rawdata_info.column_width - buff_len);
        write(stat_fd, stat_buff, buff_len);
    }

#ifdef DEBUG
    printf("\nrawdata_to_stat_2()   column_name: %s", rawdata_info.column_name);
    printf("\nrawdata_to_stat_2()   rawdata_fd: %d", rawdata_fd);
    printf("\nrawdata_to_stat_2()   stat_buff: %s", stat_buff);
    if(num_read_bytes == -1) {
        printf("\nERROR in rawdata_to_stat_2(): %s", strerror(errno));
    }
    printf("\nrawdata_to_stat_2()   FINISHED");
#endif   // DEBUG

    return num_read_bytes;
}
*/
