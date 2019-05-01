#ifndef PARSE_MNIST_H
#define PARSE_MNIST_H

// See: http://yann.lecun.com/exdb/mnist/

#include <stdint.h>

typedef struct mnist_image_struct {

    int32_t magic_number;
    int32_t num_images;
    int32_t resolution_height;
    int32_t resolution_width;
    uint8_t *images;
} mnist_image_struct;

void print_mnist_image_meta(const mnist_image_struct mnist_images);
void parse_mnist_image_file(int fd, mnist_image_struct *mnist_images);
int generate_mnist_image_file(
    const char *filename,
    const mnist_image_struct mnist_images
);

typedef struct mnist_label_struct {

    int32_t magic_number;
    int32_t num_labels;
    uint8_t *labels;
} mnist_label_struct;

void print_mnist_label_meta(const mnist_label_struct mnist_labels);
void parse_mnist_label_file(int fd, mnist_label_struct *mnist_labels);
int generate_mnist_label_file(
    const char *filename,
    const mnist_label_struct mnist_labels
);


/*
 *   Reorder Functions
 */
void reorder26500125_mnist_images(mnist_image_struct *mnist_images);
void reorder26500125_mnist_labels(mnist_label_struct *mnist_labels);

#endif   // PARSE_MNIST_H
