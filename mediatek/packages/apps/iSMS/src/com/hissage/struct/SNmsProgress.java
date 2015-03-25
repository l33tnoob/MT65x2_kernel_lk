package com.hissage.struct;

import java.io.Serializable;

public class SNmsProgress implements Serializable {
    public int total;
    public int current;
    public byte UpOrDown;
    public byte contType;
    public short recordId;
};