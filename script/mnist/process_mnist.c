#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "parse_mnist.h"

#define REORDER_STYLE_MAXLEN   50
#define FILENAME_MAXLEN      1024
#define AVAILABLE_OPTIONS "-"   "hi:j:l:m:pr:"
#define HELP_FIRST_COLUMN_WIDTH   30

void help() {

    printf("\nUsage: ./process_mnist [options]\n");
    printf("\n\t%-*s%s", HELP_FIRST_COLUMN_WIDTH, "-h",
            "Print help message");
    printf("\n\t%-*s%s", HELP_FIRST_COLUMN_WIDTH, "-i <mnist image file>",
            "Process a MNIST image file");
    printf("\n\t%-*s%s", HELP_FIRST_COLUMN_WIDTH, "-j <mnist image file>",
            "Output MNIST image file name");
    printf("\n\t%-*s%s", HELP_FIRST_COLUMN_WIDTH, "-l <mnist label file>",
            "Process a MNIST label file");
    printf("\n\t%-*s%s", HELP_FIRST_COLUMN_WIDTH, "-m <mnist label file>",
            "Output MNIST label file name");
    printf("\n\t%-*s%s", HELP_FIRST_COLUMN_WIDTH, "-p",
            "Print metadata of MNIST data");
    printf("\n\t%-*s%s", HELP_FIRST_COLUMN_WIDTH, "-r [<reorder style name>]",
            "Reorder images or/and labels");
    printf("\n");
}

int main(int argc, char *argv[]) {

    // For argument processing
    int option;
    int iflag = 0, jflag = 0, lflag = 0, mflag = 0, pflag = 0, rflag = 0;
    char imagein_filename[FILENAME_MAXLEN], labelin_filename[FILENAME_MAXLEN];
    char imageout_filename[FILENAME_MAXLEN], labelout_filename[FILENAME_MAXLEN];
    char reorder_style[REORDER_STYLE_MAXLEN];

    // For parsing MNIST dataset
    int fd;
    mnist_image_struct mnist_images;
    mnist_label_struct mnist_labels;

    while((option = getopt(argc, argv, AVAILABLE_OPTIONS)) != -1) {
        switch(option) {
        case 'h':   // Option -h without argument
            help();
            exit(0);

        case 'i':   // Option -i with required argument
            iflag = 1;
            strcpy(imagein_filename, optarg);
            break;

        case 'j':   // Option -j with required argument
            jflag = 1;
            strcpy(imageout_filename, optarg);
            break;

        case 'l':   // Option -l with required argument
            lflag = 1;
            strcpy(labelin_filename, optarg);
            break;

        case 'm':   // Option -m with required argument
            mflag = 1;
            strcpy(labelout_filename, optarg);
            break;

        case 'p':   // Option -p without argument
            pflag = 1;
            break;

        case 'r':   // Option -r with required argument
            rflag = 1;
            strcpy(reorder_style, optarg);
            break;

        case 1:   // The first non-optional argument
            // Ignore the argument
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

    /*
     *  Start job
     */
    if(iflag) {
    
        fd = open(imagein_filename, O_RDONLY);
        parse_mnist_image_file(fd, &mnist_images);
        if(pflag) {
            printf("\nMNIST image file: %s", imagein_filename);
            print_mnist_image_meta(mnist_images);
        }

        if(rflag) {
            if(!strcmp(reorder_style, "reorder26500125"))
                reorder26500125_mnist_images(&mnist_images);
        }

        close(fd);
    }

    if(jflag) {
        fd = generate_mnist_image_file(imageout_filename, mnist_images);
        close(fd);
    }

    if(lflag) {

        fd = open(labelin_filename, O_RDONLY);
        parse_mnist_label_file(fd, &mnist_labels);
        if(pflag) {
            printf("\nMNIST label file: %s", labelin_filename);
            print_mnist_label_meta(mnist_labels);
        }

        if(rflag) {
            if(!strcmp(reorder_style, "reorder26500125"))
                reorder26500125_mnist_labels(&mnist_labels);
        }

        close(fd);
    }

    if(mflag) {
        fd = generate_mnist_label_file(labelout_filename, mnist_labels);
        close(fd);
    }

    return 0;
}
