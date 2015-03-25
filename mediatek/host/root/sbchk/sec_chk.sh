#!/system/bin/sh

if [ ! -f "/data/sec/sbchk_boot.log" ]; then
  cp /data/sec/sbchk_last.log /data/sec/sbchk_boot.log
fi

rm /data/sec/sbchk_5.log
mv /data/sec/sbchk_4.log /data/sec/sbchk_5.log
mv /data/sec/sbchk_3.log /data/sec/sbchk_4.log
mv /data/sec/sbchk_2.log /data/sec/sbchk_3.log
mv /data/sec/sbchk_1.log /data/sec/sbchk_2.log
mv /data/sec/sbchk_last.log /data/sec/sbchk_1.log

touch /data/sec/sbchk_last.log
chmod 644 /data/sec/sbchk_last.log
/sbchk/sbchk > /data/sec/sbchk_last.log 2>&1
