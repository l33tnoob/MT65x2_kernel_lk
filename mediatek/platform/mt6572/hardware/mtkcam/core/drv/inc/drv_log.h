/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
//#ifndef _DRV_LOG_H_   // Can't use Header Guard because we use undef-redefine to apply new DBG_LOG_TAG and DBG_LOG_LEVEL for each file.
//#define _DRV_LOG_H_





/**************************************************************************
                       < How to Use DBG LOG APIs >

1. Copy following lines to your .cpp file, and update "WWWWWWWWWW" and "YYYYYYYYYY".:
[ Copy Start ]
#include <cutils/properties.h>              // For property_get().

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{WWWWWWWWWW} "
#include "imageio_log.h"                    // Note: DBG_LOG_TAG will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(YYYYYYYYYY);
//EXTERN_DBG_LOG_VARIABLE(YYYYYYYYYY);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (YYYYYYYYYY_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (YYYYYYYYYY_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (YYYYYYYYYY_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (YYYYYYYYYY_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (YYYYYYYYYY_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (YYYYYYYYYY_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)
[ Copy End ]

    Note1: Android will put the tag name [WWWWWWWWWW] before your debug message
           when output log to UART.
    Note2: YYYYYYYYYY is the module name you can use to control the module debug level.
    Note3: YYYYYYYYYY will be concatenated to _DbgLogEnable_* global variables.
    Note4: If you want to use the same module name YYYYYYYYYY in another file,
           when copy above lines to another file, you should use:
                EXTERN_DBG_LOG_VARIABLE(YYYYYYYYYY)
           instead of:
                DECLARE_DBG_LOG_VARIABLE(YYYYYYYYYY)
           Because YYYYYYYYYY_DbgLogEnable_* global variable is already declared
            in original file.

2. Add DBG_LOG_CONFIG(XXXXXXXXXX, YYYYYYYYYY) in your CreateInstance function
   (i.e. where it will be called whenever camera is re-entered) before using
   any debug log function.)

   Note1: XXXXXXXXXX is the group name you can use to control the group debug level.
   Note2: If you did not add DBG_LOG_CONFIG(XXXXXXXXXX, YYYYYYYYYY), default
          debug log level will be the one in "[Set default debug log level here]
          (when DBG_LOG_LEVEL_SET() is not called.)" below.

3. (optional) Change two places below with [Set default debug log level here] to
   change default debug level.

4. Now you can use LOG_AST()/LOG_ERR()/LOG_WRN()/LOG_INF()/LOG_DBG()/LOG_VRB() in your code.

5. Use following adb command to control debug log level:
    adb shell getprop debuglog.XXXXXXXXXX        => See current value of your group debug log. If there is nothing showing up, the property does not exist yet.
    adb shell setprop debuglog.XXXXXXXXXX ZZZ    => Set group debug log level. ZZZ can be one of the following value: 2(VERBOSE)/3(DEBUG)/4(INFO)/5(WARN)/6(ERROR)/7(ASSERT)/8(SILENT).
    adb shell getprop debuglog.XXXXXXXXXX.YYYYYYYYYY        => See current value of your module debug log. If there is nothing showing up, the property does not exist yet.
    adb shell setprop debuglog.XXXXXXXXXX.YYYYYYYYYY ZZZ    => Set module debug log level. ZZZ can be one of the following value: 2(VERBOSE)/3(DEBUG)/4(INFO)/5(WARN)/6(ERROR)/7(ASSERT)/8(SILENT).

   Note1: If module debug log level exists, module debug log level will be applied.
          (i.e. highest priority.)
   Note2: If module debug log level does not exist but group debug log level exists,
          group debug log level will be applied. (i.e. second priority.)
   Note3: If both module/group debug log level do not exist, default debug log level
          will be applied.

6. You must exit camera and re-enter again to make the new debug log level take effect.
**************************************************************************/

// Vent@20120615: Don't know why "__func__" and "__FUNCTION__" act like
//     "__PRETTY_FUNCTION__". Add following lines as a workaround to make
//     their behavior correct.
#undef	__func__
#define	__func__	__FUNCTION__

///////////////////////////////////////////////////////////////////////////
//                          Default Settings.                            //
///////////////////////////////////////////////////////////////////////////
#ifndef DBG_LOG_TAG     // Set default DBG_LOG_TAG.
    #define DBG_LOG_TAG     ""
#endif  // DBG_LOG_TAG

///////////////////////////////////////////////////////////////////////////
//                      Base Debug Log Functions                         //
///////////////////////////////////////////////////////////////////////////
#ifndef USING_MTK_LDVT   // Not using LDVT.
    #include <cutils/xlog.h>
    #define NEW_LINE_CHAR   ""      // XLOG?() already includes a new line char at the end of line, so don't have to add one.

    #define BASE_LOG_VRB(fmt, arg...)       XLOGV(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Verbose>: Show more detail debug information. E.g. Entry/exit of private function; contain of local variable in function or code block; return value of system function/API...
    #define BASE_LOG_DBG(fmt, arg...)       XLOGD(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Debug>: Show general debug information. E.g. Change of state machine; entry point or parameters of Public function or OS callback; Start/end of process thread...
    #define BASE_LOG_INF(fmt, arg...)       XLOGI(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Info>: Show general system information. Like OS version, start/end of Service...
    #define BASE_LOG_WRN(fmt, arg...)       XLOGW(DBG_LOG_TAG "[%s] WARNING: " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Warning>: Some errors are encountered, but after exception handling, user won't notice there were errors happened.
    #define BASE_LOG_ERR(fmt, arg...)       XLOGE(DBG_LOG_TAG "[%s, %s, line%04d] ERROR: " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg)	// When MP, will only show log of this level. // <Fatal>: Serious error that cause program can not execute. <Error>: Some error that causes some part of the functionality can not operate normally.
    #define BASE_LOG_AST(cond, fmt, arg...)     \
        do {        \
            if (!(cond))        \
                XLOGE("[%s, %s, line%04d] ASSERTION FAILED! : " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg);        \
        } while (0)

#else   // Using LDVT.
    #include "uvvf.h"
    #define NEW_LINE_CHAR   "\n"

    #define BASE_LOG_VRB(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Verbose>: Show more detail debug information. E.g. Entry/exit of private function; contain of local variable in function or code block; return value of system function/API...
    #define BASE_LOG_DBG(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Debug>: Show general debug information. E.g. Change of state machine; entry point or parameters of Public function or OS callback; Start/end of process thread...
   	#define BASE_LOG_INF(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Info>: Show general system information. Like OS version, start/end of Service...
    #define BASE_LOG_WRN(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] WARNING: " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Warning>: Some errors are encountered, but after exception handling, user won't notice there were errors happened.
  	#define BASE_LOG_ERR(fmt, arg...)        VV_ERRMSG(DBG_LOG_TAG "[%s, %s, line%04d] ERROR: " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg)	// When MP, will only show log of this level. // <Fatal>: Serious error that cause program can not execute. <Error>: Some error that causes some part of the functionality can not operate normally.
    #define BASE_LOG_AST(cond, fmt, arg...)     \
        do {        \
            if (!(cond))        \
                VV_ERRMSG("[%s, %s, line%04d] ASSERTION FAILED! : " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg);        \
        } while (0)

#endif  // USING_MTK_LDVT

///////////////////////////////////////////////////////////////////////////
//          Macros for dynamically changing debug log level              //
///////////////////////////////////////////////////////////////////////////
// [Set default debug log level here] (when DBG_LOG_LEVEL_SET() is not called.)
// Change the desired level (and following levels) to true.
// e.g. Change DEBUG and all level below DEBUG to true, then debug level is DEBUG.
#define DECLARE_DBG_LOG_VARIABLE(ModuleName)    \
    bool ModuleName ## _DbgLogEnable_VERBOSE   = false; \
    bool ModuleName ## _DbgLogEnable_DEBUG     = false; \
    bool ModuleName ## _DbgLogEnable_INFO      = true;  \
    bool ModuleName ## _DbgLogEnable_WARN      = true;  \
    bool ModuleName ## _DbgLogEnable_ERROR     = true;  \
    bool ModuleName ## _DbgLogEnable_ASSERT    = true;  \

#define EXTERN_DBG_LOG_VARIABLE(ModuleName)    \
    extern bool ModuleName ## _DbgLogEnable_VERBOSE;    \
    extern bool ModuleName ## _DbgLogEnable_DEBUG;      \
    extern bool ModuleName ## _DbgLogEnable_INFO;       \
    extern bool ModuleName ## _DbgLogEnable_WARN;       \
    extern bool ModuleName ## _DbgLogEnable_ERROR;      \
    extern bool ModuleName ## _DbgLogEnable_ASSERT;     \

// [Set default debug log level here] (when DBG_LOG_LEVEL_SET() is called.)
// Note: The default value in the property_get() does not controls the default
//       debug log level. It's the position of "default:" in the switch() controls
//       the default debug log level. E.g. put "default:" to case '5' will make
//       default debug level to WARN.
// PropertyStr: the Android property name you will use to control the debug level.
#define DBG_LOG_CONFIG(GroupName, ModuleName)                       \
    do {                                                            \ 
        char acDbgLogLevel[PROPERTY_VALUE_MAX] = {'\0'};            \
        property_get("debuglog." #GroupName "." #ModuleName, acDbgLogLevel, "0");             \
        ModuleName ## _DbgLogEnable_VERBOSE   = false;              \
        ModuleName ## _DbgLogEnable_DEBUG     = false;              \
        ModuleName ## _DbgLogEnable_INFO      = false;              \
        ModuleName ## _DbgLogEnable_WARN      = false;              \
        ModuleName ## _DbgLogEnable_ERROR     = false;              \
        ModuleName ## _DbgLogEnable_ASSERT    = false;              \
        if (acDbgLogLevel[0] == '0')                                \
        {                                                           \
            property_get("debuglog." #GroupName, acDbgLogLevel, "9");   \
        }                                                           \
        switch (acDbgLogLevel[0])                                   \
        {                                                           \
            case '2':                                               \
                ModuleName ## _DbgLogEnable_VERBOSE   = true;       \
            case '3':                                               \
                ModuleName ## _DbgLogEnable_DEBUG     = true;       \
            default:                                                \
            case '4':                                               \
                ModuleName ## _DbgLogEnable_INFO      = true;       \
            case '5':                                               \
                ModuleName ## _DbgLogEnable_WARN      = true;       \
            case '6':                                               \
                ModuleName ## _DbgLogEnable_ERROR     = true;       \
            case '7':                                               \
                ModuleName ## _DbgLogEnable_ASSERT    = true;       \
            case '8':                                               \
                break;                                              \
        }                                                           \
    } while (0)

///////////////////////////////////////////////////////////////////////////
//                          Other Definition.                            //
///////////////////////////////////////////////////////////////////////////

//#if (S_DbgLogEnable_DEBUG)
//    #define DBG_ISP_REG_DUMP    // A flag to control whether ISP Reg DUMP will be performed or not.
//#endif


//#endif  // _DRV_LOG_H_

