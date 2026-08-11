# 避坑库 · Pitfall Library

> 全部来自本仓库真机开发中踩过的坑（很多在 /var/log/kern.log 里留下了实据）。
> 内核的坑高度重复——把踩过的写下来，比重新踩一遍便宜得多。

## 一、字符设备

1. **read 到尾部必须返回 0（EOF）**
   - 现象：`cat /dev/xxx` 死循环，日志刷屏（本仓库实测留下过 `read 被调用` 重复 **600 万次**的日志）。
   - 原因：read 返回 `>0` 时用户态以为还有数据，`cat` 继续读。
   - 修法：`*pos` 到"实际写入长度"时返回 0。
   - 检测：`dd if=/dev/xxx bs=1 count=10` 若不退出即中招。

2. **`copy_to_user` / `copy_from_user`**
   - 方向写反：`copy_to_user(用户buf, 内核buf, n)`，`copy_from_user(内核buf, 用户buf, n)`。
   - 返回值不查：返回非 0 = 有字节没复制成功，必须 `return -EFAULT`。

3. **忘记 `.owner = THIS_MODULE`**
   - 后果：设备打开着就 `rmmod` → 引用计数不归零，模块被卸但 fops 还在用 → panic。

4. **`register_chrdev` 已过时**
   - 现代写法：`alloc_chrdev_region` + `cdev_add`，或直接 `miscdevice`（自动建 `/dev` 节点，推荐简单设备用）。

## 二、并发与同步

5. **spinlock 里 sleep → 直接 panic**
   - 报错：`BUG: sleeping function called from invalid context`。
   - 规则：**能 sleep 用 mutex，中断/软中断上下文/短临界区才用 spinlock**。

6. **错误路径漏解锁**
   - 修法：`goto out` 统一出口，锁/资源在出口统一释放（本仓库 hellodev 用的模式）。

7. **`jiffies` 回绕（约 24 天）**
   - 别 `if (a < b)`，用 `time_after(a, b)` / `time_before(a, b)`。

8. **等待队列漏 `wake_up` → 进程睡死**
   - `wait_event_interruptible` 必须和 `wake_up_interruptible` 配对；唤醒后还要**兜底重试**（多读者时可能被抢）。

## 三、中断与下半部

9. **上半部（硬中断/定时器回调）里 sleep → panic**
   - 上半部只做：记状态 → `schedule_work` / `disable_irq`。重活全放下半部。

10. **卸载前漏 `flush_work` / `cancel_work_sync`**
    - work 还在跑模块已卸 → use-after-free。卸载顺序：`del_timer` → `cancel_work_sync` → 释放。

## 四、块设备 blk-mq（6.8 大改，本仓库最值钱的坑）

> 老教程（2.6/4.x 时代）的 API 在 6.8 几乎全废。以当前源码为准。

11. **`blk_alloc_disk` 签名从双参变单参**
    - 老：`blk_alloc_disk(queue, 0)`；6.8：`blk_alloc_disk(NUMA_NO_NODE)` 内部自动建 queue。
    - 更推荐一步到位：`blk_mq_alloc_disk(&tag_set, NULL)`（内部用 tag_set 建 queue+disk）。

12. **`device_add_disk` 从双参变三参**
    - 老：`device_add_disk(gd, NULL)`；6.8：`device_add_disk(NULL, gd, NULL)`（parent, disk, groups）。
    - 写错编译报 `incompatible pointer type`（实测 vblk 卡这）。

13. **`blk_mq_free_queue` 在 6.8 不存在**（6.10 才有）
    - 编译报 implicit declaration。6.8 释放：`blk_mq_alloc_disk` 自含 queue，**随 `put_disk` 一起释放**。
    - ⚠️ 别对 `blk_mq_alloc_disk` 的 disk 再手动 `blk_mq_destroy_queue` → **double-free，rmmod 段错误**（本仓库实测踩过）。

14. **`block_device_operations` 的 open/release 签名 6.6+ 改了**
    - `open(struct block_device*, fmode_t)` → `open(struct gendisk*, blk_mode_t)`
    - `release(struct gendisk*, fmode_t)` → `release(struct gendisk)` 单参
    - 编译报 `incompatible pointer type`。

15. **`set_capacity` 单位是扇区（512B）不是字节**
    - 4MB 盘 → `set_capacity(gd, 8192)`（8192×512=4MB）。

16. **`blk_mq_start_request(rq)` 必须在访问 rq 数据前调用**
    - 否则 request 状态不对，IO 处理异常。

## 五、日志与安全

17. **`%px` vs `%p`**
    - `%px` 打印真实地址（暴露内核布局，生产禁用）；`%p` 是哈希地址。教学要看真实地址才用 `%px`（本仓库 mmio/mem 课故意保留，已注释说明）。

18. **`MODULE_LICENSE("GPL")` 必须写**
    - 否则内核被标 Tainted（`/proc/modules` 带 T），部分 API 用不了。

19. **Secure Boot 拦自定义模块**
    - insmod 报 "required key not available" → `mokutil --sb-state` 确认，关掉或 MOK 签名。

20. **模块版本不匹配**
    - `.ko` 用旧内核编的，升级内核后 insmod 报 version magic 不符。重编：`make -C /lib/modules/$(uname -r)/build M=$PWD`。
    - （本仓库就是 6.8.0-100 → 6.8.0-136 漂移过，全部重编。）

## 六、运维与调试（开发环境）

21. **D 状态进程杀不掉**
    - 驱动卡死 IO → 进程 uninterruptible sleep，SIGKILL 无效。**重启 VM 最干净**，别硬耗。

22. **`pkill -f` 会匹配到自己的 ssh 命令行**
    - `pkill -f "mkfs.ext4"` 在 ssh 会话里会把远程 bash（命令行含该串）一起杀了 → 连接断。
    - 用 `pkill -x`（精确进程名）或 kill 具体 PID。

23. **模块版本漂移后全量重编**
    - 换内核后旧模块全不兼容，`make clean && make` 一次清。
