adb remount
adb push ..\..\..\..\..\..\..\..\..\out\target\product\mt6577_phone\system\bin\test-surface /system/bin
adb shell chmod 777 /system/bin/test-surface
adb shell test-surface
