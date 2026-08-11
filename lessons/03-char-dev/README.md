# 03 · 字符设备：hellodev（虚拟内存设备）⭐入门分水岭

**学什么**：字符设备驱动全套——主次设备号、`file_operations` 表、`copy_to_user/copy_from_user`、ioctl 命令、`class_create/device_create` 自动建 `/dev` 节点、mutex 保护共享数据、等待队列阻塞读、sysfs 属性。

**大白话**：把一段内核内存伪装成一个"文件"，用户态 `cat/echo` 甚至 `ioctl` 都能操作它。这是驱动开发最核心的一张 `fops` 表。

**编译**：
```bash
make                                   # 产出 hellodev.ko
gcc -o test_ioctl test_ioctl.c         # 用户态 ioctl 测试程序
```

**验证**：
```bash
sudo insmod hellodev.ko                # 主设备号=238 之类，/dev/hellodev 自动建
echo hello > /dev/hellodev && cat /dev/hellodev   # 写→读
./test_ioctl                           # ioctl CLEAR / GETLEN
cat /sys/class/hellodev/hellodev/hellodev_data    # sysfs 属性看 written
# 阻塞读：开一个终端 cat /dev/hellodev（没数据会睡着），另一终端 echo 写入唤醒它
sudo rmmod hellodev
```

**坑**（本课踩过的真坑）：
- **read 到尾部必须返回 0**（EOF）——否则 `cat/dd` 死循环读。实测留下过 600 万次重复日志
- `copy_to_user/from_user` 方向写反、返回值不查 → 数据丢/权限错
- 忘记 `.owner = THIS_MODULE` → 设备打开着就 rmmod → panic
- 错误路径忘解锁 → 用 `goto out_unlock` 统一出口
