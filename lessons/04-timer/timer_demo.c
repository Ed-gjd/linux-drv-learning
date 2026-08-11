// SPDX-License-Identifier: GPL-2.0-only
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>

static struct timer_list my_timer;
static int count;

static void timer_callback(struct timer_list *t)
{
	pr_info("timer: 到点！jiffies=%lu, 第%d次\n", jiffies, ++count);
	if (count < 3)
		mod_timer(&my_timer, jiffies + msecs_to_jiffies(1000));
}

static int __init timer_init(void)
{
	pr_info("timer: 加载，HZ=%d, 当前jiffies=%lu\n", HZ, jiffies);
	timer_setup(&my_timer, timer_callback, 0);
	mod_timer(&my_timer, jiffies + msecs_to_jiffies(1000));
	return 0;
}

static void __exit timer_exit(void)
{
	del_timer(&my_timer);
	pr_info("timer: 卸载\n");
}

module_init(timer_init);
module_exit(timer_exit);
MODULE_LICENSE("GPL");
