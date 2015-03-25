package com.mediatek.engineermode.nfc;

import android.widget.CheckBox;

public class ModeMap {
    ModeMap(CheckBox c, int b) {
        mChkBox = c;
        mBit = b;
    }

    CheckBox mChkBox;
    int mBit;
}