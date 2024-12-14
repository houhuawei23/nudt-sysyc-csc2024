docker run --rm -it \
-v /home/hhw/Desktop/compilers/newsubmit:/coursegrader/submitdata \
-v /home/hhw/Desktop/compilers/sys-ycompiler/test:/coursegrader/testdata \
-v /home/hhw/Desktop/docker/lib:/coursegrader/dockerext 8baa205c448b
# -e PATH=/usr/lib/jvm/jdk-17.0.6/bin/:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin 

# -e GRANT_SUDO=yes \
docker run  -it --name csc-compile \
-v /home/hhw/Desktop/compilers/newsubmit:/coursegrader/submitdata \
-v /home/hhw/Desktop/compilers/sys-ycompiler/test:/coursegrader/testdata \
-v /home/hhw/Desktop/docker/lib:/coursegrader/dockerext 8baa205c448b

docker start -ai csc-compile 
