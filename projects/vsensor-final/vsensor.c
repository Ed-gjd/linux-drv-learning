// SPDX-License-Identifier: GPL-2.0-only
/*
 * vsensor — 虚拟传感器（合格版）
 *
 * 教学工程 projects/vsensor-final 的成品驱动。相比 lessons/10-vsensor 的
 * 升级点：
 *   1. miscdevice 替代 register_chrdev，/dev 节点自动创建
 *   2. 全部返回值检查，错误路径完整（goto/提前返回都释放）
 *   3. 模块参数：interval_ms / ring_size 可调
 *   4. ioctl：VS_RESET / VS_GET_COUNT
 *   5. pr_fmt 统一日志前缀
 *
 * 演示的核心机制（真驱动骨架）：
 *   定时器模拟中断(上半部) -> schedule_work(下半部/进程上下文)
 *   -> 写环形缓冲 -> wake_up 阻塞读的进程
 *
 * 用法：
 *   insmod vsensor.ko
 *   insmod vsensor.ko interval_ms=200 ring_size=4
 *   cat /dev/vsensor            # 阻塞读，每个采样间隔出一个新读数
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/ioctl.h>

#define DRV_NAME "vsensor"

/* 环形缓冲容量上限（ring_size 参数的有效范围） */
#define RING_MAX 64

/* ioctl 命令（魔数 'V'） */
#define VS_RESET     _IO('V', 1)
#define VS_GET_COUNT _IOR('V', 2, unsigned long)

static unsigned int interval_ms = 1000;
module_param(interval_ms, uint, 0644);
MODULE_PARM_DESC(interval_ms, "采样间隔（毫秒），默认 1000");

static unsigned int ring_size = 10;
module_param(ring_size, uint, 0644);
MODULE_PARM_DESC(ring_size, "环形缓冲容量 1..64，默认 10");

static struct timer_list sample_timer;
static struct work_struct sample_work;
static struct mutex data_lock;
static wait_queue_head_t data_wait;

/* 环形缓冲：生产者=workqueue，消费者=read */
static int *ring;
static unsigned int head_idx;
static unsigned int tail_idx;
static unsigned long total_samples;

/*
 * 下半部：进程上下文，可以 sleep。生产一个读数，唤醒阻塞读者。
 * 环形缓冲满则覆盖最旧（tail 追平 head）。
 */
static void sample_work_fn(struct work_struct *work)
{
	int val;

	mutex_lock(&data_lock);
	val = (int)total_samples * 10;
	ring[tail_idx] = val;
	tail_idx = (tail_idx + 1) % ring_size;
	if (tail_idx == head_idx)
		head_idx = (head_idx + 1) % ring_size;
	total_samples++;
	mutex_unlock(&data_lock);

	wake_up_interruptible(&data_wait);
}

/*
 * 上半部（定时器回调，软中断上下文，不能 sleep）：只安排下半部。
 */
static void sample_timer_cb(struct timer_list *t)
{
	schedule_work(&sample_work);
	mod_timer(&sample_timer, jiffies + msecs_to_jiffies(interval_ms));
}

static int vs_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t vs_read(struct file *file, char __user *buf,
		       size_t count, loff_t *pos)
{
	int val;
	int ret;

	if (file->f_flags & O_NONBLOCK) {
		if (head_idx == tail_idx)
			return -EAGAIN;
	} else {
		ret = wait_event_interruptible(data_wait,
					       head_idx != tail_idx);
		if (ret)
			return ret;
	}

	mutex_lock(&data_lock);
	if (head_idx == tail_idx) {
		/* 唤醒后又被别的读者清空：兜底，让上层重试 */
		mutex_unlock(&data_lock);
		return -EAGAIN;
	}
	val = ring[head_idx];
	head_idx = (head_idx + 1) % ring_size;
	mutex_unlock(&data_lock);

	if (copy_to_user(buf, &val, sizeof(val)))
		return -EFAULT;
	return sizeof(val);
}

static long vs_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case VS_RESET:
		mutex_lock(&data_lock);
		head_idx = 0;
		tail_idx = 0;
		mutex_unlock(&data_lock);
		return 0;
	case VS_GET_COUNT:
		if (copy_to_user((unsigned long __user *)arg, &total_samples,
				 sizeof(total_samples)))
			return -EFAULT;
		return 0;
	default:
		return -EINVAL;
	}
}

static const struct file_operations vs_fops = {
	.owner          = THIS_MODULE,
	.open           = vs_open,
	.read           = vs_read,
	.unlocked_ioctl = vs_ioctl,
};

static struct miscdevice vs_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = DRV_NAME,
	.fops  = &vs_fops,
};

static int __init vs_init(void)
{
	int ret;

	if (ring_size < 1 || ring_size > RING_MAX) {
		pr_err("ring_size 必须在 1..%d 之间，当前 %u\n", RING_MAX,
		       ring_size);
		return -EINVAL;
	}
	if (interval_ms < 1) {
		pr_err("interval_ms 必须 >= 1，当前 %u\n", interval_ms);
		return -EINVAL;
	}

	ring = kzalloc(ring_size * sizeof(*ring), GFP_KERNEL);
	if (!ring)
		return -ENOMEM;

	mutex_init(&data_lock);
	init_waitqueue_head(&data_wait);
	INIT_WORK(&sample_work, sample_work_fn);
	timer_setup(&sample_timer, sample_timer_cb, 0);

	ret = misc_register(&vs_miscdev);
	if (ret) {
		pr_err("misc_register 失败: %d\n", ret);
		kfree(ring);
		return ret;
	}

	mod_timer(&sample_timer, jiffies + msecs_to_jiffies(interval_ms));
	pr_info("启动 interval=%ums ring=%u，/dev/vsensor 已创建\n",
		interval_ms, ring_size);
	return 0;
}

static void __exit vs_exit(void)
{
	del_timer(&sample_timer);
	cancel_work_sync(&sample_work);
	misc_deregister(&vs_miscdev);
	kfree(ring);
	pr_info("卸载\n");
}

module_init(vs_init);
module_exit(vs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("playground learner");
MODULE_DESCRIPTION("虚拟传感器合格版：定时器模拟中断+workqueue下半部+环形缓冲阻塞读");
MODULE_VERSION("2.0");
