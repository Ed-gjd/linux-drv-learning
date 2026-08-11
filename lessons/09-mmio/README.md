# 09 · MMIO：ioremap / readl / writel

**学什么**：把物理地址映射成内核可访问的虚拟地址（`ioremap`）、用 `readl/writel` 读写设备寄存器。本课映射 VGA 显存基址 0xB8000 读几个字。

**大白话**：CPU 访问硬件寄存器 = 访问一块"特殊内存"。`ioremap` 给你开一扇门，`readl/writel` 是进出这扇门的规矩（带内存屏障）。

**编译**：
```bash
make
```

**验证**：
```bash
sudo insmod mmio_demo.ko
dmesg | grep "mmio:"        # 映射成功 + readl 读显存 3 个字
sudo rmmod mmio_demo
```

**坑**：
- `ioremap` 了不 `iounmap` → 资源泄漏（生产用 `devm_ioremap_resource` 自动释放）
- 裸指针访问寄存器会被编译器重排/缓冲 → 必须 `readl/writel`（带屏障）
- `__raw_readl` 无屏障，不懂别碰
