# vsensor-final · 虚拟传感器（合格版）⭐成品

从 `lessons/10-vsensor` 升级而来的**合格版成品驱动**，演示一条完整的内核机制链：

```
定时器(模拟中断,上半部) → workqueue(下半部,进程上下文) → 环形缓冲
    → wake_up 阻塞读者 → 字符设备(read/ioctl) → 用户态
```

## 相比教学版的升级

| 项 | 教学版 | 合格版 |
|---|---|---|
| 注册 | `register_chrdev` + 手动建节点 | `miscdevice` 自动建 `/dev/vsensor` |
| 错误处理 | init 漏检部分返回值 | 全部检查，失败即释放 |
| 参数 | 写死 | `interval_ms` / `ring_size` 模块参数可调 |
| 接口 | 只 read | read + ioctl(`VS_RESET`/`VS_GET_COUNT`) |
| 日志 | 裸 pr_info | `pr_fmt` 统一前缀 |
| 竞态 | 基本 | 唤醒后兜底 `-EAGAIN`（多读者安全） |

## 编译与测试

```bash
# 编译
make

# 加载（默认 1 秒一个读数，ring 容量 10）
sudo insmod vsensor.ko
dmesg | grep "vsensor:"          # 看到启动 + 每秒新读数

# 阻塞读（每个采样间隔出一个读数）
cat /dev/vsensor

# 带参数加载
sudo rmmod vsensor
sudo insmod vsensor.ko interval_ms=200 ring_size=4

# ioctl 测试
gcc -o user/vsensor_test user/vsensor_test.c
sudo ./user/vsensor_test

# 非阻塞读验证：无数据立即返回（用 timeout 防止挂住）
sudo rmmod vsensor; sudo insmod vsensor.ko interval_ms=5000
timeout 1 cat /dev/vsensor       # 读不到，不阻塞

# 卸载（必须干净，无泄漏）
sudo rmmod vsensor
dmesg | grep "vsensor:" | tail
```

## 学到的"合格驱动"习惯

1. 上半部最小化：定时器回调只 `schedule_work`，绝不在里面做重活
2. 共享数据（环形缓冲）一律锁保护，锁内不做 IO/不 sleep
3. 错误路径：`goto`/提前返回都要把已分配资源释放干净
4. 卸载路径和加载路径一样重要：`del_timer` → `cancel_work_sync` → 释放
5. 阻塞读用 `wait_event_interruptible` + `wake_up_interruptible` 配对，唤醒后兜底重试
