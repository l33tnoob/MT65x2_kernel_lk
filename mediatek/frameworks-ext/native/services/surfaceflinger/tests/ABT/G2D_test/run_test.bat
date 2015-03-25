adb remount
adb push JB\g2d_test /system/bin
:: adb push ..\..\..\..\..\..\..\..\..\out\target\product\mt6577_phone\system\bin\g2d_test /system/bin
adb shell chmod 777 /system/bin/g2d_test
adb shell g2d_test
