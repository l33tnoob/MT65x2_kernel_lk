package com.mediatek.CellConnService;

interface IPhoneStatesCallback {  
    void onComplete(int nResult);  
    void onCompleteWithPrefer(int nResult, int nPreferSlot);  
}  