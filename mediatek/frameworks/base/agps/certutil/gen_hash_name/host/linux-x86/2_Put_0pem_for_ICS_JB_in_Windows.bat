@ECHO OFF

adb remount

echo ===  /system/etc/security/cacerts   ===   before put
adb shell ls -l /system/etc/security/cacerts


echo ===  Start Put 0_pem_md5 ===
adb push 0_pem_md5 /system/etc/security/cacerts

echo ===  /system/etc/security/cacerts   ===   after put
adb shell ls -l /system/etc/security/cacerts


REM for jb new, this step is necessary
REM for jb old or ics, no agpscacertinit ==> do nothing
adb shell ls -l /system/bin/agpscacertinit
echo adb shell agpscacertinit
adb shell agpscacertinit


:EOF