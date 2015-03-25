#!/system/bin/mksh
stay_awake="delete from system where name='stay_on_while_plugged_in'; insert into system (name, value) values ('stay_on_while_plugged_in','3');"
screen_sleep="delete from system where name='screen_off_timeout'; insert into system (name, value) values ('screen_off_timeout','-1');"
/system/xbin/sqlite3 /data/data/com.android.providers.settings/databases/settings.db "${stay_awake}" ## set stay awake
/system/xbin/sqlite3 /data/data/com.android.providers.settings/databases/settings.db "${screen_sleep}" # set sleep to none
lockscreen="delete from locksettings where name='lockscreen.disabled'; insert into locksettings (name, user, value) values ('lockscreen.disabled', '0', '1');"
/system/xbin/sqlite3 /data/system/locksettings.db "${lockscreen}" ##set lock screen to none
