// SPDX-License-Identifier: GPL-2.0-only
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>

/* 驱动：声明"我处理叫 helloplat 的设备" */
static int demo_probe(struct platform_device *pdev)
{
	pr_info("platform: ★probe 被调用！设备名=%s\n", pdev->name);
	return 0;
}

/* remove 必须返回 int（可返回 -EBUSY 拒绝卸载） */
static int demo_remove(struct platform_device *pdev)
{
	pr_info("platform: remove 被调用\n");
	return 0;
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
