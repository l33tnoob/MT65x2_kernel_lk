@ECHO OFF

adb remount

echo ===  /data/agps_supl/shared   ===   before put
adb shell ls -l /data/agps_supl/shared


echo ===  Start Put 0_pem ===
adb push 0_pem_sha1 /data/agps_supl/shared

echo ===  /data/agps_supl/shared   ===   after put
adb shell ls -l /data/agps_supl/shared

:EOF