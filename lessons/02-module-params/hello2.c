// SPDX-License-Identifier: GPL-2.0-only
// hello2.c —— 第2课：模块参数 module_param
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>   // module_param 宏在这
#include <linux/kernel.h>

// 三个参数变量（带默认值）
static int myint = 3;        // 整型
static char *mystr = "def";  // 字符串指针（charp）
static bool mybool;		/* 布尔，static 默认即 false */

// 注册参数：module_param(变量, 类型, /sys权限)
// myint  : 0644 = /sys 可读可写（运行时能改）
// mystr  : 0444 = /sys 只读
// mybool : 0644 = /sys 可读可写
module_param(myint, int, 0644);
module_param(mystr, charp, 0444);
module_param(mybool, bool, 0644);

// 参数说明（modinfo 里能看到，也是好习惯）
MODULE_PARM_DESC(myint, "整型参数，默认3，运行时可改");
MODULE_PARM_DESC(mystr, "字符串参数，默认def，只读");
MODULE_PARM_DESC(mybool, "布尔参数，默认false，运行时可改");

static int __init hello2_init(void)
{
	pr_info("hello2: 加载成功！myint=%d mystr=%s mybool=%d\n",
		myint, mystr, mybool);
	return 0;
}

static void __exit hello2_exit(void)
{
	pr_info("hello2: 卸载，再见！\n");
}

module_init(hello2_init);
module_exit(hello2_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("第2课 模块参数");
MODULE_VERSION("0.1");
