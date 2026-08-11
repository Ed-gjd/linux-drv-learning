# linux-drv-learning 根 Makefile
# 用法：
#   make          全编所有课程模块 + 成品驱动
#   make list     列出会编译的子目录
#   make clean    清理编译产物
#
# 编译全部走内核 Kbuild：make -C /lib/modules/$(uname -r)/build M=<dir> modules
# 需要先安装 headers：sudo apt install linux-headers-$(uname -r)
#
# 例外目录：
#   lessons/07-device-tree  只有 .dts 设备树源文件，无需编译

KERNEL_DIR := /lib/modules/$(shell uname -r)/build

SUBDIRS := \
	lessons/01-hello \
	lessons/02-module-params \
	lessons/03-char-dev \
	lessons/04-timer \
	lessons/05-list \
	lessons/06-platform \
	lessons/08-workqueue-irq \
	lessons/09-mmio \
	lessons/10-vsensor \
	lessons/11-mem-alloc \
	lessons/12-kthread \
	lessons/13-block-dev \
	projects/vsensor-final \
	projects/memblk

all:
	@for d in $(SUBDIRS); do \
		echo "=====> 编译 $$d"; \
		$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR)/$$d modules || exit 1; \
	done
	@echo ""
	@echo "✅ 全部模块编译完成（块设备项目见 README 单独说明）"

clean:
	@for d in $(SUBDIRS); do \
		$(MAKE) -C $(KERNEL_DIR) M=$(CURDIR)/$$d clean 2>/dev/null; \
	done
	@echo "已清理编译产物"

list:
	@echo "以下子目录会被 make 编译："
	@for d in $(SUBDIRS); do echo "  - $$d"; done

.PHONY: all clean list
