TARGET=process_mnist
$(TARGET): process_mnist.o parse_mnist.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Original MNIST Test Images and Labels
MNIST_TEST_IMAGES=t10k-images-idx3-ubyte
MNIST_TEST_LABELS=t10k-labels-idx1-ubyte

.PHONY: check check-identical check-reorder

check: check-identical

check-identical: $(TARGET)
	./$(TARGET) -p -i $(MNIST_TEST_IMAGES) -j image_mnist.dat -l $(MNIST_TEST_LABELS) -m label_mnist.dat
	-diff -sq $(MNIST_TEST_IMAGES) image_mnist.dat
	-diff -sq $(MNIST_TEST_LABELS) label_mnist.dat

check-reorder: $(TARGET)
	./$(TARGET) -p -r reorder26500125 -i $(MNIST_TEST_IMAGES) -j image_mnist_reorder26500125.dat -l $(MNIST_TEST_LABELS) -m label_mnist_reorder26500125.dat

.PHONY: clean
clean: FORCE
	-rm $(TARGET) *.o

# To prevent make from searching for an implicit rule
.PHONY: FORCE
FORCE:
