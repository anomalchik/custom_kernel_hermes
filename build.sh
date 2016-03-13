#!/bin/bash
# Version 0.3

## EXPORT ##
export ARCH=arm64
#export SUBARCH=arm
#export KBUILD_BUILD_USER="anomalchik"
#export KBUILD_BUILD_HOST="sweetmachine"
export CROSS_COMPILE=/home/anomalchik/tch/android_prebuilts_gcc_linux-x86_aarch64_aarch64-linux-android-4.9-cm-13.0/bin/aarch64-linux-android-

## MAKE ##
cd kernel-3.10
make hermes_defconfig
make -j4
cd ..
mv kernel-3.10/arch/arm64/boot/Image.gz-dtb crlv/boot/boot.img-kernel

## REPACK ##
cd crlv
sh makebootimage.sh
cd ..
mv crlv/output/boot.img forpack/boot.img

## ZIPPING ##
cd forpack
zip  -r ../boot.zip ./
