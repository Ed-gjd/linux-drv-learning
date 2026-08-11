# 06-platform：platform 驱动 + 驱动/设备分离
# 三个模块（互相独立，展示"驱动注册 vs 设备注册"两种视角）
obj-m += platform_demo.o    # 第6课：platform 驱动 + 同名字设备，顺序无关触发 probe
obj-m += driver_mod.o       # 第7课：只注册"驱动"（能力）
obj-m += device_mod.o       # 第7课：只注册"设备"（实体）

KERNEL_DIR := /lib/modules/$(shell uname -r)/build
all:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KERNEL_DIR) M=$(PWD) clean
