TARGET=process_mnist
$(TARGET): process_mnist.o parse_mnist.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Original MNIST Test Images and Labels
MNIST_TEST_IMAGES=t10k-images-idx3-ubyte
MNIST_TEST_LABELS=t10k-labels-idx1-ubyte

.PHONY: check check-image check-label
check: check-image check-label

check-image: $(TARGET)
	./$(TARGET) -p -i $(MNIST_TEST_IMAGES) -j mnist_image.dat
	-diff -sq $(MNIST_TEST_IMAGES) mnist_image.dat

check-label: $(TARGET)
	./$(TARGET) -p -l $(MNIST_TEST_LABELS) -m label.dat
	-diff -sq $(MNIST_TEST_LABELS) label.dat

.PHONY: clean
clean: FORCE
	-rm $(TARGET) *.o

# To prevent make from searching for an implicit rule
.PHONY: FORCE
FORCE:
