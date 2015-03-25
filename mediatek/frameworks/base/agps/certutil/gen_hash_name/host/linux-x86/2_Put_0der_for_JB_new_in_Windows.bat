@ECHO OFF


echo === Put 0_der_md5 ===
adb push 0_der_md5            /data/misc/keychain/cacerts-added
adb shell chown system.system /data/misc/keychain/cacerts-added
adb shell chmod 0755          /data/misc/keychain/cacerts-added

REM adb push 0_pem_md5 /data/agps_supl/cacerts

REM agpscacertinit will setup /data/agps_supl/cacerts
adb shell ls -l /system/bin/agpscacertinit
echo adb shell agpscacertinit
adb shell agpscacertinit

:EOF
pause