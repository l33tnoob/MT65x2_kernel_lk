<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
	package="com.mediatek.bluetooth" android:sharedUserId="android.uid.bluetooth">

	<application android:label="@string/app_label" 
		android:name="com.mediatek.bluetooth.BluetoothApplication"
		android:persistent="true"
		android:icon="@drawable/bluetooth"
		android:description="@string/app_description"
		android:supportsRtl="true">
		<!-- BLUEANGEL::PLACEHOLDER -->

	</application>

	<!-- permission -->
    <uses-permission android:name="com.android.permission.HANDOVER_STATUS" />
	<uses-permission android:name="android.permission.READ_CALL_LOG" />
    <uses-permission android:name="android.permission.INTERACT_ACROSS_USERS_FULL" />
	<uses-permission android:name="android.permission.RECEIVE_BOOT_COMPLETED"/>
	<uses-permission android:name="android.permission.BLUETOOTH"/>
	<uses-permission android:name="android.permission.BLUETOOTH_ADMIN"/>
	<uses-permission android:name="android.permission.READ_CONTACTS"/>
	<uses-permission android:name="android.permission.WAKE_LOCK"/>
	<uses-permission android:name="android.permission.WRITE_CONTACTS"/>
	<uses-permission android:name="android.permission.READ_PHONE_STATE" />	
	<uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE"/>
	<uses-permission android:name="android.permission.ACCESS_NETWORK_STATE"/>
	<uses-permission android:name="android.permission.CHANGE_NETWORK_STATE"/>
	<uses-permission android:name="android.permission.CONNECTIVITY_INTERNAL"/>
	<uses-permission android:name="android.permission.VIBRATE"/>
	<uses-permission android:name="android.permission.FLASHLIGHT"/>
	<uses-permission android:name="com.android.email.permission.ACCESS_PROVIDER"/>
	<uses-permission android:name="com.android.email.permission.ACCESS_PROVIDER"/>
	<uses-permission android:name="android.permission.READ_SMS"/>
	<uses-permission android:name="android.permission.WRITE_SMS"/>
	<uses-permission android:name="android.permission.SEND_SMS" />
	<uses-permission android:name="android.permission.WRITE_APN_SETTINGS" />
	<uses-permission android:name="android.permission.INTERNET" />
	<uses-permission android:name="android.permission.WRITE_WAPPUSH" />
	<uses-permission android:name="android.permission.READ_WAPPUSH" />
	<uses-permission android:name="android.permission.READ_PROFILE" />
	<uses-permission android:name="android.permission.CHANGE_CONFIGURATION" />
	<uses-permission android:name="android.permission.ACCESS_ALL_DOWNLOADS" />
	<uses-permission android:name="android.permission.ACCESS_DOWNLOAD_MANAGER"/>
	<uses-permission android:name="android.permission.ACCESS_ALL_EXTERNAL_STORAGE"/>
    <uses-permission android:name="com.google.android.gallery3d.permission.GALLERY_PROVIDER"/>
    <uses-permission android:name="android.permission.WRITE_MEDIA_STORAGE"/>
</manifest> 
