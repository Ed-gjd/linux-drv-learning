// SPDX-License-Identifier: GPL-2.0-only
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>

#define VGA_BASE 0xB8000
#define MAP_SIZE 0x1000

static int __init mmio_init(void)
{
	void __iomem *vga;

	pr_info("mmio: ioremap(0x%x, 0x%x)\n", VGA_BASE, MAP_SIZE);
	vga = ioremap(VGA_BASE, MAP_SIZE);
	if (!vga) {
		pr_err("mmio: ioremap 失败\n");
		return -ENOMEM;
	}

	/* %px 故意用之：教学要看真实映射地址（生产代码请用 %p 哈希地址） */
	pr_info("mmio: 映射成功，可访问地址=%px\n", vga);
	pr_info("mmio: readl 读显存前3个字:\n");
	pr_info("mmio:   [0]=0x%08x  [1]=0x%08x  [2]=0x%08x\n",
		readl(vga), readl(vga + 4), readl(vga + 8));

	iounmap(vga);
	return 0;
}

static void __exit mmio_exit(void)
{
	pr_info("mmio: 卸载\n");
}

module_init(mmio_init);
module_exit(mmio_exit);
MODULE_LICENSE("GPL");
