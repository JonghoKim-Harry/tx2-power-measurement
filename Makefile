#
# export informations to child Makefiles
#
export CAFFE_HOME = $(HOME)/caffe
export POWER_MEASUREMENT_HOME = $(shell pwd)
export POWER_MEASUREMENT_SCRIPT_HOME = $(POWER_MEASUREMENT_HOME)/script
export CFLAGS := -Wall -DNDEBUG

# Uncomment if you trace caffe timestamp
# Note that some caffe apps do not give timestamp
CFLAGS += -DTRACE_CAFFE_TIMESTAMP

#TARGET_DEVICE := tx2
TARGET_PATH := $(shell pwd)
TARGET := power_measurement
DEBUG_TARGET := $(TARGET)_debug

.PHONY: all
all: $(TARGET)

HEADERS := constants.h \
	       default_values.h \
		   enhanced_shcmd.h \
		   log_to_stat.h \
		   measurement_info.h \
		   parse_caffelog.h \
		   rawdata_to_powerlog.h \
		   stat.h \
		   summary.h \
		   tx2_sysfs_power.h

OBJECTS := measurement_info.o \
           rawdata_to_powerlog.o \
		   summary.o \
	       log_to_stat.o \
		   stat.o \
		   parse_caffelog.o \
		   enhanced_shcmd.o \
		   constants.o

SUBDIR_OBJECTS := governor/governor.o \
	              governor/ondemand8050.o \
				  runtime/collect_rawdata.o

$(TARGET): $(SUBDIR_OBJECTS) $(TARGET).o $(OBJECTS) $(HEADERS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean: FORCE
	-rm $(TARGET) *.o *.txt governor/*.o runtime/*.o

.PHONY: check check-intro check-lenet check-cifar10 check-alexnet
check: $(TARGET) check-intro check-lenet check-cifar10 check-alexnet
	@echo "Finished ALL Selftesting"

check-intro: FORCE
	@echo "\n"
	@echo "Start Selftesting with:   * LeNet"
	@echo "                          * CIFAR-10 Network"
	@echo "                          * AlexNet"

#
# Check with LeNet and MNIST dataset
#
CAFFE_COMMAND_LENET := $(CAFFE_HOME)/build/tools/caffe test -model $(CAFFE_HOME)/examples/lenet/lenet_train_test.prototxt -weights $(CAFFE_HOME)/examples/mnist/lenet_iter_10000.caffemodel -gpu all
EXP_RESULT_PATH_LENET :=  $(POWER_MEASUREMENT_HOME)/exp_result/lenet

check-lenet: $(TARGET)
	@echo "\n** Start selftesting with LeNet\n"
	cd $(CAFFE_HOME); $(TARGET_PATH)/$(TARGET) -c gpu -f $(EXP_RESULT_PATH_LENET)/lenet_gpu_power.txt -i 10000 $(CAFFE_COMMAND_LENET)
	@echo "\n** Finish selftesting with LeNet\n"

#
# Check with CIFAR-10 dataset
#
CAFFE_COMMAND_CIFAR10 := $(CAFFE_HOME)/build/tools/caffe test -model $(CAFFE_HOME)/examples/cifar10/cifar10_full_train_test.prototxt -weights $(CAFFE_HOME)/examples/cifar10/cifar10_full_iter_70000.caffemodel.h5 -gpu all
EXP_RESULT_PATH_CIFAR10 :=  $(POWER_MEASUREMENT_HOME)/exp_result/cifar-10

check-cifar10: $(TARGET)
	@echo "\n** Start selftesting with CIFAR-10\n"
	cd $(CAFFE_HOME); $(TARGET_PATH)/$(TARGET) -c gpu -f $(EXP_RESULT_PATH_CIFAR10)/cifar10_gpu_power.txt -i 10000 $(CAFFE_COMMAND_CIFAR10)
	@echo "\n** Finish selftesting with CIFAR-10\n"

#
# Check with AlexNet and ImageNet'2012 dataset
#
CAFFE_COMMAND_ALEXNET := $(CAFFE_HOME)/build/tools/caffe test -model $(CAFFE_HOME)/models/bvlc_alexnet/train_val.prototxt -weights $(CAFFE_HOME)/models/bvlc_alexnet/bvlc_alexnet.caffemodel -gpu all
EXP_RESULT_PATH_ALEXNET :=  $(POWER_MEASUREMENT_HOME)/exp_result/alexnet
check-alexnet: $(TARGET)
	@echo "\n** Start selftesting with AlexNet\n"
	cd $(CAFFE_HOME); $(TARGET_PATH)/$(TARGET) -c gpu -f $(EXP_RESULT_PATH_ALEXNET)/alexnet_gpu_power.txt -i 10000 $(CAFFE_COMMAND_ALEXNET)
	@echo "\n** Finish selftesting with AlexNet\n"

check-plot: check-cifar10 check-lenet check-alexnet
	@echo "\n** Drawing a plot for selftesting result with CIFAR-10\n"
	gnuplot -c script/plot/draw_single.plot $(EXP_RESULT_PATH_CIFAR10)/cifar10_gpu_power.txt $(EXP_RESULT_PATH_CIFAR10)/cifar10_plot.png CIFAR-10
	@echo "\n** Drawing a plot for selftesting result with LeNet\n"
	gnuplot -c script/plot/draw_single.plot $(EXP_RESULT_PATH_LENET)/lenet_gpu_power.txt $(EXP_RESULT_PATH_LENET)/lenet_plot.png LeNet

debug: CFLAGS += -g -DDEBUG -UNDEBUG
debug: $(TARGET)

# To prevent make from searching for an implicit rule
.PHONY: FORCE
FORCE:
