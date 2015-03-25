#ifndef I_HW_SCENARIO_TYPE_H
#define I_HW_SCENARIO_TYPE_H

namespace NSHwScenario {

enum EhwMode {
    eHW_UNKNOWN = 0,
    eHW_VSS,
    eHW_ZSD
};

enum EHwBufIdx{
    eID_Unknown    = 0,
    eID_Pass1In    = 0x01,
    eID_Pass1Out   = 0x02,
    eID_Pass2In    = 0x04,  
    eID_Pass2DISPO = 0x08,  
    eID_Pass2VIDO  = 0x10,
    // zsd added
    eID_Pass1RawOut = 0x20,
    eID_Pass1DispOut = 0x40,
};

enum EWaitType{
    eIRQ_NONE = 0,
    eIRQ_VS, 
    eIRQ_PASS1DONE,
    eIRQ_PASS2DONE,
};
};
#endif
