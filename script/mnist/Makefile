# Original MNIST Test Images and Labels
MNIST_TEST_IMAGES=t10k-images-idx3-ubyte
MNIST_TEST_LABELS=t10k-labels-idx1-ubyte

TARGET=process_mnist
$(TARGET): process_mnist.o parse_mnist.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: check
check: process_mnist
	./$(TARGET) -p -i $(MNIST_TEST_IMAGES) -l $(MNIST_TEST_LABELS)

.PHONY: clean
clean: FORCE
	-rm $(TARGET) *.o

# To prevent make from searching for an implicit rule
.PHONY: FORCE
FORCE:
