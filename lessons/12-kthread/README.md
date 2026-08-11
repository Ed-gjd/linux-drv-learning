# 12 · 内核线程 kthread

**学什么**：`kthread_run` 起内核线程、循环里查 `kthread_should_stop()`、卸载时 `kthread_stop` 优雅退出。内核里"自己干活的后台工人"。

**大白话**：和用户态线程一个思路，但它生在内核、没有用户空间——用来做周期性的内核工作。

**编译**：
```bash
make
```

**验证**：
```bash
sudo insmod kthread_demo.ko
dmesg | grep "kthread:"     # 线程启动，每秒第N次工作
ps aux | grep mykthread     # 进程表里能看到它
sudo rmmod kthread_demo     # 卸载时 kthread_stop 停线程
```

**坑**：
- 线程循环不查 `kthread_should_stop()` → 停不下来
- 卸载不 `kthread_stop` → 线程泄漏
- `kthread_run` 返回 `IS_ERR` 要查，别当 NULL 判
