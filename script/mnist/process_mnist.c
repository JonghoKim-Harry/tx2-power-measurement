#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "parse_mnist.h"

#define FILENAME_MAXLEN   1024
#define AVAILABLE_OPTIONS "-"   "hi:l:p"
#define HELP_FIRST_COLUMN_WIDTH   30

void help() {

    printf("\nUsage: ./process_mnist [options]\n");
    printf("\n\t%-*s%s", HELP_FIRST_COLUMN_WIDTH, "-h",
            "Print help message");
    printf("\n\t%-*s%s", HELP_FIRST_COLUMN_WIDTH, "-i <mnist image file>",
            "Process a MNIST image file");
    printf("\n\t%-*s%s", HELP_FIRST_COLUMN_WIDTH, "-l <mnist label file>",
            "Process a MNIST label file");
    printf("\n\t%-*s%s", HELP_FIRST_COLUMN_WIDTH, "-p",
            "Print metadata of MNIST data");
    printf("\n");
}

int main(int argc, char *argv[]) {

    // For argument processing
    int option;
    int iflag = 0, lflag = 0, pflag = 0;
    char image_filename[FILENAME_MAXLEN], label_filename[FILENAME_MAXLEN];

    // For parsing MNIST dataset
    int fd;
    mnist_image_struct mnist_images;
    mnist_label_struct mnist_labels;

    while((option = getopt(argc, argv, AVAILABLE_OPTIONS)) != -1) {
        switch(option) {
        case 'h':   // Option -h
            help();
            exit(0);

        case 'i':   // Option -i
            iflag = 1;
            strcpy(image_filename, optarg);
            break;

        case 'l':   // Option -l
            lflag = 1;
            strcpy(label_filename, optarg);
            break;

        case 'p':   // Option -p
            pflag = 1;
            break;

        case 1:   // The first non-optional argument
            optind--;
            break;

        case ':':   // Missing arguments
            fprintf(stderr, "\nMissing arguments for option %c", optopt);
            help();
            exit(-1);

        case '?':   // Invalid option
            fprintf(stderr, "\nInvalid option: %c\n", optopt);
            break;
        }
    }

    if(iflag) {
    
        fd = open(image_filename, O_RDONLY);
        get_mnist_image_meta(fd, &mnist_images);
        if(pflag) {
            printf("\nMNIST image file: %s", image_filename);
            print_mnist_image_meta(mnist_images);
        }
        close(fd);
    }

    if(lflag) {

        fd = open(label_filename, O_RDONLY);
        get_mnist_label_meta(fd, &mnist_labels);
        if(pflag) {
            printf("\nMNIST label file: %s", label_filename);
            print_mnist_label_meta(mnist_labels);
        }
        close(fd);
    }

    return 0;
}
