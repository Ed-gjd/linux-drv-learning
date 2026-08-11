// SPDX-License-Identifier: GPL-2.0-only
// hellodev.c —— 字符设备驱动：一个"虚拟内存设备"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/wait.h>

#define DEV_NAME "hellodev"
#define BUF_SIZE 100

/* 第4课：ioctl 命令定义（type='H' 魔数，nr=序号） */
#define HELLODEV_CLEAR  _IO('H', 1)        /* 命令1：清空缓冲区（无参数） */
#define HELLODEV_GETLEN _IOR('H', 2, int)  /* 命令2：取已写字节数（读方向,int） */

static char dev_buf[BUF_SIZE];
static int dev_major;
static int written;              /* 记录实际写入的字节数 */
static struct class *hellodev_class;   /* 第5课：设备类 */
static struct mutex dev_lock;    /* 第2课(阶段2)：互斥锁，保护 written+dev_buf */
static wait_queue_head_t read_wait;   /* 第3课(阶段2)：等待队列，阻塞读用 */

static int dev_open(struct inode *inode, struct file *file)
{
	pr_info("hellodev: open 被调用\n");
	return 0;
}

static ssize_t dev_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
	int ret;

	pr_info("hellodev: read 被调用, count=%zu, pos=%lld\n", count, *pos);

	/* 第3课：阻塞读——没数据就睡，写入唤醒 */
	if (file->f_flags & O_NONBLOCK) {
		if (written == 0)
			return -EAGAIN;              /* 非阻塞模式：没数据立即返回"再试" */
	} else {
		wait_event_interruptible(read_wait, written > 0);  /* 阻塞：睡到有数据 */
	}

	mutex_lock(&dev_lock);                          /* 拿锁（保护 dev_buf/written） */
	if (*pos >= written) {                          /* ① 位置到"实际写入长度" */
		ret = 0;                                    /* 返回0 = EOF */
		goto out_unlock;
	}
	if (count > written - *pos)                     /* ② 要读的超过剩余 */
		count = written - *pos;                     /* 截到剩余 */
	if (copy_to_user(buf, dev_buf + *pos, count)) {
		ret = -EFAULT;
		goto out_unlock;
	}
	*pos += count;                                  /* ③ 推进位置 */
	ret = count;
out_unlock:
	mutex_unlock(&dev_lock);                        /* 放锁 */
	return ret;
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
	int ret;

	pr_info("hellodev: write 被调用, count=%zu\n", count);
	mutex_lock(&dev_lock);                          /* 拿锁 */
	if (count > BUF_SIZE)
		count = BUF_SIZE;
	if (copy_from_user(dev_buf, buf, count)) {
		ret = -EFAULT;
		goto out_unlock;
	}
	written = count;                                /* 记录实际写入字节数 */
	*pos = 0;                                       /* 写后读位置归零 */
	wake_up_interruptible(&read_wait);              /* 第3课：唤醒等数据的读者 */
	ret = count;
out_unlock:
	mutex_unlock(&dev_lock);                        /* 放锁 */
	return ret;
}

/* 第4课：ioctl 处理函数（用户调 ioctl 时内核调它） */
static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case HELLODEV_CLEAR:                 /* 命令1：清空 */
		written = 0;
		memset(dev_buf, 0, BUF_SIZE);
		pr_info("hellodev: ioctl CLEAR 清空缓冲区\n");
		return 0;
	case HELLODEV_GETLEN:                /* 命令2：取长度 */
		if (copy_to_user((int __user *)arg, &written, sizeof(written)))
			return -EFAULT;
		pr_info("hellodev: ioctl GETLEN = %d\n", written);
		return 0;
	default:
		return -EINVAL;                  /* 未知命令 → 返回"非法参数" */
	}
}

/* 阶段3第1课：sysfs 属性——cat /sys/class/hellodev/hellodev/hellodev_data 看 written */
static ssize_t hellodev_data_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sysfs_emit(buf, "written=%d\n", written);
}
static DEVICE_ATTR_RO(hellodev_data);

static struct attribute *hellodev_attrs[] = {
	&dev_attr_hellodev_data.attr,
	NULL,
};
ATTRIBUTE_GROUPS(hellodev);

static struct file_operations dev_fops = {
	.owner          = THIS_MODULE,
	.open           = dev_open,
	.read           = dev_read,
	.write          = dev_write,
	.unlocked_ioctl = dev_ioctl,         /* 第4课：挂 ioctl */
};

static int __init hellodev_init(void)
{
	mutex_init(&dev_lock);                          /* 初始化锁 */
	init_waitqueue_head(&read_wait);                /* 初始化等待队列 */
	dev_major = register_chrdev(0, DEV_NAME, &dev_fops);
	if (dev_major < 0) {
		pr_err("hellodev: 注册失败\n");
		return dev_major;
	}

	/* 第5课：创建设备类 + 设备 → udev 自动建 /dev 节点 */
	hellodev_class = class_create(DEV_NAME);
	if (IS_ERR(hellodev_class)) {            /* class_create 失败 */
		unregister_chrdev(dev_major, DEV_NAME);
		return PTR_ERR(hellodev_class);
	}
	device_create_with_groups(hellodev_class, NULL, MKDEV(dev_major, 0), NULL,
				 hellodev_groups, DEV_NAME);

	pr_info("hellodev: 注册成功，主设备号=%d，/dev/%s 已自动创建\n", dev_major, DEV_NAME);
	return 0;
}

static void __exit hellodev_exit(void)
{
	device_destroy(hellodev_class, MKDEV(dev_major, 0));  /* 删设备节点 */
	class_destroy(hellodev_class);                        /* 删设备类 */
	unregister_chrdev(dev_major, DEV_NAME);
	pr_info("hellodev: 卸载\n");
}

module_init(hellodev_init);
module_exit(hellodev_exit);
MODULE_LICENSE("GPL");
