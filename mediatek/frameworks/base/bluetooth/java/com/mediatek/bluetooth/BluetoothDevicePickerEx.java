/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.bluetooth;

/**
 * A helper to show a system "Device Picker" activity to the user.
 *
 * @hide
 */
public interface BluetoothDevicePickerEx {
    public static final String EXTRA_FILTER_TYPE_1 =
        "android.bluetooth.devicepicker.extra.FILTER_TYPE_1";

    /* MTK Added : Begin */
    public static final int FILTER_TYPE_PRINTER = 5;
    /** Ask device picker to show BT devices that support BIP */
    public static final int FILTER_TYPE_BIP = 6;
    /** Ask device picker to show BT devices that support HID */
    public static final int FILTER_TYPE_HID = 7;
    public static final int FILTER_TYPE_PRX = 8;
    /* MTK Added : End */
}
