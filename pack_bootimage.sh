#!/bin/bash
#make to bootimage.sh
PRJ=tinno82_wet_kk
 echo "		gen out/target/product/tinno82_wet_kk/ boot.img ..."
 mediatek/build/tools/images/acp -uv out/target/product/${PRJ}/kernel_${PRJ}.bin out/target/product/${PRJ}/kernel
 mediatek/build/tools/images/mkbootfs mediatek/host/root | mediatek/build/tools/images/minigzip > out/target/product/${PRJ}/ramdisk.img
 mediatek/build/tools/mkimage out/target/product/${PRJ}/ramdisk.img ROOTFS > out/target/product/${PRJ}/ramdisk_android.img
 mv out/target/product/${PRJ}/ramdisk.img mediatek/kernel/trace32/${PRJ}_ramdisk.img
 mv out/target/product/${PRJ}/ramdisk_android.img out/target/product/${PRJ}/ramdisk.img
 mediatek/build/tools/images/mkbootimg  --kernel out/target/product/${PRJ}/kernel --ramdisk out/target/product/${PRJ}/ramdisk.img --board 1336460062 --output out/target/product/${PRJ}/boot.img
 echo "		==> [OK]   All done!"

