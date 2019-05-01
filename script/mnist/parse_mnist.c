#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include "parse_mnist.h"

#define MNIST_IMAGE_MAGIC_NUMBER   0x00000803
#define MNIST_LABEL_MAGIC_NUMBER   0x00000801
#define MAX_RESOLUTION                    784   // 28x28
#define REVERSE(x_ref)  x_ref = reverse32(x_ref);

// Reference: http://esr.ibiblio.org/?p=5095
#define IS_BIG_ENDIAN (*(uint16_t *)"\0\xff" < 0x100)

static int32_t reverse32(const int32_t bits) {

    int i;
    uint8_t byte1, byte2, byte3, byte4;
    int32_t reversed;

    byte1 = (bits & 0x000000ff);
    byte2 = (bits & 0x0000ff00) >>  8;
    byte3 = (bits & 0x00ff0000) >> 16;
    byte4 = (bits & 0xff000000) >> 24;

    reversed = 0;

    reversed |= (byte1 << 24);
    reversed |= (byte2 << 16);
    reversed |= (byte3 <<  8);
    reversed |=  byte4;

    return reversed;
}

static void get_mnist_image_meta(int fd, struct mnist_image_struct *mnist_images) {

    ssize_t read_bytes;
    char buff[4];

    lseek(fd, 0, SEEK_SET);

    // Magic Number
    read_bytes = read(fd, buff, 4);
    memcpy(&mnist_images->magic_number, buff, 4);
    if(!IS_BIG_ENDIAN)
        REVERSE(mnist_images->magic_number);

    if(mnist_images->magic_number != MNIST_IMAGE_MAGIC_NUMBER)
        perror("MAGIC NUMBER NOT MATCHED: NOT MNIST IMAGE");

    // Number of images
    read_bytes = read(fd, buff, 4);
    memcpy(&mnist_images->num_images, buff, 4);
    if(!IS_BIG_ENDIAN)
        REVERSE(mnist_images->num_images);

    // Resolution
    read_bytes = read(fd, buff, 4);
    memcpy(&mnist_images->resolution_height, buff, 4);
    if(!IS_BIG_ENDIAN)
        REVERSE(mnist_images->resolution_height);

    read_bytes = read(fd, buff, 4);
    memcpy(&mnist_images->resolution_width, buff, 4);
    if(!IS_BIG_ENDIAN)
        REVERSE(mnist_images->resolution_width);

    return;
}

void print_mnist_image_meta(const struct mnist_image_struct mnist_images) {

    printf("\nFormat: MNIST IMAGE");
    printf("\nNumber of images: %d", mnist_images.num_images);
    printf("\nResolution: %dx%d", mnist_images.resolution_width, mnist_images.resolution_height);
    printf("\n____\n");

    return;
}

void parse_mnist_image_file(int fd, struct mnist_image_struct *mnist_images) {

    int idx;
    int num_pixels_per_image;
    ssize_t read_bytes;
    char pixels[MAX_RESOLUTION];

    get_mnist_image_meta(fd, mnist_images);
    num_pixels_per_image = mnist_images->resolution_height * mnist_images->resolution_width;

    // Images
    mnist_images->images = malloc(mnist_images->num_images * sizeof(uint8_t *));

    for(idx=0; idx<mnist_images->num_images; ++idx) {

        mnist_images->images[idx] = malloc(num_pixels_per_image * sizeof(uint8_t));
        read_bytes = read(fd, pixels, num_pixels_per_image);
        if(read_bytes < 0)
            perror("read() error in parse_mnist_image_file()");
        memcpy(&mnist_images->images[idx], pixels, read_bytes);
    }
}

int generate_mnist_image_file(const char *filename,
                              const mnist_image_struct mnist_images) {

    // This function returns fd of generated file
    // If fail, this function returns -1
    int fd;
    int32_t val;
    char buff[4];
    int num_pixels_per_image;
    int idx;
    ssize_t written_bytes;

    fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);

    if(fd == -1)
        return -1;

    lseek(fd, 0, SEEK_SET);

    // Magic number
    val = mnist_images.magic_number;
    assert(val == reverse32(reverse32(val)));   // assert() for debug
    if(!IS_BIG_ENDIAN)
        REVERSE(val);
    memcpy(buff, &val, 4);
    write(fd, buff, 4);

    // Number of images
    val = mnist_images.num_images;
    if(!IS_BIG_ENDIAN)
        REVERSE(val);
    memcpy(buff, &val, 4);
    write(fd, buff, 4);

    // Resolution
    val = mnist_images.resolution_height;
    if(!IS_BIG_ENDIAN)
        REVERSE(val);
    memcpy(buff, &val, 4);
    write(fd, buff, 4);

    val = mnist_images.resolution_width;
    if(!IS_BIG_ENDIAN)
        REVERSE(val);
    memcpy(buff, &val, 4);
    write(fd, buff, 4);

    // Images
    num_pixels_per_image = mnist_images.resolution_width   *
                           mnist_images.resolution_height;

    for(idx=0; idx<mnist_images.num_images; idx++) {
        written_bytes = write(fd, &mnist_images.images[idx], num_pixels_per_image);
        if(written_bytes < 0)
            perror("write() error in generate_mnist_image_file()");
    }

    // Note that we do NOT close the generated file,
    // and just return the fd of the generated file
    return fd;
}

static void get_mnist_label_meta(int fd, struct mnist_label_struct *mnist_labels) {

    ssize_t read_bytes;
    char buff[4];

    lseek(fd, 0, SEEK_SET);

    // Magic Number
    read_bytes = read(fd, buff, 4);
    memcpy(&mnist_labels->magic_number, buff, 4);
    if(!IS_BIG_ENDIAN)
        REVERSE(mnist_labels->magic_number);

    if(mnist_labels->magic_number != MNIST_LABEL_MAGIC_NUMBER)
        perror("MAGIC NUMBER NOT MATCHED: NOT MNIST LABEL");

    // Number of labels
    read_bytes = read(fd, buff, 4);
    memcpy(&mnist_labels->num_labels, buff, 4);
    if(!IS_BIG_ENDIAN)
        REVERSE(mnist_labels->num_labels);

    return;
}

void print_mnist_label_meta(const struct mnist_label_struct mnist_labels) {

    printf("\nFormat: MNIST LABEL");
    printf("\nNumber of labels: %d", mnist_labels.num_labels);
    printf("\n____\n");

    return;
}

void parse_mnist_label_file(int fd, struct mnist_label_struct *mnist_labels) {

    int idx;
    ssize_t read_bytes;
    char label[1];

    get_mnist_label_meta(fd, mnist_labels);

    // Labels
    mnist_labels->labels = malloc(mnist_labels->num_labels * sizeof(uint8_t));

    for(idx=0; idx<mnist_labels->num_labels; ++idx) {
    
        read_bytes = read(fd, label, 1);
        if(read_bytes < 0)
            perror("read() error in parse_mnist_label_file()");

        memcpy(&mnist_labels->labels[idx], label, read_bytes);
    }
}

int generate_mnist_label_file(const char *filename,
                              const mnist_label_struct mnist_labels) {

    // This function returns fd of generated file
    // If fail, this function returns -1
    int fd;
    int32_t val;
    char buff[4];
    ssize_t written_bytes;
    int idx;

    fd = open(filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);

    if(fd == -1)
        return -1;

    lseek(fd, 0, SEEK_SET);

    // Magic number
    val = mnist_labels.magic_number;
    assert(val == reverse32(reverse32(val)));   // assert() for debug
    if(!IS_BIG_ENDIAN)
        REVERSE(val);
    memcpy(buff, &val, 4);
    write(fd, buff, 4);

    // Number of labels
    val = mnist_labels.num_labels;
    if(!IS_BIG_ENDIAN)
        REVERSE(val);
    memcpy(buff, &val, 4);
    write(fd, buff, 4);

    // Labels
    for(idx=0; idx<mnist_labels.num_labels; idx++) {

        memcpy(buff, &mnist_labels.labels[idx], 1);
        write(fd, buff, 1);
    }

    // Note that we do NOT close the generated file,
    // and just return the fd of the generated file
    return fd;
}
