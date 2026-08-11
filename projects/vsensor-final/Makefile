# vsensor-final：虚拟传感器合格版
obj-m += vsensor.o

KERNEL_DIR := /lib/modules/$(shell uname -r)/build
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
