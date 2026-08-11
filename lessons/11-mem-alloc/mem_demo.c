// SPDX-License-Identifier: GPL-2.0-only
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/io.h>          /* virt_to_phys */

static int __init mem_init(void)
{
	void *k;      /* kmalloc：物理连续 */
	void *v;      /* vmalloc：虚拟连续 */

	k = kmalloc(1024, GFP_KERNEL);
	v = vmalloc(1024);

	/* %px 故意用之：教学要对比 kmalloc/vmalloc 的真实地址差（生产请用 %p） */
	pr_info("mem: kmalloc 虚拟=%px 物理=%llx\n", k,
		(unsigned long long)virt_to_phys(k));
	pr_info("mem: vmalloc 虚拟=%px 物理=%llx\n", v,
		(unsigned long long)virt_to_phys(v));
	pr_info("mem: 虚拟地址差=%ld\n", (long)v - (long)k);

	kfree(k);
	vfree(v);
	return 0;
}

static void __exit mem_exit(void)
{
	pr_info("mem: 卸载\n");
}

module_init(mem_init);
module_exit(mem_exit);
MODULE_LICENSE("GPL");
