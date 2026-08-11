# Makefile —— 用内核的 Kbuild 系统编译外部模块
# obj-m: 把 hello.o 编成可加载模块 hello.ko（m = module）
obj-m += hello.o

# KERNEL_DIR: 指向当前运行内核的编译目录（headers 装好就有）
KERNEL_DIR := /lib/modules/$(shell uname -r)/build

# 所有模块编译统一走这条，绝不要手写 gcc -c
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
