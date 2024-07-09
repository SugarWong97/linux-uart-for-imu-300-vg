#!/bin/bash

## 指定的输出文件名
APP_NAME=quick-cpp

# 指定输出目录(主目录是在编译目录中，需要使用"../"或者"绝对路径")
OUTPUT_DIR=`pwd`/out


BUILD_DIR=./.build

rm $BUILD_DIR -rf
mkdir $BUILD_DIR -p

cd $BUILD_DIR
cmake .. -DOUTPUT_APPNAME=$APP_NAME -DOUTPUT_DIRNAME=${OUTPUT_DIR}
#cmake ..  -DCROSS_COMPILE=aarch64-rockchip1031-linux-gnu-
make -j16
