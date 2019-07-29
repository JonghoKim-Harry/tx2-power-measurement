#
# export informations to child Makefiles
#
export CAFFE_HOME = $(HOME)/caffe
export POWER_MEASUREMENT_HOME = $(shell pwd)
export POWER_MEASUREMENT_SCRIPT_HOME = $(POWER_MEASUREMENT_HOME)/script
export CFLAGS := -Wall -DNDEBUG -DTRACE_CPU -DTRACE_MEM

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
		   caffelog.h \
		   powerlog.h \
		   stat.h \
		   summary.h \
		   tx2_sysfs_power.h

OBJECTS := measurement_info.o \
           powerlog.o \
		   summary.o \
	       log_to_stat.o \
		   stat.o \
		   caffelog.o \
		   enhanced_shcmd.o \
		   constants.o

SUBDIR_OBJECTS := governor/governor.o \
	              governor/ondemand8050.o \
				  rawdata.o

$(TARGET): $(SUBDIR_OBJECTS) $(TARGET).o $(OBJECTS) $(HEADERS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean: FORCE
	-rm $(TARGET) *.o *.txt governor/*.o

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
EXP_RESULT_PATH_LENET :=  $(POWER_MEASUREMENT_HOME)/exp_result/lenet
LENET_SELFTEST_CMD :=  $(POWER_MEASUREMENT_HOME)/script/run/run_lenet-batch100.sh

check-lenet: $(TARGET)
	@echo "\n** Start selftesting with LeNet (default batch size: 100)\n"
	cd $(CAFFE_HOME); $(TARGET_PATH)/$(TARGET) -c gpu -f $(EXP_RESULT_PATH_LENET)/lenet_gpu_power.txt -i 10000 $(LENET_SELFTEST_CMD)
	@echo "\n** Finish selftesting with LeNet\n"

#
# Check with CIFAR-10 dataset
#
EXP_RESULT_PATH_CIFAR10 :=  $(POWER_MEASUREMENT_HOME)/exp_result/cifar10
CIFAR10_SELFTEST_CMD := $(POWER_MEASUREMENT_HOME)/script/run/run_cifar10-batch100.sh

check-cifar10: $(TARGET)
	@echo "\n** Start selftesting with CIFAR-10 (default batch size: 100)\n"
	cd $(CAFFE_HOME); $(TARGET_PATH)/$(TARGET) -c gpu -f $(EXP_RESULT_PATH_CIFAR10)/cifar10_gpu_power.txt -i 10000 $(CIFAR10_SELFTEST_CMD)
	@echo "\n** Finish selftesting with CIFAR-10\n"

#
# Check with AlexNet and ImageNet'2012 dataset
#
EXP_RESULT_PATH_ALEXNET :=  $(POWER_MEASUREMENT_HOME)/exp_result/alexnet
ALEXNET_SELFTEST_CMD :=  $(POWER_MEASUREMENT_HOME)/script/run/run_alexnet-batch50.sh

check-alexnet: $(TARGET)
	@echo "\n** Start selftesting with AlexNet (default batch size: 50)\n"
	cd $(CAFFE_HOME); $(TARGET_PATH)/$(TARGET) -c gpu -f $(EXP_RESULT_PATH_ALEXNET)/alexnet_gpu_power.txt -i 10000 $(ALEXNET_SELFTEST_CMD)
	@echo "\n** Finish selftesting with AlexNet\n"

#
# Check with GoogLeNet and ImageNet'2012 dataset
#
EXP_RESULT_PATH_GOOGLENET :=  $(POWER_MEASUREMENT_HOME)/exp_result/googlenet
GOOGLENET_SELFTEST_CMD :=  $(POWER_MEASUREMENT_HOME)/script/run/run_googlenet-batch50.sh
check-googlenet: $(TARGET)
	@echo "\n** Start selftesting with GoogLeNet (default batch size: 50)\n"
	cd $(CAFFE_HOME); $(TARGET_PATH)/$(TARGET) -c gpu -f $(EXP_RESULT_PATH_GOOGLENET)/googlenet_gpu_power.txt -i 10000 $(GOOGLENET_SELFTEST_CMD)
	@echo "\n** Finish selftesting with GoogLeNet\n"


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
