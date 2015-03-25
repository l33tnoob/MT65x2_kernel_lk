package com.mediatek.CellConnService;

import com.mediatek.CellConnService.IPhoneStatesCallback;

interface IPhoneStatesMgrService
{
    int verifyPhoneState(int slot, int reqType, IPhoneStatesCallback cb);
}
