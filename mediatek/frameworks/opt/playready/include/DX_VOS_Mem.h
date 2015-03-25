
 
 
#ifndef _DX_VOS_MEM_H
#define _DX_VOS_MEM_H

/*! \file DX_VOS_Mem.h
This module provide memory allocation & deallocation utilities
and memory manipulation utilities.
In debug mode the module also provides memory leak detection system.
The memory leak detection system writes all allocations and deallocation
to debug log and keep track of current memory usage.

Functions that returns DxStatus type will return:
- DX_SUCCESS on success
- DX_BAD_ARGUMENTS if a pointer passed to them is NULL.
- DX_BUFFER_IS_NOT_BIG_ENOUGH in all copy functions if 
	aSourceSize is bigger than aTargetSize
	In this case only that data that can fit aTarget will be copied. 

A Buffer pointer parameter may be NULL only if its size is 0.
*/

#include "DX_VOS_BaseTypes.h"
#if defined(_DEBUG) || defined (DX_DEBUG_PRINT)
#include "DX_VOS_DbgPrint.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define DX_VOS_MemMalloc_(a,b) DX_VOS_MemMalloc(b)
#define DX_VOS_MemFree_(a,b) DX_VOS_MemFree(b)
#define DX_MALLOC(type)	(type*)DX_VOS_MemMalloc(sizeof(type))

#ifdef __cplusplus
#define DX_SAFE_DELETE(objPtr)  do { if ((objPtr) != DX_NULL) { delete objPtr; objPtr = DX_NULL; }} while (0)
#endif

#if defined(_DEBUG) && !defined(DX_NO_DEBUG_MEM) && !defined(DX_DEBUG_MEM)
#define DX_DEBUG_MEM
#endif

#if defined(DX_DEBUG_MEM) && defined(DX_DEBUG_PRINT)
extern DxUint32 DxCurrAllocNum;
#define DX_VOS_MemMallocNoFail(size) _DX_VOS_DebugMemMallocNoFail(__FILE__, __LINE__, __FUNCTION__, size)
#define DX_VOS_MemMalloc(size) _DX_VOS_DebugMemMalloc(__FILE__, __LINE__, __FUNCTION__, size)
#define DX_VOS_MemFree(buff) _DX_VOS_DebugMemFree(__FILE__, __LINE__, __FUNCTION__, buff)
#define DX_VOS_MemRealloc(buff, size) _DX_VOS_DebugMemRealloc(__FILE__, __LINE__, __FUNCTION__, buff, size)

DxStatus DX_VOS_MemFailAlloc(DxUint32 relativeAlloc);

void* _DX_VOS_DebugMemMallocNoFail(const DxChar* fileName, DxUint32 lineNum, 
                                            const DxChar* funcName, DxUint size);

void* _DX_VOS_DebugMemMalloc(const DxChar* fileName, DxUint32 lineNum, 
									  const DxChar* funcName, DxUint size);
void _DX_VOS_DebugMemFree(const DxChar* fileName, DxUint32 lineNum, 
									const DxChar* funcName, void* aBuff);
void* _DX_VOS_DebugMemRealloc(	const DxChar* fileName, DxUint32 lineNum, 
									   const DxChar* funcName, void* aBuff, DxUint size);

#define DX_DBG_LOG_OBJECT_CREATION(objType, objAddr)  DX_VOS_DebugPrint(DX_DBG_MEM_ALLOC, __FILE__, __LINE__, __FUNCTION__, DX_DBGPRINT_INFO_LEVEL, "%s Created at 0x%08X (No. %d)", #objType, objAddr, ++DxCurrAllocNum)
#define DX_DBG_LOG_OBJECT_DESTRUCTION(objType, objAddr)  DX_VOS_DebugPrint(DX_DBG_MEM_ALLOC, __FILE__, __LINE__, __FUNCTION__, DX_DBGPRINT_INFO_LEVEL, "%s Destroyed at 0x%08X", #objType, objAddr)

#else
#define DX_VOS_MemMalloc	        _DX_VOS_MemMalloc
#define DX_VOS_MemFree		        _DX_VOS_MemFree
#define DX_VOS_MemRealloc	        _DX_VOS_MemRealloc
#define DX_VOS_MemMallocNoFail      _DX_VOS_MemMalloc
#define DX_DBG_LOG_OBJECT_CREATION(objType, objAddr)  
#define DX_DBG_LOG_OBJECT_DESTRUCTION(objType, objAddr)  
#endif

void DX_VOS_DeallocMem (void** data);

#ifdef DX_DEBUG_MEM
/*! \return The number of bytes currently allocated. */
DxUint32 DX_VOS_GetMemUsage(void);
#else
#define DX_VOS_GetMemUsage() 0
#endif
	/*!	Allocates buffer with specified size.
	\return Allocated buffer or DX_NULL if allocation failed.
	**/
	void* _DX_VOS_MemMalloc(DxUint aSize	/*!< [in] Number of bytes to allocate. */
		);

	/*! Reallocates buffer with specified new size. The content of the old buffer
		is moved to the new location.
		\return Allocated buffer or NULL if allocation failed.
		\note if allocation failed the original buffer can be used.
	**/
	void* _DX_VOS_MemRealloc(
		void* aBuff,		/*!<[in] old buffer position. If NULL a new buffer will be allocated.*/
		DxUint aSize		/*!<[in] required size in bytes of new buffer. 
									 0 is not a valid value. in this case NULL will be
									 returned and the old buffer will not be released. */
		);

	/*!	Releases buffer's memory. If aBuf is NULL function does nothing. */
	void _DX_VOS_MemFree(void* aBuff);

	/*!	Copies buffer source to target.
	\note Overlapping ranges MUST be supported.
	**/
	DxStatus DX_VOS_MemCpy(
		void* aTarget,			/*!<[out] Pointer to target*/
		DxUint aTargetSize,	/*!<[in]  Size of target buffer in bytes.*/
		const void* aSource,	/*!<[in]  Pointer to source*/
		DxUint aSourceSize	/*!<[in]  Size of source buffer in bytes.*/
		);

	/*!	Copies buffer source to target without any parameters checks.
	\note Overlapping ranges MUST be supported.
	**/
	void DX_VOS_FastMemCpy(
		void* aTarget,			/*!<[out] Pointer to target*/
		const void* aSource,	/*!<[in]  Pointer to source*/
		DxUint aSourceSize	/*!<[in]  Number of bytes to copy. */
		);


    /*!	Copies buffer source to target reversing byte order during the copying.
    \note Overlapping ranges only supported if source and destination are the same.
    **/
    DxStatus DX_VOS_MemCpyReverse(
        void* aTarget,			/*!<[out] Pointer to target*/
        DxUint aTargetSize,	/*!<[in]  Size of target buffer in bytes.*/
        const void* aSource,	/*!<[in]  Pointer to source*/
        DxUint aSourceSize	/*!<[in]  Size of source buffer in bytes.*/
        );

    /*!	Copies buffer source to target reversing byte order during the copying without any parameters checks.
    \note Overlapping ranges MUST be supported.
    **/
    void DX_VOS_FastMemCpyReverse(
        void* aTarget,			/*!<[out] Pointer to target*/
        const void* aSource,	/*!<[in]  Pointer to source*/
        DxUint aSourceSize	/*!<[in]  Number of bytes to copy. */
        );


	/*!	Fills buffer with specific value. */
	void DX_VOS_MemSet6(
		void* aTarget,		/*!<[out] Pointer to target	*/
		DxUint8 aChar,		/*!<[in]  Value to be set	*/
		DxUint aTargetSize	/*!<[in]  Number of bytes to set with aChar. */
		);

    #define DX_VOS_MemSet   DX_VOS_MemSet6
	
    /*!	Sets buffer with  binary zeros (i.e. 0x00)	**/
	void DX_VOS_MemSetZero6(
		void* aTarget,		/*!<[out] A pointer to target.*/
		DxUint aTargetSize	/*!<[in]  Number of bytes to set with 0.*/
		);

    #define DX_VOS_MemSetZero   DX_VOS_MemSetZero6

	/*!
	Compares buffers (source buffer with target buffer).
	\return 
		0 - if buffers are identical.
		-1 - if first buffer is smaller then second buffer
		1 - if second buffer is smaller then first buffer
	\note DX_NULL is considered to be the smallest buffer.
	**/
	DxInt DX_VOS_MemCmp(
		const void* aTarget,	/*!<[in] A pointer to target buffer */
		const void* aSource,	/*!<[in] A pointer to source buffer*/
		DxUint aSize			/*!<[in] Number of bytes to compare.*/
		);

    DxByte* DX_VOS_MemChr(const DxByte* data, DxUint32 dataSize, DxByte chVal);
	/*!	Copies 16-bit ptr to 8-bit ptr(source to target) 
	\note Overlapping ranges are supported but if the target buffer is fully contained
	in the source buffer aTarget != aSource than the operation can't be performed
	and DX_BAD_ARGUMENTS will be returned. In this case buffers will not be changed.
	*/
	DxStatus DX_VOS_MemCopy16bitTo8bit(
		DxInt8* aTarget,			/*[out] Pointer to target*/
		DxUint aTargetSize,			/*[in]  Size of target buffer in bytes*/
		const DxInt16* aSource,	/*[in]  Pointer to source*/
		DxUint aSourceSize			/*[in]  Size of source buffer in bytes*/
		);

	/*!	Copies 8-bit ptr to 16-bit ptr(source to target).
		\note Overlapping ranges are supported but if the source buffer is fully contained
		in the target buffer aTarget != aSource than the operation can't be performed
		and DX_BAD_ARGUMENTS will be returned. In this case buffers will not be changed.
	*/
	DxStatus DX_VOS_MemCopy8bitTo16bit(
		DxInt16* aTarget,			/*[out] Pointer to target*/
		DxUint aTargetSize,			/*[in]  Size of target buffer in bytes*/
		const DxInt8* aSource,	/*[in]  A pointer to source*/
		DxUint aSourceSize			/*[in]  Size of source buffer in bytes*/
		);

#ifdef DX_USE_LEGACY_VOS

#undef DX_VOS_MemFree
#if defined(DX_DEBUG_MEM) && defined(DX_DEBUG_PRINT)
#define DX_VOS_MemFree(buff) (_DX_VOS_DebugMemFree(__FILE__, __LINE__, __FUNCTION__, buff), DX_SUCCESS)
#else
#define DX_VOS_MemFree(buff) (_DX_VOS_MemFree( (buff) ), DX_SUCCESS)
#endif

#define DX_VOS_MemCopy( to, from, sz )  \
  (DX_VOS_FastMemCpy( (to), (from), (sz) ), DX_SUCCESS)

#undef DX_VOS_MemSet
DxStatus DX_VOS_MemSet(void* aTarget, DxUint8 aChar, DxUint aSize);

#undef DX_VOS_MemSetZero
DxStatus DX_VOS_MemSetZero(void* aTarget, DxUint aSize);

#include "DX_VOS_String.h"

#endif

#ifdef __cplusplus
}
#endif

#endif /*_DX_VOS_MEM_H*/

