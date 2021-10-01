# Readme for YCSB-C

Scan默认为prefix_scan, prefix_len长度默认为19， 可以在[Tree.h:140](./ART/Tree.h)行修改。

在CMakeList.txt中手动添加了gtest的支持，如果跟pmem_rocksdb一起编译，需要注释掉`add_subdirectory(gtest-1.8.1/fused-src/gtest)`.
```
include_directories(./gtest-1.8.1/fused-src)
add_subdirectory(gtest-1.8.1/fused-src/gtest)
```

在进行多线程测试时，每次创建一个新的工作线程时，需要在线程内手动调用`ycsbc_roart::register_threadinfo();`