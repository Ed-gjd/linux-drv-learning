# Linux Kernel Driver Learning Lab

> **Linux 内核驱动学习工坊** — 从 Hello World 到"能写合格驱动"的完整路径，全部真机验证。

A hands-on journey from *Hello World module* to *production-grade drivers*.
Every lesson compiles, every demo ran on a real kernel, two finished drivers included.
All code passes `checkpatch` (0 ERROR); CI rebuilds everything on every push.

![Status](https://img.shields.io/badge/status-active-brightgreen)
![License](https://img.shields.io/badge/license-GPL--2.0-blue)

---

## Why this repo / 为什么值得看

- **13 progressive lessons**（13 节循序渐进课程）covering char devices, locking, blocking I/O, platform drivers, device tree, interrupts & bottom halves, MMIO, kthreads, block devices.
- **2 production-style showcase drivers**（2 个成品驱动）: a virtual block device you can `mkfs`+`mount`, and a complete virtual sensor (interrupt → bottom half → ring buffer → char device).
- **Real & verified**（全部真实验证）: compiled and run on kernel **6.8.0-136** (Ubuntu 24.04); every lesson documents its own pitfalls (some bugs were hit and fixed for real — see the EOF-loop story in `03-char-dev`).
- **Engineering discipline**（工程化）: `checkpatch` 0 ERROR, CI compile gate, GPL-2.0, clean unload paths.

---

## Course Map / 课程地图

| # | Topic（英文） | 学什么（中文要点） | Status |
|---|---|---|---|
| 01 | Hello World module | 内核模块骨架：module_init/exit、insmod/rmmod、dmesg | ✅ |
| 02 | Module parameters | module_param 传参、/sys 运行时调整 | ✅ |
| 03 | Char device (`hellodev`) | 字符设备全套：fops、copy_*, ioctl、自动建节点、mutex、阻塞读、sysfs | ✅ |
| 04 | Kernel timer | timer_setup + jiffies，软中断上下文规则 | ✅ |
| 05 | Kernel list | list_head + container_of | ✅ |
| 06 | Platform driver | 驱动/设备分离，probe/remove 匹配 | ✅ |
| 07 | Device tree (DTS) | 设备树语法、compatible/reg | ✅ |
| 08 | Bottom half (workqueue) | 上半部最小化 + schedule_work 下半部 | ✅ |
| 09 | MMIO | ioremap / readl / writel + 内存屏障 | ✅ |
| 10 | Virtual sensor (`vsensor`) | 阶段4 验收：中断+下半部+环形缓冲+阻塞读 | ✅ |
| 11 | Kernel memory | kmalloc vs vmalloc、GFP | ✅ |
| 12 | Kernel threads | kthread_run / kthread_should_stop | ✅ |
| 13 | Block device (`vblk`) | blk-mq 块设备（6.8 API） | ✅ |

## Showcase / 成品驱动

| Project | 是什么 | 亮点 |
|---|---|---|
| [`projects/memblk/`](projects/memblk/) | 内存虚拟块设备 | **能 `mkfs.ext4` + `mount` + 读写**，数据一致性 md5 验证，反复重载无泄漏 |
| [`projects/vsensor-final/`](projects/vsensor-final/) | 虚拟传感器（合格版） | miscdevice + 模块参数 + ioctl + 完整错误处理，全链路实测 |

---

## Quick Start / 快速开始

```bash
# 1. 准备编译环境（Ubuntu/Debian）
sudo apt install build-essential linux-headers-$(uname -r)

# 2. 一键编译所有课程 + 成品驱动
make

# 3. 玩一个（以 vsensor 为例）
cd projects/vsensor-final
sudo insmod vsensor.ko
cat /dev/vsensor        # 每秒出一个读数
sudo rmmod vsensor
```

完整逐课加载/验证命令见各课 `README.md`；块设备成品的完整 `mkfs`/`mount` 演示见 `projects/memblk/README.md`。

---

## Supported Kernels / 内核版本

- **Verified on 6.8.x** (Ubuntu 24.04, `6.8.0-136-generic`) — full compile + runtime tested.
- Generally works on 6.x; the Makefile targets `/lib/modules/$(uname -r)/build` and 6.8 blk-mq APIs are used (see `lessons/13-block-dev` for the 6.8 API notes).
- Kernel 5.x / earlier: not guaranteed (block API has changed significantly).

---

## Engineering Notes / 工程规范

- **checkpatch**: all source files pass with **0 ERROR**. Known intentional warnings: `%px` specifiers in `09-mmio`/`11-mem-alloc` — used deliberately to show real kernel addresses for teaching (annotated in code).
- **CI**: GitHub Actions rebuilds every module on every push (`ubuntu-latest`, dynamic headers). See `.github/workflows/build.yml`.
- **License**: GPL-2.0. **Style**: kernel coding style (tabs, 80-col, `dev_*`/`pr_*` logging, `goto` error paths).

---

## Pitfall Library / 避坑库

Real bugs hit during development, collected in [`docs/pitfalls.md`](docs/pitfalls.md):
- `read()` must return 0 at EOF or `cat` loops forever (a 6-million-line logstorm in the lab log)
- `blk_alloc_disk` / `device_add_disk` / `blk_mq_free_queue` signatures changed in 6.8 — old tutorials mislead
- `%px` vs `%p`; `spinlock` sleep; missing `flush_work`; and more.

---

## Roadmap / 路线

- [ ] Kernel 6.10+ compatibility check
- [ ] QEMU-based insmod test in CI
- [ ] KASAN test kernel for deeper debugging lessons

## License / 许可

GPL-2.0-only. This is a learning project — every module was written and tested by hand on a real kernel, and mistakes are documented on purpose (see the pitfall library).
