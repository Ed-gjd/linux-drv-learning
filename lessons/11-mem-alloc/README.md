# 11 · 内核内存：kmalloc vs vmalloc

**学什么**：`kmalloc`（物理连续，小对象，快）vs `vmalloc`（虚拟连续、物理可散，大对象/不常访问）。`virt_to_phys` 看两者物理地址差异。

**大白话**：`kmalloc` 是"一整套连续抽屉"，`vmalloc` 是"虚拟上连续、实际东一块西一块"——内核把碎片映射成整块给你。

**编译**：
```bash
make
```

**验证**：
```bash
sudo insmod mem_demo.ko
dmesg | grep "mem:"        # 对比 kmalloc/vmalloc 的虚拟与物理地址差
sudo rmmod mem_demo
```

**坑**：
- 中断上下文用 `kmalloc(..., GFP_ATOMIC)`，忘了 → "sleeping in atomic context"
- 大对象（>几个页）用 vmalloc 更好，但访问慢
- 申请了不释放 → 泄漏（生产用 `devm_kzalloc` 托管）
