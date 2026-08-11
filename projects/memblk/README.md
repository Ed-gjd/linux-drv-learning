# memblk · 内存虚拟块设备（合格版）⭐成品

用内存当磁盘的 **blk-mq 块设备驱动**——加载后出现 `/dev/memblk`，可以像真磁盘一样 `mkfs`、`mount`、读写文件。这是 `lessons/13-block-dev` 实验态 vblk 的**成品版**，修好了 6.8 内核的 API 大改。

```
用户态(文件系统 ext4) → 块层 bio/request → memblk_queue_rq → 复制进出内存
```

## 6.8 blk-mq 正确写法（本驱动已验证）

| API | 6.8 用法 | 老教程(错误)写法 |
|---|---|---|
| 建队列 | `blk_mq_alloc_disk(&tag, NULL)` 一步建 disk+queue | `blk_alloc_disk(q, 0)` 双参（6.3 前） |
| 注册磁盘 | `device_add_disk(NULL, gd, NULL)` 三参 | `device_add_disk(gd, NULL)` 双参 |
| 释放队列 | `blk_mq_alloc_disk` 自含 queue，`put_disk` 一起释放 | `blk_mq_free_queue()`（6.10 才引入） |
| 回调 | `queue_rq` + `blk_mq_start_request` | `request_fn`（已废弃） |

## 编译与完整测试

```bash
# 编译
make

# 加载（默认 4MB 虚拟盘）
sudo insmod memblk.ko
lsblk | grep memblk          # 看到 /dev/memblk

# 当磁盘用：mkfs + mount + 读写
sudo mkfs.ext4 /dev/memblk
sudo mkdir -p /mnt/memblk
sudo mount /dev/memblk /mnt/memblk
echo "hello from memblk" | sudo tee /mnt/memblk/test.txt
sudo cat /mnt/memblk/test.txt          # 读回
sudo umount /mnt/memblk

# 带大小参数重载（10MB）
sudo rmmod memblk
sudo insmod memblk.ko disk_size_kb=10240
sudo mkfs.ext4 /dev/memblk && sudo mount /dev/memblk /mnt/memblk
df -h | grep memblk

# 卸载（必须干净）
sudo umount /mnt/memblk
sudo rmmod memblk
```

## 学到的要点

1. 块设备按**扇区(512B)**寻址，`set_capacity` 单位是扇区不是字节
2. `blk_mq_start_request` 必须在访问 `rq` 数据前调用
3. `op_is_write(req_op(rq))` 判断读写方向——方向错数据会神秘错乱
4. 卸载顺序严格：`del_gendisk` → `put_disk`（自含 queue 一起释放）→ `blk_mq_free_tag_set`
   - ⚠️ 别对 `blk_mq_alloc_disk` 创建的 disk 再显式 `blk_mq_destroy_queue`（会 double-free 崩溃，实测踩过）
5. 大块内存用 `vzalloc`（物理可不连续），不用 `kmalloc`
