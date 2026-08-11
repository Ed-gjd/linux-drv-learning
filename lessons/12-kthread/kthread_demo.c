// SPDX-License-Identifier: GPL-2.0-only
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static struct task_struct *my_thread;
static int work_count;

static int kthread_fn(void *data)
{
	pr_info("kthread: 线程启动\n");
	while (!kthread_should_stop()) {
		work_count++;
		pr_info("kthread: 第%d次工作\n", work_count);
		msleep(1000);
	}
	pr_info("kthread: 收到停止信号，退出\n");
	return 0;
}

static int __init kt_init(void)
{
	my_thread = kthread_run(kthread_fn, NULL, "mykthread");
	if (IS_ERR(my_thread))
		return PTR_ERR(my_thread);
	pr_info("kthread: 已创建，PID=%d\n", task_pid_nr(my_thread));
	return 0;
}

static void __exit kt_exit(void)
{
	kthread_stop(my_thread);
	pr_info("kthread: 卸载\n");
}

module_init(kt_init);
module_exit(kt_exit);
MODULE_LICENSE("GPL");
