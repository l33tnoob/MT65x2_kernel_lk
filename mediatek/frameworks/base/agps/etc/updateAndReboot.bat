adb remount

adb push agps_profiles_conf.xml /etc/agps_profiles_conf.xml
adb shell rm /data/data/com.android.settings/databases/agps_profiles.db
adb shell rm /data/data/com.android.settings/databases/agps_profiles.db-journal
adb shell rm /data/data/com.android.settings/shared_prefs/agps_em_setting_config.xml
adb shell rm /data/data/com.android.settings/shared_prefs/agps_selected_profile.xml
adb shell rm /data/data/com.android.settings/shared_prefs/com.android.settings_preferences.xml
adb shell rm /data/data/com.android.settings/shared_prefs/*
adb shell ls /data/data/com.android.settings/shared_prefs/
adb shell reboot
pause