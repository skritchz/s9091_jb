s9091_jb
========

Kernel repo for the Wiko PEAX based on latest 3.4 released by WIKO


to build

    cd kernel
    CROSS_COMPILE=/somewhereonyourcomputer/prebuilt/linux-x86/toolchain/arm-linux-androideabi-4.4.x/bin/arm-linux-androideabi- TARGET_PRODUCT=tinno77_s9091jb ./build.sh release verbose

This will create a file in the kernel/out/kernel_tinno77_s9091jb.bin folder. You can use it directly with the repack method. The mkimage has already been done.


CAUTION: You need to use the generated m4u.ko otherwise it won't boot.
