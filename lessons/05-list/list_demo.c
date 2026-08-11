// SPDX-License-Identifier: GPL-2.0-only
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/slab.h>

struct item {
	int data;
	struct list_head node;
};

static LIST_HEAD(item_list);

static int __init list_init(void)
{
	struct item *p, *tmp;
	int i;

	for (i = 1; i <= 3; i++) {
		p = kmalloc(sizeof(*p), GFP_KERNEL);
		p->data = i * 10;
		list_add_tail(&p->node, &item_list);
		pr_info("list: 添加 data=%d\n", p->data);
	}

	pr_info("list: 遍历：");
	list_for_each_entry(p, &item_list, node)
		pr_info("  data=%d", p->data);
	pr_info("\n");

	pr_info("list: 清理：");
	list_for_each_entry_safe(p, tmp, &item_list, node) {
		list_del(&p->node);
		kfree(p);
	}
	pr_info("list: 清完，链表空=%d\n", list_empty(&item_list));
	return 0;
}

static void __exit list_exit(void)
{
	pr_info("list: 卸载\n");
}

module_init(list_init);
module_exit(list_exit);
MODULE_LICENSE("GPL");
