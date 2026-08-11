# 13 · 块设备驱动 vblk（实验态）⚠️

**学什么**：`blk-mq` 块设备——`blk_mq_tag_set`/`request_queue`/`gendisk`、`queue_rq` 回调处理读写、`device_add_disk` 注册。这是"内存当磁盘"的最简版，概念完整。

**大白话**：字符设备是"文件一样流着读写"，块设备是"磁盘一样按扇区寻址"——文件系统、分区表全建立在它上面。

> ⚠️ **实验态**：此课源码在 6.8 blk-mq API 下**未完全编通**（旧教程 API 已废弃）。完整可用的成品见 `projects/memblk/`（能 `mkfs.ext4` + `mount` 的版本）。

**编译**（尝试，不保证过）：
```bash
make
```

**坑**（6.8 API 变化重点）：
- 老教程的 `request_fn`/`blk_init_queue`/`add_disk` 在 6.x 已废弃 → 全部走 `blk_mq` + `device_add_disk`
- `set_capacity` 单位是**扇区**（512B），不是字节
- 卸载顺序严格：`del_gendisk` → `put_disk` → `blk_mq_free_queue` → `blk_mq_free_tag_set`
- 回调里 `blk_mq_start_request` 必须在访问 `rq` 数据前调用
