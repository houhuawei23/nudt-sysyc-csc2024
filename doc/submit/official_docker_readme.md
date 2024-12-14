# Official Docker README

使用方法:

```bash
docker load -i compile.tar.gz

docker run --rm -it \
-v 源代码路径:/coursegrader/submitdata 

# /coursegrader/submitdata为源代码挂载路径，
# /coursegrader/testdata为测试数据挂载路径，
# /coursegrader/dockerext为评测程序及相关lib挂载路径。

# docker启动选项：
--network no-internet --user root \
-e GRANT_SUDO=yes \
-e PATH=/usr/lib/jvm/jdk-17.0.6/bin/:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin

# docker启动命令
java -jar /coursegrader/dockerext/ARMKernel.jar /coursegrader/dockerext/config.json -Xmx16G

```

评测镜像还在开发，部分环境不全；评测程序源码不开源，仅供环境参考。
