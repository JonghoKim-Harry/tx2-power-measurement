#ifndef PARSE_MNIST_H
#define PARSE_MNIST_H

#include <stdint.h>

typedef struct mnist_image_struct {

    int32_t magic_number;
    int32_t num_images;
    int32_t resolution_height;
    int32_t resolution_width;
    uint8_t **images;
} mnist_image_struct;

void get_mnist_image_meta(int fd, struct mnist_image_struct *mnist_images);
void print_mnist_image_meta(const struct mnist_image_struct mnist_images);
void parse_mnist_image_file(int fd, struct mnist_image_struct *mnist_images);

typedef struct mnist_label_struct {

    int32_t magic_number;
    int32_t num_labels;
    uint8_t *labels;
} mnist_label_struct;

void get_mnist_label_meta(int fd, struct mnist_label_struct *mnist_labels);
void print_mnist_label_meta(const struct mnist_label_struct mnist_labels);
void parse_mnist_label_file(int fd, struct mnist_label_struct *mnist_labels);

#endif   // PARSE_MNIST_H
