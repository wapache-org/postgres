

## 配置

```shell

./configure --prefix=${安装路径} --enable-depend --enable-cassert --enable-debug

# 解决断点乱跳问题
./configure --prefix=${安装路径} --enable-depend --enable-cassert --enable-debug CFLAGS="-ggdb -O0"

```

## 编译安装

1. 运行`make`
1. 运行`make install`
1. 使用`initdb`初始化数据库，指定数据目录
1. 使用Clion导入postgresql源码

## 调试

打开Clion的`debug configuration`，增加一个Application，
Target选postgres，
Executable选择到源码目录的`src/backend/postgres`，
程序参数写 `-D ${数据目录}`

## 参考资料

[clion调试postgresql](https://www.cnblogs.com/qiumingcheng/p/10738736.html)

