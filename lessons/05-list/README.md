# 05 · 内核链表 list_head

**学什么**：内核自己的双向链表 `list_head` + `container_of` 宏——把链表节点"嵌入"业务结构体，遍历用 `list_for_each_entry`。

**大白话**：内核链表不是"存指针的容器"，而是"嵌进结构体的一块小骨头"，`container_of` 顺着骨头摸回整只鸡。

**编译**：
```bash
make
```

**验证**：
```bash
sudo insmod list_demo.ko
dmesg | grep "list:"          # 添加3个→遍历→安全删除→链表空
sudo rmmod list_demo
```

**坑**：
- 遍历删除必须用 `list_for_each_entry_safe`（先存 next 再删，否则悬垂指针）
- 链表节点零初始化：用 `LIST_HEAD()` 静态声明或 `INIT_LIST_HEAD` 动态初始化
