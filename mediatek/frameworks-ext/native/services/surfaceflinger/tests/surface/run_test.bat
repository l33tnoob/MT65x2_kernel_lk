adb remount
adb push ..\..\..\..\..\..\..\out\target\product\mt6582_phone_v1_2\system\bin\test-surface /system/bin
adb shell chmod 777 /system/bin/test-surface
adb shell test-surface
