#
# export informations to child Makefiles
#
export CAFFE_HOME = $(HOME)/caffe
export POWER_MEASUREMENT_HOME = $(shell pwd)
export POWER_MEASUREMENT_SCRIPT_HOME = $(POWER_MEASUREMENT_HOME)/script
export CFLAGS := -Wall -DNDEBUG -DTRACE_CPU -DTRACE_DDR

# Uncomment if you trace caffe timestamp
# Note that some caffe apps do not give timestamp
CFLAGS += -DTRACE_CAFFE_TIMESTAMP

#TARGET_DEVICE := tx2
TARGET_PATH := $(shell pwd)
TARGET := power_measurement
DEBUG_TARGET := $(TARGET)_debug

all: $(TARGET)

$(TARGET): $(TARGET).o register_rawdata.o caffelog.o measurement_info.o \
	       mkdir_p.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean: FORCE
	-rm $(TARGET) *.o *.txt

.PHONY: check check-intro check-cifar10 check-mnist
check: $(TARGET) check-intro check-cifar10 check-mnist
	@echo "Finished ALL Selftesting"

check-intro: FORCE
	@echo "\n"
	@echo "Start Selftesting with:   * CIFAR-10"
	@echo "                          * MNIST"

#
# Check with CIFAR-10 dataset
#
CAFFE_COMMAND_CIFAR10 := $(CAFFE_HOME)/build/tools/caffe test -model $(CAFFE_HOME)/examples/cifar10/cifar10_full_train_test.prototxt -weights $(CAFFE_HOME)/examples/cifar10/cifar10_full_iter_70000.caffemodel.h5 -gpu all
EXP_RESULT_PATH_CIFAR10 :=  $(POWER_MEASUREMENT_HOME)/exp_result/cifar-10

check-cifar10: $(TARGET)
	@echo "\n** Start selftesting with CIFAR-10\n"
	cd $(CAFFE_HOME); $(TARGET_PATH)/$(TARGET) -c gpu -f $(EXP_RESULT_PATH_CIFAR10)/cifar10_gpu_power.txt -i 2000 $(CAFFE_COMMAND_CIFAR10)
	@echo "\n** Finish selftesting with CIFAR-10\n"

#
# Check with MNIST dataset and LeNet network
#
CAFFE_COMMAND_MNIST := $(CAFFE_HOME)/build/tools/caffe test -model $(CAFFE_HOME)/examples/mnist/lenet_train_test.prototxt -weights $(CAFFE_HOME)/examples/mnist/lenet_iter_10000.caffemodel -gpu all
EXP_RESULT_PATH_MNIST :=  $(POWER_MEASUREMENT_HOME)/exp_result/mnist

check-mnist: $(TARGET)
	@echo "\n** Start selftesting with MNIST\n"
	cd $(CAFFE_HOME); $(TARGET_PATH)/$(TARGET) -c gpu -f $(EXP_RESULT_PATH_MNIST)/mnist_gpu_power.txt -i 2000 $(CAFFE_COMMAND_MNIST)
	@echo "\n** Finish selftesting with MNIST\n"

check-plot: check-cifar10 check-mnist
	@echo "\n** Drawing a plot for selftesting result with CIFAR-10\n"
	gnuplot -c script/plot/draw_single.plot $(EXP_RESULT_PATH_CIFAR10)/cifar10_gpu_power.txt $(EXP_RESULT_PATH_CIFAR10)/cifar10_plot.png CIFAR-10
	@echo "\n** Drawing a plot for selftesting result with MNIST\n"
	gnuplot -c script/plot/draw_single.plot $(EXP_RESULT_PATH_MNIST)/mnist_gpu_power.txt $(EXP_RESULT_PATH_MNIST)/mnist_plot.png MNIST

debug: CFLAGS += -g -DDEBUG -UNDEBUG
debug: $(TARGET)
	@echo "\nSTART DEBUGGING"
	@echo "\n** Start selftesting with CIFAR-10 with DEBUG VERSION\n"
	@if [ ! -d $(EXP_RESULT_PATH_CIFAR10) ]; then mkdir -p $(EXP_RESULT_PATH_CIFAR10); fi;
	cd $(CAFFE_HOME); $(TARGET_PATH)/$(TARGET) -c gpu -f $(EXP_RESULT_PATH_CIFAR10)/cifar10_gpu_power-debug.txt $(CAFFE_COMMAND_CIFAR10)
	@echo "\n** Finish selftesting with CIFAR-10 with DEBUG VERSION\n"
	@echo "\n** Start selftesting with MNIST with DEBUG VERSION\n"
	@if [ ! -d $(EXP_RESULT_PATH_MNIST) ]; then mkdir -p $(EXP_RESULT_PATH_MNIST); fi;
	cd $(CAFFE_HOME); $(TARGET_PATH)/$(TARGET) -c gpu -f $(EXP_RESULT_PATH_MNIST)/mnist_gpu_power-debug.txt $(CAFFE_COMMAND_MNIST)
	@echo "\n** Finish selftesting with MNIST with DEBUG VERSION\n"
	@echo "\nFINISH DEBUGGING"

# To prevent make from searching for an implicit rule
.PHONY: FORCE
FORCE:
