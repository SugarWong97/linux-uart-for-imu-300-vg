#!/bin/bash

## 指定的输出文件名
APP_NAME=imu300_vg_uart

# 指定输出目录(主目录是在编译目录中，需要使用"../"或者"绝对路径")
OUTPUT_DIR=`pwd`/out



BUILD_DIR=./.build

rm $BUILD_DIR -rf
mkdir $BUILD_DIR -p

cd $BUILD_DIR
cmake .. -DOUTPUT_APPNAME=$APP_NAME -DOUTPUT_DIRNAME=${OUTPUT_DIR}  -DCROSS_COMPILE=aarch64-himix100-linux-
#cmake .. -DCROSS_COMPILE=aarch64-himix100-linux-
make -j16
cp $OUTPUT_DIR/* ~/share_nfs -v
