// SPDX-License-Identifier: GPL-2.0-only
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

/* 兼容 6.12+：del_timer 更名 timer_delete */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 12, 0)
#define timer_delete(t) del_timer(t)
#endif
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/wait.h>

#define DEV_NAME "vsensor"
#define RING_SIZE 10

static struct timer_list sample_timer;
static struct work_struct sample_work;
static wait_queue_head_t data_wait;
static struct mutex data_lock;
static int ring[RING_SIZE];
static int head_idx, tail_idx;
static int sample_count;
static int dev_major;
static struct class *vs_class;

static void sample_timer_cb(struct timer_list *t)
{
	schedule_work(&sample_work);
	mod_timer(&sample_timer, jiffies + msecs_to_jiffies(1000));
}

static void sample_work_fn(struct work_struct *work)
{
	int val;

	val = ++sample_count * 100;
	mutex_lock(&data_lock);
	ring[tail_idx] = val;
	tail_idx = (tail_idx + 1) % RING_SIZE;
	if (head_idx == tail_idx)
		head_idx = (head_idx + 1) % RING_SIZE;
	mutex_unlock(&data_lock);
	wake_up_interruptible(&data_wait);
	pr_info("vsensor: 新读数 %d\n", val);
}

static ssize_t vs_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
	int val;

	if (file->f_flags & O_NONBLOCK) {
		if (head_idx == tail_idx)
			return -EAGAIN;
	} else {
		if (wait_event_interruptible(data_wait, head_idx != tail_idx))
			return -ERESTARTSYS;
	}
	mutex_lock(&data_lock);
	val = ring[head_idx];
	head_idx = (head_idx + 1) % RING_SIZE;
	mutex_unlock(&data_lock);

	if (copy_to_user(buf, &val, sizeof(val)))
		return -EFAULT;
	return sizeof(val);
}

static struct file_operations vs_fops = {
	.owner = THIS_MODULE,
	.read  = vs_read,
};

static int __init vs_init(void)
{
	INIT_WORK(&sample_work, sample_work_fn);
	timer_setup(&sample_timer, sample_timer_cb, 0);
	init_waitqueue_head(&data_wait);
	mutex_init(&data_lock);

	dev_major = register_chrdev(0, DEV_NAME, &vs_fops);
	if (dev_major < 0)
		return dev_major;
	vs_class = class_create(DEV_NAME);
	device_create(vs_class, NULL, MKDEV(dev_major, 0), NULL, DEV_NAME);

	mod_timer(&sample_timer, jiffies + msecs_to_jiffies(1000));
	pr_info("vsensor: 启动，主设备号=%d\n", dev_major);
	return 0;
}

static void __exit vs_exit(void)
{
	timer_delete(&sample_timer);
	flush_work(&sample_work);
	device_destroy(vs_class, MKDEV(dev_major, 0));
	class_destroy(vs_class);
	unregister_chrdev(dev_major, DEV_NAME);
	pr_info("vsensor: 卸载\n");
}

module_init(vs_init);
module_exit(vs_exit);
MODULE_LICENSE("GPL");
