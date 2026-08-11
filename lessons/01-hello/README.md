# 01 · Hello World 内核模块

**学什么**：内核模块的最小骨架——`module_init/module_exit` 两个钩子、`insmod/rmmod/lsmod`、`dmesg` 看日志、`MODULE_LICENSE`。内核模块没有 `main()`，靠钩子被内核回调。

**大白话**：模块 = 一张可以随时插拔进内核的"说明书"卡片。`insmod` 插进去，`rmmod` 抽出来，`dmesg` 是它留下的便签。

**编译**：
```bash
make          # 内部是 make -C /lib/modules/$(uname -r)/build M=$PWD modules
```
前置：`sudo apt install build-essential linux-headers-$(uname -r)`

**验证**：
```bash
sudo insmod hello.ko && dmesg | grep "hello:"     # 加载，看日志
lsmod | grep hello                                  # 模块在列表里
sudo rmmod hello && dmesg | grep "hello:"          # 卸载，再看日志
```

**坑**：
- Secure Boot 开着会拒签模块：先 `mokutil --sb-state` 确认
- 忘 `MODULE_LICENSE("GPL")` → 内核被标 Tainted（`/proc/modules` 里带 T）
- 只测加载不测卸载 → 资源泄漏只在 rmmod 后暴露
