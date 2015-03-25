adb remount
adb push ..\..\..\..\..\..\..\..\out\target\product\mt6589_phone_720p\system\bin\test-flagsEx /system/bin
adb shell chmod 777 /system/bin/test-flagsEx
adb shell test-flagsEx
