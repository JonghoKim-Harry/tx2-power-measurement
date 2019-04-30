#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "parse_mnist.h"

int main(int argc, char *argv[]) {

    int fd;
    mnist_image_struct mnist_images;
    mnist_label_struct mnist_labels;

    fd = open(argv[2], O_RDONLY);

    if(!strcmp(argv[1], "-i")) {

        get_mnist_image_meta(fd, &mnist_images);
        print_mnist_image_meta(mnist_images);
    }

    if(!strcmp(argv[1], "-l")) {

        get_mnist_label_meta(fd, &mnist_labels);
        print_mnist_label_meta(mnist_labels);
    }

    close(fd);

    return 0;
}
