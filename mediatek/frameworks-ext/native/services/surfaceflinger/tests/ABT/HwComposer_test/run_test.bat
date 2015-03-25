adb remount
adb push JB\hwcomposer_test /system/bin
:: adb push ..\..\..\..\..\..\..\..\..\out\target\product\mt6577_phone\system\bin\hwcomposer_test /system/bin
adb shell chmod 777 /system/bin/hwcomposer_test
adb shell hwcomposer_test
::wait for restart
ping 127.0.0.1 -n 20 > nul

