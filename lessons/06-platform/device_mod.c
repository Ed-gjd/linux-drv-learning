// SPDX-License-Identifier: GPL-2.0-only
#include <linux/platform_device.h>
static struct platform_device *dev_pdev;
static int __init dev_init(void)
{
	pr_info("device_mod: 注册设备，触发匹配...\n");
	dev_pdev = platform_device_register_simple("helloplat", -1, NULL, 0);
	return 0;
}
static void __exit dev_exit(void)
{
	platform_device_unregister(dev_pdev);
	pr_info("device_mod: 注销设备\n");
}
module_init(dev_init);
module_exit(dev_exit);
MODULE_LICENSE("GPL");
