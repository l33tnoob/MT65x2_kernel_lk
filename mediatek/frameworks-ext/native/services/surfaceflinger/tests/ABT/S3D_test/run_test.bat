adb remount
adb push JB\s3d_test /system/bin
:: adb push ..\..\..\..\..\..\..\..\..\out\target\product\mt6577_phone\system\bin\layer_test /system/bin
adb shell chmod 777 /system/bin/s3d_test
adb shell s3d_test