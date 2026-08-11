# 10 · vsensor：虚拟传感器完整驱动 ⭐阶段4 验收

**学什么**：把前几课串成**一个能感知事件的完整驱动**——定时器模拟中断 → workqueue 下半部 → 环形缓冲存数据 → 字符设备阻塞读给用户态。这是"中断 + 下半部 + 字符设备"全链路，真实驱动骨架。

**大白话**：一个每秒"生产"一个读数的温度计，用户态 `cat` 它时没有新数就**睡着等**，有数才醒来拿走——阻塞读就是"按需取货"。

**编译**：
```bash
make
```

**验证**：
```bash
sudo insmod vsensor.ko
dmesg | grep "vsensor:"       # 每秒新读数
# 开一个终端：cat /dev/vsensor   （没读数会阻塞睡着）
# 每过 1 秒会冒出一个 100/200/300... 的读数
# 非阻塞：cat 用 O_NONBLOCK 时无数据立即返回 EAGAIN（试试 timeout 1 cat /dev/vsensor）
sudo rmmod vsensor
```

**坑**：
- 生产者（workqueue）与消费者（read）共享环形缓冲，必须加锁 → `mutex`
- 阻塞等待用 `wait_event_interruptible` + 生产后 `wake_up_interruptible` 配对，漏唤醒就睡死
- 环形缓冲空/满判定写错 → 丢数据或读脏数据

> 升级版：见 `projects/vsensor-final/`（devm_* 托管资源 + 完整错误处理 + 可调参数）。
