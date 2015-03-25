adb remount
adb push JB\layer_test /system/bin
:: adb push ..\..\..\..\..\..\..\..\..\out\target\product\mt6577_phone\system\bin\layer_test /system/bin
adb shell chmod 777 /system/bin/layer_test
adb shell layer_test
