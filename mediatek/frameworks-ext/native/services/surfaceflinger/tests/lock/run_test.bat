adb remount
adb push .\..\..\..\..\..\..\..\..\out\target\product\mt6577_phone_qhd\system\bin\test-lock /system/bin
adb shell chmod 777 /system/bin/test-lock
adb shell /system/bin/test-lock
