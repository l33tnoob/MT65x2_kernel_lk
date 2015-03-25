package com.hissage.struct;

import java.io.Serializable;

public class SNmsAssertAlertMsgData implements Serializable {
    private static final long serialVersionUID = -6404612989585652602L;
    public String fileName ;
    public int    line ;
    
    public SNmsAssertAlertMsgData(String errorContent){
        fileName = errorContent;
        line = 0;
    }
}
