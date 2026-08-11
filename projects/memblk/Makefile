# memblk：内存虚拟块设备
obj-m += memblk.o

KERNEL_DIR := /lib/modules/$(shell uname -r)/build
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
