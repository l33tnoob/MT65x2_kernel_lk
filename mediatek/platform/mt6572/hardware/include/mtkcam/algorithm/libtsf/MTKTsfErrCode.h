//MTK_SWIP_PROJECT_START
#ifndef _MTK_TSF_ERRCODE_H
#define _MTK_TSF_ERRCODE_H

///////////////////////////////////////////////////////////////////////////////                     
//!  Error code formmat is:
//!
//!  Bit 31~24 is global, each module must follow it, bit 23~0 is defined by module
//!  | 31(1 bit) |30-24(7 bits) |         23-0   (24 bits)      |
//!  | Indicator | Module ID    |   Module-defined error Code   |
//!  
//!  Example 1:
//!  | 31(1 bit) |30-24(7 bits) |   23-16(8 bits)   | 15-0(16 bits) |
//!  | Indicator | Module ID    | group or sub-mod  |    Err Code   |
//! 
//!  Example 2:
//!  | 31(1 bit) |30-24(7 bits) | 23-12(12 bits)| 11-8(8 bits) | 7-0(16 bits)  |
//!  | Indicator | Module ID    |   line number |    group     |    Err Code   |
//! 
//!  Indicator  : 0 - success, 1 - error
//!  Module ID  : module ID, defined below
//!  Extended   : module dependent, but providee macro to add partial line info
//!  Err code   : defined in each module's public include file,
//!               IF module ID is MODULE_COMMON, the errocode is
//!               defined here
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//! Error code type definition
///////////////////////////////////////////////////////////////////////////
typedef MINT32 MRESULT;

///////////////////////////////////////////////////////////////////////////
//! Helper macros to define error code
///////////////////////////////////////////////////////////////////////////
#define ERRCODE(modid, errid)           \
  ((MINT32)                              \
    ((MUINT32)(0x80000000) |             \
     (MUINT32)((modid & 0x7f) << 24) |   \
     (MUINT32)(errid & 0xffff))          \
  )

#define OKCODE(modid, okid)             \
  ((MINT32)                              \
    ((MUINT32)(0x00000000) |             \
     (MUINT32)((modid & 0x7f) << 24) |   \
     (MUINT32)(okid & 0xffff))           \
  )

///////////////////////////////////////////////////////////////////////////
//! Helper macros to check error code
///////////////////////////////////////////////////////////////////////////
#define SUCCEEDED(Status)   ((MRESULT)(Status) >= 0)
#define FAILED(Status)      ((MRESULT)(Status) < 0)

#define MODULE_MTK_TSF (0) // Temp value

#define MTKTSF_OKCODE(errid)         OKCODE(MODULE_MTK_TSF, errid)
#define MTKTSF_ERRCODE(errid)        ERRCODE(MODULE_MTK_TSF, errid)


// Detection error code
#define S_TSF_OK                  MTKTSF_OKCODE(0x0000)

#define E_TSF_NEED_OVER_WRITE     MTKTSF_ERRCODE(0x0001)
#define E_TSF_NULL_OBJECT         MTKTSF_ERRCODE(0x0002)
#define E_TSF_WRONG_STATE         MTKTSF_ERRCODE(0x0003)
#define E_TSF_WRONG_CMD_ID        MTKTSF_ERRCODE(0x0004)
#define E_TSF_WRONG_CMD_PARAM     MTKTSF_ERRCODE(0x0005)
#define E_TSF_PROCESS_OVER_TIME   MTKTSF_ERRCODE(0x0006)

#define E_TSF_ERR                 MTKTSF_ERRCODE(0x0100)

#endif
//MTK_SWIP_PROJECT_END