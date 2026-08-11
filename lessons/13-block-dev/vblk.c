// SPDX-License-Identifier: GPL-2.0-only
/*
 * vblk —— 块设备驱动入门（blk-mq 最简版）
 *
 * 用 6.8 内核正确的 blk-mq API 编写（对比 lessons 里会踩的坑）：
 *   - blk_mq_alloc_tag_set 注册多队列标签集
 *   - blk_mq_alloc_disk(&tag, NULL) —— 一步创建 request_queue + gendisk
 *   - device_add_disk(NULL, gd, NULL) —— 6.8 三参签名
 *   - 卸载：del_gendisk → put_disk → blk_mq_free_tag_set
 *
 * 完整成品版（能 mkfs+mount，已全量验证）见 projects/memblk/。
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/blk-mq.h>
#include <linux/blkdev.h>
#include <linux/vmalloc.h>

#define VBLK_SECTORS 2048
#define VBLK_SIZE   (VBLK_SECTORS << 9)

static char *vblk_data;
static struct gendisk *vblk_gd;
static struct blk_mq_tag_set *vblk_tag;
static int vblk_major;

static blk_status_t vblk_queue_rq(struct blk_mq_hw_ctx *hctx,
				  const struct blk_mq_queue_data *bd)
{
	struct request *rq = bd->rq;
	struct bio_vec bvec;
	struct req_iterator iter;
	unsigned long offset = blk_rq_pos(rq) << 9;
	bool write = rq_data_dir(rq) == WRITE;

	blk_mq_start_request(rq);
	rq_for_each_segment(bvec, rq, iter) {
		char *buf = bvec_kmap_local(&bvec);

		if (write)
			memcpy(vblk_data + offset, buf, bvec.bv_len);
		else
			memcpy(buf, vblk_data + offset, bvec.bv_len);
		kunmap_local(buf);
		offset += bvec.bv_len;
	}
	blk_mq_end_request(rq, BLK_STS_OK);
	return BLK_STS_OK;
}

static const struct blk_mq_ops vblk_mq_ops = {
	.queue_rq = vblk_queue_rq,
};

static int vblk_open(struct gendisk *gd, blk_mode_t mode)
{
	return 0;
}

static void vblk_release(struct gendisk *gd)
{
}

static const struct block_device_operations vblk_fops = {
	.owner   = THIS_MODULE,
	.open    = vblk_open,
	.release = vblk_release,
};

static int __init vblk_init(void)
{
	int ret;

	vblk_major = register_blkdev(0, "vblk");
	if (vblk_major < 0)
		return vblk_major;

	vblk_data = vzalloc(VBLK_SIZE);
	if (!vblk_data) {
		ret = -ENOMEM;
		goto err_unregister;
	}

	vblk_tag = kzalloc(sizeof(*vblk_tag), GFP_KERNEL);
	if (!vblk_tag) {
		ret = -ENOMEM;
		goto err_free_data;
	}
	vblk_tag->ops = &vblk_mq_ops;
	vblk_tag->nr_hw_queues = 1;
	vblk_tag->queue_depth = 8;
	ret = blk_mq_alloc_tag_set(vblk_tag);
	if (ret)
		goto err_free_tag;

	vblk_gd = blk_mq_alloc_disk(vblk_tag, NULL);	/* 6.8 一步建 queue+disk */
	if (IS_ERR(vblk_gd)) {
		ret = PTR_ERR(vblk_gd);
		goto err_free_tag_set;
	}
	vblk_gd->major = vblk_major;
	vblk_gd->first_minor = 0;
	vblk_gd->minors = 1;
	snprintf(vblk_gd->disk_name, 32, "vblk");
	vblk_gd->fops = &vblk_fops;
	set_capacity(vblk_gd, VBLK_SECTORS);

	ret = device_add_disk(NULL, vblk_gd, NULL);	/* 6.8 三参 */
	if (ret)
		goto err_put_disk;

	pr_info("1MB 虚拟磁盘已创建 /dev/vblk\n");
	return 0;

err_put_disk:
	put_disk(vblk_gd);
err_free_tag_set:
	blk_mq_free_tag_set(vblk_tag);
err_free_tag:
	kfree(vblk_tag);
err_free_data:
	vfree(vblk_data);
err_unregister:
	unregister_blkdev(vblk_major, "vblk");
	return ret;
}

static void __exit vblk_exit(void)
{
	del_gendisk(vblk_gd);
	put_disk(vblk_gd);			/* 自含 queue，随 put_disk 释放 */
	blk_mq_free_tag_set(vblk_tag);
	kfree(vblk_tag);
	vfree(vblk_data);
	unregister_blkdev(vblk_major, "vblk");
	pr_info("卸载\n");
}

module_init(vblk_init);
module_exit(vblk_exit);
MODULE_LICENSE("GPL");
