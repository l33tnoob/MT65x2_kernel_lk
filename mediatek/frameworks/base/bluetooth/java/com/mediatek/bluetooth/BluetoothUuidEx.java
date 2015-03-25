package com.mediatek.bluetooth;

import java.util.UUID;
import android.os.ParcelUuid;

/**
 * A class used to define those proprietary uuid
 *
 * @hide
 *
 */
public class BluetoothUuidEx {

    /**
     * declare the parcelable uuid for Bpp receiver
     *
     * @hide
     * @internal
     *
     */
    public static final ParcelUuid BppReceiver =
             ParcelUuid.fromString("00001118-0000-1000-8000-00805F9B34FB");

    /**
     * declare the parcelable uuid for Bip responder
     *
     * @hide
     * @internal
     *
     */
    public static final ParcelUuid BipResponder =
             ParcelUuid.fromString("0000111B-0000-1000-8000-00805F9B34FB");

    /**
     * declare the parcelable uuid for proximity
     *
     * @hide
     * @internal
     *
     */
    public static final ParcelUuid Proximity =
             ParcelUuid.fromString("00001803-0000-1000-8000-00805F9B34FB");

    /**
     * declare the parcelable uuid for obex file transfer
     *
     * @hide
     * @internal
     *
     */
    public static final ParcelUuid ObexFileTransfer =
            ParcelUuid.fromString("00001106-0000-1000-8000-00805F9B34FB");
}
