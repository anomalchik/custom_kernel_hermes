## EXPORT ##
export ARCH=arm64
#export SUBARCH=arm
#export KBUILD_BUILD_USER="anomalchik"
#export KBUILD_BUILD_HOST="sweetmachine"
export CROSS_COMPILE=/home/anomalchik/tch/android_prebuilts_gcc_linux-x86_aarch64_aarch64-linux-android-4.9-cm-13.0/bin/aarch64-linux-android-

cd kernel-3.10
make mrproper
cd ..
rm forpack/boot.img
rm boot.zip
