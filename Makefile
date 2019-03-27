#
# export informations to child Makefiles
#
export CAFFE_HOME = $(HOME)/caffe
export POWER_MEASUREMENT_HOME = $(shell pwd)
export POWER_MEASUREMENT_SCRIPT_HOME = $(POWER_MEASUREMENT_HOME)/script
export CFLAGS := -Wall -DTRACE_CPU
export LIBS := -lpthread

TARGET_DEVICE := tx2
TARGET_PATH := $(shell pwd)
TARGET := $(TARGET_DEVICE)_power_measurement
DEBUG_TARGET := $(TARGET)_debug

all: $(TARGET)

$(TARGET): $(TARGET).o sysfs_to_stat.o
	$(CC) $(CFLAGS) $? -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean: FORCE
	-rm $(TARGET) *.o *.txt

.PHONY: check check-intro check-cifar10 check-mnist
check: $(TARGET) check-intro check-cifar10 check-mnist
	@echo "Finished ALL Selftesting"

check-intro: FORCE
	@echo "Start Selftesting with:   * CIFAR-10"
	@echo "                          * MNIST"

#
# Check with CIFAR-10 dataset
#
CAFFE_COMMAND_CIFAR10 := $(CAFFE_HOME)/build/tools/caffe test -model $(CAFFE_HOME)/examples/cifar10/cifar10_full_train_test.prototxt -weights $(CAFFE_HOME)/examples/cifar10/cifar10_full_iter_70000.caffemodel.h5 -gpu all
TEST_RESULT_PATH_CIFAR10 :=  $(POWER_MEASUREMENT_HOME)/test_result/cifar-10

check-cifar10: $(TARGET)
	@echo "\n** Start selftesting with CIFAR-10\n"
	@if [ ! -d $(TEST_RESULT_PATH_CIFAR10) ]; then mkdir -p $(TEST_RESULT_PATH_CIFAR10); fi;
	cd $(CAFFE_HOME); $(TARGET_PATH)/$(TARGET) -c gpu -f $(TEST_RESULT_PATH_CIFAR10)/cifar10_gpu_power.txt $(CAFFE_COMMAND_CIFAR10) 1> $(TEST_RESULT_PATH_CIFAR10)/cifar10_powerlog.txt 2> $(TEST_RESULT_PATH_CIFAR10)/cifar10_caffelog.txt
	@echo "\n** Finish selftesting with CIFAR-10\n"

#
# Check with MNIST dataset and LeNet network
#
CAFFE_COMMAND_MNIST := $(CAFFE_HOME)/build/tools/caffe test -model $(CAFFE_HOME)/examples/mnist/lenet_train_test.prototxt -weights $(CAFFE_HOME)/examples/mnist/lenet_iter_10000.caffemodel -gpu all
TEST_RESULT_PATH_MNIST :=  $(POWER_MEASUREMENT_HOME)/test_result/mnist

check-mnist: $(TARGET)
	@echo "\n** Start selftesting with MNIST\n"
	@if [ ! -d $(TEST_RESULT_PATH_MNIST) ]; then mkdir -p $(TEST_RESULT_PATH_MNIST); fi;
	cd $(CAFFE_HOME); $(TARGET_PATH)/$(TARGET) -c gpu -f $(TEST_RESULT_PATH_MNIST)/mnist_gpu_power.txt $(CAFFE_COMMAND_MNIST) 1> $(TEST_RESULT_PATH_MNIST)/mnist_powerlog.txt 2> $(TEST_RESULT_PATH_MNIST)/mnist_caffelog.txt
	@echo "\n** Finish selftesting with MNIST\n"

check-plot: check-cifar10 check-mnist
	@echo "\n** Drawing a plot for selftesting result with CIFAR-10\n"
	gnuplot -c script/plot/draw_single.plot $(TEST_RESULT_PATH_CIFAR10)/cifar10_gpu_power.txt $(TEST_RESULT_PATH_CIFAR10)/cifar10_plot.png CIFAR-10
	@echo "\n** Drawing a plot for selftesting result with MNIST\n"
	gnuplot -c script/plot/draw_single.plot $(TEST_RESULT_PATH_MNIST)/mnist_gpu_power.txt $(TEST_RESULT_PATH_MNIST)/mnist_plot.png MNIST

debug: CFLAGS += -DDEBUG
debug: $(TARGET)
	@echo "\nSTART DEBUGGING"
	@echo "\n** Start selftesting with CIFAR-10 with DEBUG VERSION\n"
	@if [ ! -d $(TEST_RESULT_PATH_CIFAR10) ]; then mkdir -p $(TEST_RESULT_PATH_CIFAR10); fi;
	cd $(CAFFE_HOME); $(TARGET_PATH)/$(TARGET) -c gpu -f $(TEST_RESULT_PATH_CIFAR10)/cifar10_gpu_power.debug.txt $(CAFFE_COMMAND_CIFAR10) 1> $(TEST_RESULT_PATH_CIFAR10)/cifar10_powerlog.debug.txt 2> $(TEST_RESULT_PATH_CIFAR10)/cifar10_caffelog.txt
	@echo "\n** Finish selftesting with CIFAR-10 with DEBUG VERSION\n"
	@echo "\n** Start selftesting with MNIST with DEBUG VERSION\n"
	@if [ ! -d $(TEST_RESULT_PATH_MNIST) ]; then mkdir -p $(TEST_RESULT_PATH_MNIST); fi;
	cd $(CAFFE_HOME); $(TARGET_PATH)/$(TARGET) -c gpu -f $(TEST_RESULT_PATH_MNIST)/mnist_gpu_power.debug.txt $(CAFFE_COMMAND_MNIST) 1> $(TEST_RESULT_PATH_MNIST)/mnist_powerlog.debug.txt 2> $(TEST_RESULT_PATH_MNIST)/mnist_caffelog.txt
	@echo "\n** Finish selftesting with MNIST with DEBUG VERSION\n"
	@echo "\nFINISH DEBUGGING"

# To prevent make from searching for an implicit rule
.PHONY: FORCE
FORCE:
