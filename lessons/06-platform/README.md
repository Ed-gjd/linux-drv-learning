# 06 · platform 驱动 + 驱动/设备分离

**学什么**：现代驱动范式——`platform_driver`（能力）与 `platform_device`（实体）**分离注册**，内核按名字/`compatible` 匹配后回调 `probe`。三个模块演示：

| 模块 | 作用 |
|---|---|
| `platform_demo` | 一个模块里注册驱动+注册同名字设备，顺序无关，匹配即 probe |
| `driver_mod` | 只注册"驱动"，等设备来配 |
| `device_mod` | 只注册"设备"，触发已注册驱动 probe |

**大白话**：驱动是"会修这台设备的师傅"，设备是"这台设备"，谁先到都行，内核当红娘。

**编译**：
```bash
make          # 编出 platform_demo.ko driver_mod.ko device_mod.ko
```

**验证**：
```bash
sudo insmod platform_demo.ko && dmesg | grep "platform:"   # 看 probe
# 分离演示：先装 driver_mod（无人配），再装 device_mod（触发 probe）
sudo insmod driver_mod.ko && dmesg | grep driver_mod
sudo insmod device_mod.ko && dmesg | grep driver_mod       # probe 被触发
sudo rmmod device_mod driver_mod platform_demo
ls /sys/bus/platform/devices/                              # 设备树里可见
```

**坑**：
- `probe` 不执行 = 最常见卡壳：名字/`compatible` 没对上，查 `/sys/bus/platform/devices/`
- 驱动/设备分离后，加载顺序无关是设计目标，不是 bug
