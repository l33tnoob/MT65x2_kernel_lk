adb remount
adb push ..\..\..\..\..\..\..\..\out\target\product\mt6577_phone_qhd\system\bin\test-S3D_video /system/bin
adb shell chmod 777 /system/bin/test-S3D_video
adb shell /system/bin/test-S3D_video
