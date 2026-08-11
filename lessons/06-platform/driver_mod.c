// SPDX-License-Identifier: GPL-2.0-only
#include <linux/platform_device.h>
#include <linux/version.h>
static int d_probe(struct platform_device *pdev)
{
	pr_info("driver_mod: ★probe！匹配到设备=%s\n", pdev->name);
	return 0;
}
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)
static int d_remove(struct platform_device *pdev)
#else
static void d_remove(struct platform_device *pdev)
#endif
{
	pr_info("driver_mod: remove\n");
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)
	return 0;
#endif
}
static struct platform_driver d_drv = {
	.driver = { .name = "helloplat" },
	.probe = d_probe,
	.remove = d_remove,
};
static int __init d_init(void)
{
	pr_info("driver_mod: 注册驱动，等待设备...\n");
	platform_driver_register(&d_drv);
	return 0;
}
static void __exit d_exit(void)
{
	platform_driver_unregister(&d_drv);
	pr_info("driver_mod: 注销驱动\n");
}
module_init(d_init);
module_exit(d_exit);
MODULE_LICENSE("GPL");
