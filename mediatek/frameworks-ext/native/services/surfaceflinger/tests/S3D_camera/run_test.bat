adb remount
adb push .\..\..\..\..\..\..\..\..\..\out\target\product\mt6577_phone_qhd\system\bin\test-S3D_camera /system/bin
adb push .\data\3D_SBS.png /data
adb push .\data\camera.png /data
adb shell chmod 777 /system/bin/test-S3D_camera
adb shell /system/bin/test-S3D_camera
