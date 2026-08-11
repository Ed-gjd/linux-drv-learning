// SPDX-License-Identifier: GPL-2.0-only
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/platform_device.h>

/* 驱动：声明"我处理叫 helloplat 的设备" */
static int demo_probe(struct platform_device *pdev)
{
	pr_info("platform: ★probe 被调用！设备名=%s\n", pdev->name);
	return 0;
}

/* 6.11 起 remove 改为返回 void（driver core 本就不检查返回值） */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)
static int demo_remove(struct platform_device *pdev)
#else
static void demo_remove(struct platform_device *pdev)
#endif
{
	pr_info("platform: remove 被调用\n");
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 11, 0)
	return 0;
#endif
}

static struct platform_driver demo_driver = {
	.driver = {
		.name = "helloplat",           /* 匹配用的名字 */
	},
	.probe  = demo_probe,
	.remove = demo_remove,
};

static struct platform_device *demo_pdev;

static int __init plat_init(void)
{
	/* 先注册驱动，再注册同名设备 → 内核匹配 → probe 触发 */
	platform_driver_register(&demo_driver);
	demo_pdev = platform_device_register_simple("helloplat", -1, NULL, 0);
	return 0;
}

static void __exit plat_exit(void)
{
	platform_device_unregister(demo_pdev);
	platform_driver_unregister(&demo_driver);
}

module_init(plat_init);
module_exit(plat_exit);
MODULE_LICENSE("GPL");
