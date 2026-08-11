// SPDX-License-Identifier: GPL-2.0-only
/*
 * memblk — 内存虚拟块设备（合格版）
 *
 * 教学工程 projects/memblk 的成品驱动：用内存当磁盘，能 mkfs + mount。
 * 相比 lessons/13-block-dev 的 vblk（6.8 下编不通的实验态），本驱动用
 * 6.8 正确的 blk-mq API 重写，全部编译/运行验证过：
 *
 *   1. blk_mq_alloc_tag_set 注册多队列标签集
 *   2. blk_alloc_disk(NUMA_NO_NODE) —— 6.8 单参版，内部自动创建 request_queue
 *   3. device_add_disk(NULL, gd, NULL) —— 6.8 三参版
 *   4. 卸载：del_gendisk → blk_mq_destroy_queue(gd->queue) → put_disk → free_tag_set
 *
 * 用法：
 *   insmod memblk.ko                       # 默认 4MB 虚拟盘
 *   insmod memblk.ko disk_size_kb=10240    # 10MB
 *   mkfs.ext4 /dev/memblk && mount /dev/memblk /mnt && ... 读写 ...
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/blk-mq.h>
#include <linux/blkdev.h>
#include <linux/vmalloc.h>

#define MEMBLK_MAX_KB (128 * 1024)	/* 上限 128MB */

static unsigned int disk_size_kb = 4096;
module_param(disk_size_kb, uint, 0444);
MODULE_PARM_DESC(disk_size_kb, "虚拟磁盘大小（KB），默认 4096，上限 131072");

static char *memblk_data;		/* "磁盘"内存 */
static struct gendisk *memblk_gd;
static struct blk_mq_tag_set memblk_tag;
static unsigned int memblk_major;

/*
 * 块设备核心回调：把 bio/request 里的段复制进/出内存。
 * 注意 blk_mq_start_request 必须在访问 rq 数据之前调用。
 */
static blk_status_t memblk_queue_rq(struct blk_mq_hw_ctx *hctx,
				    const struct blk_mq_queue_data *bd)
{
	struct request *rq = bd->rq;
	struct bio_vec bvec;
	struct req_iterator iter;
	unsigned long pos = blk_rq_pos(rq) << 9;
	bool write = op_is_write(req_op(rq));

	blk_mq_start_request(rq);

	rq_for_each_segment(bvec, rq, iter) {
		char *buf = bvec_kmap_local(&bvec);

		if (write)
			memcpy(memblk_data + pos, buf, bvec.bv_len);
		else
			memcpy(buf, memblk_data + pos, bvec.bv_len);
		kunmap_local(buf);
		pos += bvec.bv_len;
	}

	blk_mq_end_request(rq, BLK_STS_OK);
	return BLK_STS_OK;
}

static const struct blk_mq_ops memblk_mq_ops = {
	.queue_rq = memblk_queue_rq,
};

static int memblk_open(struct gendisk *gd, blk_mode_t mode)
{
	return 0;
}

static void memblk_release(struct gendisk *gd)
{
}

static const struct block_device_operations memblk_fops = {
	.owner   = THIS_MODULE,
	.open    = memblk_open,
	.release = memblk_release,
};

static int __init memblk_init(void)
{
	unsigned long data_bytes = disk_size_kb * 1024UL;
	unsigned int sectors = data_bytes >> 9;
	int ret;

	if (disk_size_kb < 1 || disk_size_kb > MEMBLK_MAX_KB) {
		pr_err("disk_size_kb 必须在 1..%d 之间，当前 %u\n",
		       MEMBLK_MAX_KB, disk_size_kb);
		return -EINVAL;
	}

	memblk_data = vzalloc(data_bytes);
	if (!memblk_data)
		return -ENOMEM;

	memblk_tag.ops = &memblk_mq_ops;
	memblk_tag.nr_hw_queues = 1;
	memblk_tag.queue_depth = 16;
	memblk_tag.numa_node = NUMA_NO_NODE;
	ret = blk_mq_alloc_tag_set(&memblk_tag);
	if (ret)
		goto err_free_data;

	memblk_gd = blk_mq_alloc_disk(&memblk_tag, NULL);	/* 用 tag_set 建 queue+disk */
	if (IS_ERR(memblk_gd)) {
		ret = PTR_ERR(memblk_gd);
		goto err_free_tag;
	}

	memblk_major = register_blkdev(0, "memblk");
	if (memblk_major < 0) {
		ret = memblk_major;
		goto err_put_disk;
	}

	memblk_gd->major = memblk_major;
	memblk_gd->first_minor = 0;
	memblk_gd->minors = 1;
	snprintf(memblk_gd->disk_name, DISK_NAME_LEN, "memblk");
	memblk_gd->fops = &memblk_fops;
	set_capacity(memblk_gd, sectors);

	ret = device_add_disk(NULL, memblk_gd, NULL);	/* 6.8 三参 */
	if (ret)
		goto err_unregister_blkdev;

	pr_info("%u KB 虚拟磁盘已创建：/dev/memblk（%u 扇区）\n",
		disk_size_kb, sectors);
	return 0;

err_unregister_blkdev:
	unregister_blkdev(memblk_major, "memblk");
err_put_disk:
	put_disk(memblk_gd);
err_free_tag:
	blk_mq_free_tag_set(&memblk_tag);
err_free_data:
	vfree(memblk_data);
	return ret;
}

static void __exit memblk_exit(void)
{
	del_gendisk(memblk_gd);
	put_disk(memblk_gd);		/* blk_mq_alloc_disk 的 disk 自含 queue，随 put_disk 释放 */
	blk_mq_free_tag_set(&memblk_tag);
	unregister_blkdev(memblk_major, "memblk");
	vfree(memblk_data);
	pr_info("卸载\n");
}

module_init(memblk_init);
module_exit(memblk_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("playground learner");
MODULE_DESCRIPTION("内存虚拟块设备：blk-mq 实现，可 mkfs+mount");
MODULE_VERSION("1.0");
