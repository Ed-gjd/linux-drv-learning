# 04 · 内核定时器 timer + jiffies

**学什么**：`timer_setup` + `mod_timer` 做延时任务、`jiffies` 与 `HZ` 的关系。定时器回调在**软中断上下文**运行——不能 sleep、不能调可能睡眠的 API。

**大白话**：内核的"闹钟"，到点回调一次，可以 `mod_timer` 重新上闹钟变成循环。

**编译**：
```bash
make
```

**验证**：
```bash
sudo insmod timer_demo.ko
dmesg | grep "timer:"          # 每秒一次，共 3 次（回调里 count<3 重设）
sudo rmmod timer_demo
```

**坑**：
- 回调是软中断上下文：`kmalloc(..., GFP_KERNEL)` 都会炸，要用 `GFP_ATOMIC`
- `jiffies` 会回绕（约 24 天），比较要用 `time_after/time_before`
- 卸载前 `del_timer` 防"回调时模块已没了"
