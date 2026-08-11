# 02 · 模块参数 module_param

**学什么**：`module_param` 让 insmod 时传参（`int/charp/bool` 三种类型），参数会暴露到 `/sys/module/` 下，按权限可读可写。

**大白话**：给"说明书卡片"预留了几个可调的旋钮——加载时拧好，运行时还能在 `/sys` 里再拧。

**编译**：
```bash
make
```

**验证**：
```bash
sudo insmod hello2.ko myint=42 mystr="world" mybool=1
dmesg | grep hello2                    # 看到传入的值
cat /sys/module/hello2/parameters/myint    # /sys 里可见
echo 99 | sudo tee /sys/module/hello2/parameters/myint   # 0644 可运行时改
modinfo hello2.ko | grep parm         # 参数说明
sudo rmmod hello2
```

**坑**：
- `/sys` 权限位（0644/0444）决定运行时能不能改
- `charp` 参数别用 `static char mystr[]`（指向不可变的字面量），用 `char *`
