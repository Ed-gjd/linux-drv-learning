// SPDX-License-Identifier: GPL-2.0-only
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

static struct timer_list sensor_timer;
static struct work_struct sensor_work;
static int event_count;

static void sensor_timer_cb(struct timer_list *t)
{
	event_count++;
	pr_info("sensor: [上半部/中断上下文] 事件%d，安排下半部\n", event_count);
	schedule_work(&sensor_work);

	mod_timer(&sensor_timer, jiffies + msecs_to_jiffies(1000));
}

static void sensor_work_fn(struct work_struct *work)
{
	pr_info("sensor: [下半部/进程上下文] 处理数据\n");
}

static int __init sensor_init(void)
{
	INIT_WORK(&sensor_work, sensor_work_fn);
	timer_setup(&sensor_timer, sensor_timer_cb, 0);
	mod_timer(&sensor_timer, jiffies + msecs_to_jiffies(1000));
	pr_info("sensor: 虚拟传感器启动，每秒模拟一次中断\n");
	return 0;
}

static void __exit sensor_exit(void)
{
	del_timer(&sensor_timer);
	flush_work(&sensor_work);
	pr_info("sensor: 卸载\n");
}

module_init(sensor_init);
module_exit(sensor_exit);
MODULE_LICENSE("GPL");
