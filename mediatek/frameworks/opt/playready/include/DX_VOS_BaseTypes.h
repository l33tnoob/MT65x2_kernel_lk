
 
 
/*! \file DX_VOS_basetypes.h
    \brief This file holds the basic VOS definitions which includes type & utility macros.
*/


#ifndef _DX_VOS_BASETYPES_H
#define _DX_VOS_BASETYPES_H
#ifdef __cplusplus
extern "C" {
#endif

#include "DX_VOS_config.h"
#include "DX_VOS_Errors.h"

// This DX_SECURITY_LEVEL value configures the security level of several DRM system features.
// Currently The affected features are:
//  - ConfigFile - DxConfigFile_GetSecureString (and others) are disabled when DX_SECURITY_LEVEL < 3
//  - AutoProvisioning - AutoProvisioninig of OMAV2 & PlayReady are disabled when DX_SECURITY_LEVEL < 3
#ifndef DX_SECURITY_LEVEL
#define DX_SECURITY_LEVEL     1
#endif

#define DISABLE_VOS_DBG_PRINT 1

#define DxUint_t      DxUint
#define DxUint8_t     DxUint8
#define DxUint16_t    DxUint16
#define DxUint32_t  DxUint32
#define DxUint64_t	DxUint64

#define DxInt_t       DxInt
#define DxInt8_t      DxInt8
#define DxInt16_t     DxInt16
#define DxInt32_t     DxInt32
#define DxInt64_t     DxInt64

#define DxChar_t      DxChar
#define DxWideChar_t  DxWideChar
#define DxWideChar32_t  DxWideChar32

#define DxByte_t      DxByte

#define DxBool_t      DxBool

#define DxError_t   DxStatus
#define DxSize_t  unsigned int

#define IN
#define OUT
#define INOUT

#define DX_MAX_UINT64		0xFFFFFFFFFFFFFFFFULL
#define DX_MAX_INT64		0x7FFFFFFFFFFFFFFFLL

#define DX_MAX_UINT32		0xFFFFFFFFUL
#define DX_MAX_INT32		0x7FFFFFFFL
#define DX_MIN_INT32		0x80000000L
#define DX_MAX_UINT16		0xFFFFU
#define DX_MAX_UINT8		0xFFU

#define DX_BITS_IN_BYTE     8
#define DX_BITS_IN_DWORD    32
#define DX_GET_LSBS(value, numOfBits) (value & ((1UL << numOfBits) - 1))

#define DX_MAX_UINT			((DxUint)(-1))
#define DX_MAX(a,b)			((a) > (b) ? (a) : (b))
#define DX_MIN(a,b)			((a) < (b) ? (a) : (b))

#define DX_DIVIDE_ROUND_UP(num, divider)			(((num) + (divider) - 1) / (divider))

#define DX_IGNORE_RETURN_VALUE(cmd)     cmd
typedef void (*DxMemDeallocator)(void**);

#ifdef _DEBUG
	#define ONLY_IN_DEBUG(cmd)	cmd
#else
	#define ONLY_IN_DEBUG(cmd)
#endif

#ifdef DX_AUTO_CHECK_PERFORMANCE
#define DX_CHECK_PERFORMANCE
#endif

#ifdef DX_CHECK_PERFORMANCE
    #include "TestUtils/DxPerfMon.h"
    #ifdef DX_AUTO_CHECK_PERFORMANCE
        #define DX_PERFORMANCE_STUB DxStatus profileStub = PERFORMANCE_START("");
        #define DX_PERFORMANCE_END_STUB PERFORMANCE_END
    #else
        #define DX_PERFORMANCE_STUB
        #define DX_PERFORMANCE_END_STUB
    #endif
#else
    #define PERFORMANCE_FILE
    #define PERFORMANCE_START(tag)   DX_SUCCESS
    #define PERFORMANCE_END
    #define PERFORMANCE_MEASURE(cmd)    do { cmd; } while(0)
    #define DxPerfMon_StartThreadMeasure()

    #define DxPerfMon_StartMeasure(outputFileName)
    #define DxPerfMon_StopMeasure()
    #define DxPerfMon_SaveFilesRegistry(outputFileName)
    #define DxPerfMon_Flush()
    #define DX_PERFORMANCE_STUB
    #define DX_PERFORMANCE_END_STUB
#endif

#include "DX_VOS_DbgPrint.h"

#ifdef DX_DEBUG_PRINT
#include "DX_VOS_DbgVosModulesCodes.h"

#define RETURN_VAR_STATUS(errorCode)	\
	do {	\
		DxStatus localCode = (DxStatus)errorCode;	\
		if (localCode != DX_SUCCESS)	\
			DX_VOS_DebugPrint(DX_DBG_MODULE, __FILE__, __LINE__, __FUNCTION__,	\
				DX_ERROR_PROPAGATION_LEVEL, "Returning Error Code: 0x%08X", localCode);	\
		DX_RETURN(localCode);	\
	} while (0)

#define RETURN_VAR_WARNING(errorCode, warningCode)	\
	do {	\
	    DxUint32 localCode = (DxUint32)errorCode;	\
	    if (localCode != DX_SUCCESS)	\
	        DX_VOS_DebugPrint(DX_DBG_MODULE, __FILE__, __LINE__, __FUNCTION__,	\
	            localCode == warningCode ? DX_DBGPRINT_WARNING_LEVEL : DX_ERROR_PROPAGATION_LEVEL, \
                "Returning %s Code: 0x%08X", localCode == warningCode ? "Warning" : "Error", localCode);	\
	    DX_RETURN(localCode);	\
	} while (0)

#define RETURN_CONST_STATUS(errorCode)	\
	do {	\
		DX_VOS_DebugPrint((DxUint32)DX_DBG_MODULE, __FILE__, __LINE__, __FUNCTION__,	\
			DX_ERROR_PROPAGATION_LEVEL, "Returning Error Code: %s (0x%08X)", #errorCode, errorCode);	\
		DX_RETURN(errorCode);	\
	} while (0)

#define RETURN_CONST_WARNING(errorCode)	\
    do {	\
        DX_VOS_DebugPrint(	DX_DBG_MODULE, __FILE__, __LINE__, __FUNCTION__,	\
            DX_DBGPRINT_WARNING_LEVEL, "Returning Warning Code: %s (0x%08X)", #errorCode, errorCode);	\
        DX_RETURN(errorCode);	\
    } while (0)

#define GOTO_END_WITH_CONST_STATUS(errorCode)\
    do {	\
        DX_VOS_DebugPrint(DX_DBG_MODULE, __FILE__, __LINE__, __FUNCTION__,	\
            DX_ERROR_PROPAGATION_LEVEL, "Returning Error Code: %s (0x%08X)", #errorCode, errorCode);	\
        result = errorCode; \
        goto end;	\
    } while (0)

#define GOTO_END_WITH_VAR_STATUS(errorCode)\
    do {	\
	    result = errorCode;	\
		DX_VOS_DebugPrint(	DX_DBG_MODULE, __FILE__, __LINE__, __FUNCTION__,	\
	        DX_ERROR_PROPAGATION_LEVEL, "Returning Error Code: (0x%08X)", result);	\
        goto end;   \
    } while (0)

#define GOTO_END_WITH_CONST_WARNING(errorCode)\
    do {	\
        DX_VOS_DebugPrint(DX_DBG_MODULE, __FILE__, __LINE__, __FUNCTION__,	\
            DX_ERROR_PROPAGATION_LEVEL, "Returning Warning Code: %s (0x%08X)", #errorCode, errorCode);	\
        result = errorCode; \
        goto end;	\
    } while (0)

#define GOTO_END_WITH_VAR_WARNING(errorCode, warningCode)\
    do {	\
        result = errorCode;	\
        if (result != DX_SUCCESS)	\
            DX_VOS_DebugPrint(DX_DBG_MODULE, __FILE__, __LINE__, __FUNCTION__,	\
            result == warningCode ? DX_DBGPRINT_WARNING_LEVEL : DX_ERROR_PROPAGATION_LEVEL, \
            "Returning %s Code: 0x%08X", result == warningCode ? "Warning" : "Error", result);	\
        goto end;   \
    } while (0)


#define DISABLE_VOS_MODULE_PRINT(ModuleCode) \
    do {	\
        if (DISABLE_VOS_DBG_PRINT)	\
        ModuleCode = DX_VOS_DbgPrint_DisableModulePrinting(DX_DBG_MODULE_VOS); \
    } while (0)

#define ENABLE_VOS_MODULE_PRINT(ModuleCode) \
    do {	\
        if (DISABLE_VOS_DBG_PRINT)	\
        DX_VOS_DbgPrint_SetModulesMask(ModuleCode);   \
    } while (0)

#define DX_RETURN(value)  do { DX_PERFORMANCE_END_STUB; DX_STACK_END; return value; } while(0)
#define DX_DECLARE(type, varName, initialValue) DX_PERFORMANCE_STUB type varName = initialValue 
#else
#define RETURN_VAR_STATUS(errorCode)	return errorCode
#define RETURN_CONST_STATUS(errorCode)	return errorCode
#define RETURN_CONST_WARNING(errorCode)	return errorCode
#define RETURN_VAR_WARNING(errorCode, warningCode)	return errorCode
#define GOTO_END_WITH_CONST_STATUS(errorCode) do { result = errorCode; goto end; } while (0)
#define GOTO_END_WITH_CONST_WARNING(errorCode) do { result = errorCode; goto end; } while (0)
#define GOTO_END_WITH_VAR_STATUS(errorCode) do { result = errorCode; goto end; } while (0)
#define GOTO_END_WITH_VAR_WARNING(errorCode, warningCode) do { result = errorCode; goto end; } while (0)
#define DX_DECLARE(type, varName, initialValue) type varName = initialValue
#define DX_RETURN(value)  return value
#endif

#ifdef DX_THREADX
#define DX_UNREFERENCED(varName) varName = varName
#else
#define DX_UNREFERENCED(varName) varName
#endif

#define RETURN_IF_ALLOC_FAILED(ptr)	\
    if (ptr == DX_NULL) RETURN_NEW_ERROR(DX_MEM_ALLOCATION_ERROR)

#define GOTO_END_IF_ALLOC_FAILED(ptr)	\
    if (ptr == DX_NULL) GOTO_END_WITH_CONST_STATUS(DX_MEM_ALLOCATION_ERROR)

#define DX_VALID_PTR_DATA(data, length) ((data) != DX_NULL || (length) == 0)

#if !defined(DX_NO_CHECK_PARAMS) && !defined(DX_CHECK_PARAMS)
#define DX_CHECK_PARAMS
#endif

#ifdef DX_CHECK_PARAMS
#define DX_ASSERT_PARAM(condition)  if (!(condition)) RETURN_NEW_ERROR(DX_BAD_ARGUMENTS)
#define DX_ASSERT(condition)        if (!(condition)) RETURN_CONST_STATUS(DX_ASSERTION_ERROR)
#define DX_GOTO_ASSERT(condition)        if (!(condition)) GOTO_END_WITH_CONST_STATUS(DX_ASSERTION_ERROR)
#else
#define DX_ASSERT_PARAM(condition) 
#define DX_ASSERT(condition)
#define DX_GOTO_ASSERT(condition)
#endif

#define CSTATIC_ASSERT( x ) switch(0){case 0:case (x):;}

#ifdef DX_DEBUG_STACK
void DX_VOS_StackStart(void* stackStart);
void DX_VOS_StackEnd(void* stackEnd);
DxUint32 DX_VOS_GetStackUsage();

#define DX_STACK_START  {   DxUint32 stackStart = 0; DX_VOS_StackStart(&stackStart);  }
#define DX_STACK_END    {   DxUint32 stackEnd = 0; DX_VOS_StackEnd(&stackEnd);    }
#else
#define DX_STACK_START  
#define DX_STACK_END       
#define DX_VOS_GetStackUsage()  0
#endif

typedef enum {
    DX_OPTIONAL,
    DX_MANDATORY
} DxFieldRequirement;

#define DX_VERIFY_PARAM(condition) if (!(condition)) RETURN_NEW_ERROR(DX_BAD_ARGUMENTS)

/* RMI hints macros */
#define DX_FUNC
#define DX_SECURE_FUNC(securityLevel)
#define DX_OUT
#define DX_BYVAL
#define DX_LEN(x)
#define DX_UNIT(x)
#define DX_FIXEDARRAY
#define DX_EXPORT_BEGIN
#define DX_EXPORT_END
#define DX_ENUM

#define DX_ITEMS_IN_ARRAY(array) (sizeof(array)/sizeof(array[0]))
#define DX_OFFSET_OF(s,m)   ((DxUint32)&(((s *)0)->m))

#ifdef LITTLE__ENDIAN

	#ifdef VXWORKS_ENVIRONMENT

		#define DX_HTONS(x)   htons
		#define DX_NTOHS(x)   ntohs
		#define DX_HTONL(x)   htonl
		#define DX_NTOHL(x)   ntohl
	#else
		#define DX_HTONS(x) (((((unsigned short)(x)) >> 8) & 0xff) | \
			((((unsigned short)(x)) & 0xff) << 8))
		#define DX_NTOHS(x) (((((unsigned short)(x)) >> 8) & 0xff) | \
			((((unsigned short)(x)) & 0xff) << 8) )
		#define DX_HTONL(x) ((((x)>>24) & 0xffL) | (((x)>>8) & 0xff00L) | \
			(((x)<<8) & 0xff0000L) | (((x)<<24) & 0xff000000L))
		#define DX_NTOHL(x) ((((x)>>24) & 0xffL) | (((x)>>8) & 0xff00L) | \
			(((x)<<8) & 0xff0000L) | (((x)<<24) & 0xff000000L))
	#endif
#elif defined(BIG__ENDIAN)

	#define DX_HTONS(x) (x)
	#define DX_NTOHS(x) (x)
	#define DX_HTONL(x) (x)
	#define DX_NTOHL(x) (x)
#else
	#error neither BIG__ENDIAN nor LITTLE__ENDIAN defined, cannot compile
#endif

#define DX_VOS_INVERSE_UINT32_BYTES( val ) \
   ( ((val) >> 24) | (((val) & 0x00FF0000) >> 8) | (((val) & 0x0000FF00) << 8) | (((val) & 0x000000FF) << 24) )

#define DX_VOS_INVERSE_UINT16_BYTES( val ) \
	( ((val) << 8) | (((val) & 0x0000FF00) >> 8))

#define DX_VOS_INVERSE_UINT64_BYTES( val ) \
	DX_VOS_INVERSE_UINT32_BYTES(val >> 32) | ((DX_VOS_INVERSE_UINT32_BYTES(val & 0xffffffff))<< 32)



/* Set of Macros similar to the HTONS/L, NTOHS/L ones but converting to/from little endian instead of big endian*/
#ifdef LITTLE__ENDIAN /* BIG_ENDIAN*/
#define DX_LITTLE_ENDIAN_TO_HOST_16BIT(x) (x)
#define DX_LITTLE_ENDIAN_TO_HOST_32BIT(x) (x)
#define DX_LITTLE_ENDIAN_TO_HOST_64BIT(x) (x)
#define DX_HOST_TO_LITTLE_ENDIAN_16BIT(x) (x)
#define DX_HOST_TO_LITTLE_ENDIAN_32BIT(x) (x)
#define DX_HOST_TO_LITTLE_ENDIAN_64BIT(x) (x)
#elif defined(BIG__ENDIAN)

#define DX_LITTLE_ENDIAN_TO_HOST_16BIT(x) (DX_VOS_INVERSE_UINT16_BYTES(x))
#define DX_LITTLE_ENDIAN_TO_HOST_32BIT(x) (DX_VOS_INVERSE_UINT32_BYTES(x))
#define DX_LITTLE_ENDIAN_TO_HOST_64BIT(x) (DX_VOS_INVERSE_UINT64_BYTES(x))
#define DX_HOST_TO_LITTLE_ENDIAN_16BIT(x) (DX_VOS_INVERSE_UINT16_BYTES(x))
#define DX_HOST_TO_LITTLE_ENDIAN_32BIT(x) (DX_VOS_INVERSE_UINT32_BYTES(x))
#define DX_HOST_TO_LITTLE_ENDIAN_64BIT(x) (DX_VOS_INVERSE_UINT64_BYTES(x))

#else
#error neither BIG__ENDIAN nor LITTLE__ENDIAN defined, cannot compile
#endif




/* code section added at FW's request*/

#ifndef LITTLE__ENDIAN /* BIG_ENDIAN*/

#define DX_VOS_CHANGE_WORD_ENDIANNESS(val) ( DX_VOS_INVERSE_UINT32_BYTES(val) )

#define DX_VOS_CHANGE_ENDIANNESS(startAddr, numOfWords) \
do \
{ \
  DxUint32  count; \
  for(count = 0; count < numOfWords; count ++) \
  { \
    *((DxUint32_t *)startAddr + count) = \
        DX_VOS_INVERSE_UINT32_BYTES(*((DxUint32_t *)startAddr + count)); \
  } \
}while(0)

#else /*LITTLE__ENDIAN*/

#define DX_VOS_CHANGE_WORD_ENDIANNESS(val) (val)
#define DX_VOS_CHANGE_ENDIANNESS(startAddr, numOfWords)

#endif

/* end of FW code section*/
 


#ifdef DX_USE_LEGACY_VOS
typedef DxStatus DxVosResult_t; 

#endif
#ifdef __cplusplus
}
#endif

#endif /* ifndef _DX_VOS_BASETYPES_H */



