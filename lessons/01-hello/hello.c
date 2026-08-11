// SPDX-License-Identifier: GPL-2.0-only
// hello.c —— 第1课：最简内核模块
// 内核模块没有 main()，靠 module_init/module_exit 两个钩子
#include <linux/init.h>    // module_init/module_exit 宏
#include <linux/module.h>  // 模块相关（MODULE_LICENSE 等）
#include <linux/kernel.h>  // pr_info 等日志函数

// 模块加载时调用（类似"构造函数"）
// __init 标记：这段初始化代码用完即丢弃，节省内存
static int __init hello_init(void)
{
	pr_info("hello: Hello World from kernel module\n");
	return 0;   // 返回 0 = 加载成功；负值 = 加载失败并回滚
}

// 模块卸载时调用（类似"析构函数"）
// __exit 标记：编译进内核（非模块）时这段代码直接不要
static void __exit hello_exit(void)
{
	pr_info("hello: 模块已卸载！Goodbye kernel\n");
}

// 注册两个钩子
module_init(hello_init);
module_exit(hello_exit);

// 元信息：许可证必须是 GPL，否则内核被标记 Tainted（带 T）
MODULE_LICENSE("GPL");
MODULE_AUTHOR("playground learner");
MODULE_DESCRIPTION("第1课 Hello World 内核模块");
MODULE_VERSION("0.1");
