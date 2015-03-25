
 
 #ifndef _DX_VOS_ERROR_H
#define _DX_VOS_ERROR_H

#include "DxTypes.h"
#ifdef __cplusplus
extern "C" {
#endif

/*! \file DX_VOS_Errors.h
    \brief This file contains OS errors definitions
*/
	 
typedef enum {
    DX_SUBSYSTEM_MW = 0x01,
	DX_SUBSYSTEM_GENERAL = 0x02,
	DX_SUBSYSTEM_INFRA,
    DX_SUBSYSTEM_DRM,
    DX_SUBSYSTEM_MOBILETV,
    DX_SUBSYSTEM_ROAP,
	DX_SUBSYSTEM_VOS = 0x0A,
	DX_SUBSYSTEM_IMODE_QA,
	DX_SUBSYSTEM_HDCP = 0x0C
} EDxSubSystems;

/******************** Defines  ********************/
#define ERROR_CODE_BITS 24

// These are general purpose error codes.
typedef enum {
	DX_FAILURE = (DX_SUBSYSTEM_GENERAL << ERROR_CODE_BITS),
	DX_BAD_ARGUMENTS,
	DX_MEM_ALLOCATION_ERROR,
	DX_BUFFER_IS_NOT_BIG_ENOUGH,
	DX_WRONG_BUFFER_SIZE,
	DX_TIMEOUT_EXPIRED,
	DX_INVALID_FORMAT,
	DX_OVERFLOW,
	DX_INVALID_URL,
	DX_ITEM_NOT_FOUND,
	DX_ITEM_ALREADY_EXISTS,
    DX_DEADLOCK,
    DX_NOT_SUPPORTED,
    DX_ASSERTION_ERROR,
    DX_NOT_INITIALIZED,
	DX_ALREADY_INITIALIZED,
    DX_NOT_AVAILABLE,
    DX_SECURE_TIME_NOT_SET,
	DX_IPC_FAILURE,
    DX_NOT_AUTHORIZED
} EDxGeneralErrors;


// These are VOS specific error codes.
typedef enum {
	DX_VOS_FILE_ERROR = (DX_SUBSYSTEM_VOS << ERROR_CODE_BITS),
	DX_VOS_END_OF_FILE,
	DX_VOS_THREAD_ERROR,
	DX_VOS_SEM_ERROR,
	DX_VOS_SOCKET_ERROR,
	DX_VOS_SOCKET_CLOSED,
	DX_VOS_TIME_ERROR,
	DX_VOS_TIMER_ERROR,
	DX_VOS_MEMMAP_ERROR,
	DX_VOS_DRM_UTILS_ERROR,
	DX_VOS_MOBILE_UTILS_ERROR,
	DX_VOS_NOTIFICATION_ERROR,
	DX_VOS_NO_NOTIFICATION,
    DX_VOS_SIM_ERROR,
    DX_VOS_SECURE_TIME_ERROR,
    DX_VOS_SECURE_TIME_NOT_SET,
    DX_VOS_SIM_INVALID_RESPONSE,
	DX_VOS_STORAGE_READ_ERROR

} EDxVosErrors;

#define DX_ERROR		 ((DxUint32)-1)
#define DX_INVALID_INDEX ((DxUint32)-1)

#ifdef DX_NO_DEBUG_PRINT
#undef DX_DEBUG_PRINT
#endif

#if defined(_DEBUG) && !defined(DX_NO_DEBUG_PRINT) && !defined(DX_DEBUG_PRINT)
#define DX_DEBUG_PRINT
#endif

#ifdef DX_DEBUG_PRINT
#define LOG_ERROR_STACK()  DxErrorStack_Log(DX_DBG_MODULE, DX_ERROR_PROPAGATION_LEVEL) 
#define CLEAR_ERROR()      DxErrorStack_Clear()

#define ADD_NEW_ERROR(errorCode) \
    do { LOG_ERROR_STACK(); DxErrorStack_Add(__FILE__,__LINE__,__FUNCTION__,errorCode,#errorCode); result = errorCode; } while (0)

#define ADD_OLD_ERROR(errorCode) \
    do { DxErrorStack_Add(__FILE__,__LINE__,__FUNCTION__,errorCode,#errorCode); result = errorCode;} while (0)

#define RETURN_NEW_ERROR(errorCode) \
    do { LOG_ERROR_STACK(); DxErrorStack_Add(__FILE__,__LINE__,__FUNCTION__,errorCode,#errorCode); return errorCode; } while (0)

#define RETURN_OLD_ERROR(errorCode) \
    do { DxErrorStack_Add(__FILE__,__LINE__,__FUNCTION__,errorCode,#errorCode); return errorCode; } while (0)

#define GOTO_END_WITH_NEW_ERROR(errorCode) \
    do { ADD_NEW_ERROR(errorCode); goto end; } while (0)

#define GOTO_END_WITH_OLD_ERROR(errorCode) \
    do { ADD_OLD_ERROR(errorCode); goto end; } while (0)


void DxErrorStack_Add(const DxChar* fileName, DxUint32 lineNum, const DxChar* funcName, DxUint32 errorCode, const DxChar* errorName);
void DxErrorStack_Clear(void);
void DxErrorStack_Log(DxUint32 moduleCode, DxUint32 debugLevel);
void DxErrorStack_Delete(void);
void DxErrorStack_Terminate(void);

#else
#define LOG_ERROR_STACK()  
#define CLEAR_ERROR()      

#define ADD_NEW_ERROR(errorCode) result = errorCode
#define ADD_OLD_ERROR(errorCode) result = errorCode
#define RETURN_NEW_ERROR(errorCode) return errorCode
#define RETURN_OLD_ERROR(errorCode) return errorCode

#define DxErrorStack_Add(fileName, lineNum, funcName, errorCode, errorName)
#define DxErrorStack_Clear()
#define DxErrorStack_Log(moduleCode, debugLevel)
#define DxErrorStack_Delete()
#define DxErrorStack_Terminate()
#endif

#define GOTO_END_WITH_NEW_ERROR(errorCode) \
    do { ADD_NEW_ERROR(errorCode); goto end; } while (0)

#define GOTO_END_WITH_OLD_ERROR(errorCode) \
    do { ADD_OLD_ERROR(errorCode); goto end; } while (0)

#ifdef DX_USE_LEGACY_VOS
#define DX_VOS_OK               DX_SUCCESS
#define DX_VOS_FAIL				DX_FAILURE
#define DX_VOS_CONVERSION_FAIL  DX_INVALID_FORMAT
#define DX_VOS_BAD_ARGUMENTS    DX_BAD_ARGUMENTS
#endif

#ifdef __cplusplus
}
#endif

#endif /*_DX_VOS_ERROR_H*/
