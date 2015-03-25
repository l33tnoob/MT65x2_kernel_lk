adb remount
adb push ..\..\..\..\..\..\..\..\out\target\product\mt6577_phone_qhd\system\bin\test-S3D_image /system/bin
adb push .\data\3D_SBS.png /data
adb push .\data\3D_TAB.png /data
adb shell chmod 777 /system/bin/test-S3D_image
adb shell /system/bin/test-S3D_image
