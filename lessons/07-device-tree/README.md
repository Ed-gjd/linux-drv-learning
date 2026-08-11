# 07 · 设备树 DTS

**学什么**：设备树语法——`/` 根节点、`compatible` 匹配字符串、`reg` 地址/大小、`#address-cells/#size-cells`。ARM 用设备树描述硬件，x86 用 ACPI，两种体系二选一。

**大白话**：硬件是一张"菜单"，设备树就是点菜单——告诉内核"这台板子上有啥、在哪、叫啥"，驱动按名字去认领。

**文件**：
```bash
less demo2.dts      # 根节点 model/compatible + 一个 hellodev@1000 子节点
```

**验证**（无板子也能玩）：
```bash
sudo apt install device-tree-compiler
dtc -I dts -O dtb -o /tmp/demo2.dtb demo2.dts    # 编译成二进制 dtb
dtc -I dtb -O dts /tmp/demo2.dtb                 # 反编译回来（来回验证语法）
```

**坑**：
- `compatible` 字符串必须和驱动的 `of_match_table` 完全一致，否则 probe 不触发
- `#address-cells/#size-cells` 配错 → `reg` 解析错 → 资源拿不到
- 改 `.dts` 不重编 dtb → 白改
