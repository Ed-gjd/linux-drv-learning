# 08 · 下半部 workqueue（模拟中断）

**学什么**：中断处理的"上半部/下半部"拆法——定时器模拟中断源（上半部，软中断上下文，必须快），`schedule_work` 把重活丢给 workqueue 下半部（进程上下文，能 sleep）。这是真实驱动骨架的最小形态。

**大白话**：前台接线生（上半部）只记下"来事了"，转身交给后台工单（workqueue）慢慢处理——前台绝不能堵。

**编译**：
```bash
make
```

**验证**：
```bash
sudo insmod virtual_sensor.ko
dmesg | grep "sensor:"      # 每秒：[上半部]事件N → [下半部]处理数据
sudo rmmod virtual_sensor
```

**坑**：
- 上半部（定时器/硬中断回调）里 sleep → 内核 panic "sleeping function called from invalid context"
- 卸载前必须 `flush_work`，否则 work 还在跑模块已卸 → use-after-free
