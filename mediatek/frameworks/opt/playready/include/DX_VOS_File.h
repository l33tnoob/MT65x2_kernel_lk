
 
 
#ifndef _DX_VOS_FILE_H
#define _DX_VOS_FILE_H

/*! \file DX_VOS_FILE.h
This module enables standard file access operations. 
These function are suitable for files up to 2GB.

The user of these functions may assume:
- Pointers should not be NULL (unless specified otherwise).
  A Buffer pointer parameter may be NULL only if its size is 0.
- File Handles should not be NULL (unless specified otherwise).
- Enum parameters should have valid values.
- The value of output parameter has no effect.
- All const DxChar* parameters should be UTF8 encoded.

All functions returns DxStatus that should be:
- DX_SUCCESS - on success 
- DX_BAD_ARGUMENTS - if one of the parameters is not valid (see rules mentioned earlier)
- DX_VOS_FILE_ERROR - if operation failure.

if a function return another error code it will be stated explicitly in the function documentation.
*/

#include "DX_VOS_BaseTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DX_SEEK_SET 0
#define DX_SEEK_CUR 1
#define DX_SEEK_END 2

    typedef enum {
        DX_FILE_CREATE = 1,
        DX_FILE_TRUNCATE = 2,
        DX_FILE_READ = 4,
        DX_FILE_WRITE = 8,
        DX_FILE_OPEN_EXISITING = DX_FILE_READ | DX_FILE_WRITE,
        DX_FILE_CREATE_NEW = DX_FILE_CREATE | DX_FILE_TRUNCATE | DX_FILE_READ | DX_FILE_WRITE,
        DX_FILE_READ_ONLY = DX_FILE_READ,
        DX_FILE_WRITE_ONLY = DX_FILE_CREATE | DX_FILE_TRUNCATE | DX_FILE_WRITE,
        DX_FILE_OPEN_ALWAYS = DX_FILE_CREATE | DX_FILE_READ | DX_FILE_WRITE,
        DX_FILE_OPEN_ALWAYS_READ_ONLY = DX_FILE_CREATE | DX_FILE_READ,
        DX_FILE_OPEN_ALWAYS_WRITE_ONLY = DX_FILE_CREATE | DX_FILE_WRITE,
        DX_FILE_TRUNCATE_AND_WRITE= DX_FILE_TRUNCATE | DX_FILE_WRITE,
        DX_FILE_TRUNCATE_READ_WRITE = DX_FILE_TRUNCATE | DX_FILE_READ | DX_FILE_WRITE,
    } EDxOpenMode;

    typedef enum {
        DX_NO_SHARE,
        DX_SHARE_READ,
        DX_SHARE_WRITE,
        DX_SHARE_READ_WRITE,
        DX_NUMBER_OF_FILE_SHARE_MODES
    } EDxShareMode;
	/*************************** Typedefs *********************/

	typedef struct _DxVosFile*  		  DxVosFile;

	/*************************** Functions *********************/

	/*!	Opens aFileName and associates it with aFileHandle.
        If aFileName consist of %s (only one time) the %s will be replaces with current timestamp.
	\note	If the directory in which the file to be created doesn't exist it 
			should be created by this function if the file is opened for writing
	*/
	 DxStatus DX_VOS_FOpen(
		DxVosFile* Handle,       /*!< [out] Handle to the opened file */
		const DxChar *aFileName, /*!< [in] A pointer to the full path of the file name */
		const DxChar *aMode		 /*!< [in] A pointer to the opening mode of the file
											The mode syntax is:	<r|w>[+][b|t].

											Examples: "r", "r+", "w+t", "wb"

											When w or a is used the file is created if it doesn't exist.
											use + to open the file with read/write privileges. */
		);

    DxStatus DX_VOS_FileOpen(DxVosFile* aFileHandle, const DxChar *aFileName, DxUint32 openMode, DxUint32 shareMode);

    DxStatus DX_VOS_FileOpenWithTimeStamp(DxVosFile* aFileHandle, const DxChar *aFileName, DxUint32 openMode, DxUint32 shareMode);

	DxStatus DX_VOS_CreateFileHandleFromOsHandle(DxVosFile* aFileHandle, void* osHandle);

    DxStatus DX_VOS_GetOsHandleFromFileHandle(DxVosFile aFileHandle, void** osHandle);

	/*!	Closes the file.
	\note   If the File Handle is NULL it is not considered as error.
	*/
	DxStatus DX_VOS_FClose(DxVosFile aFileHandle); 
    DxStatus DX_VOS_FileClose(DxVosFile* aFileHandle); 


    DxBool DX_VOS_IsFileExists(const DxChar* fileName);
	/*!	Reads data from file.
	\note If not all the requested bytes can be read and aReadBytesPtr is NULL the function returns DX_VOS_FILE_ERROR.
	\note If the operation fails the file pointer doesn't necessarily stay in the same place.
	***/
	DxStatus DX_VOS_FReadEx(
		DxVosFile aFileHandle, /*!< [in]  A handle to input file*/
		void* aBuf, /*!< [out] Pointer to buffer. Buffer must be at least aSize bytes long. */
		DxUint32 aSize, /*!< [in]  Number of bytes to be read. 0 is a valid value.*/
		DxUint32* aReadBytesPtr /*!< [out] if not NULL, contains the number of bytes that were actually read. */
		);

	/*! This is a convenience macro that can be used when all bytes requested should be read successfully */
	#define DX_VOS_FRead(aFileHandle, aBuf, aSize) DX_VOS_FReadEx(aFileHandle, aBuf, aSize, DX_NULL)
		
	/*!	Writes data to file.
	\note If DX_VOS_FILE_ERROR is returned the number of bytes actually written is unknown.
	***/
	DxStatus DX_VOS_FWrite(
		DxVosFile aFileHandle, /*!< [in] A handle to output file.*/
		const void *aBuf, /*!< [in] A Pointer to Buffer that contains the data to write.*/
		DxUint32 aSize /*!< [in] Number of bytes to write.*/
		);

	/*!	Writes aSize bytes (with zero value) to the file (\x00 * aSize).
	\note If DX_VOS_FILE_ERROR is returned the number of bytes actually written is unknown.
	***/
	DxStatus DX_VOS_FWriteZeros(
		DxVosFile aFileHandle, /*!< [in] A handle to output file.*/
		DxUint32 aSize /*!< [in] Number of zero bytes to write.*/
		);

	/*!	Sets the file pointer to a new position.
	\note If the operation fails the file pointer doesn't necessarily stay in the same place.
	*/
	DxStatus DX_VOS_FSeekEx(
		DxVosFile aFileHandle, /*!< [in] A Pointer to file handle.*/
		DxInt32 aOffset, /*!< [in] The offset in bytes from initial position*/
		DxInt aOrigin /*!< [in] Initial position: may be one of the following:
							- DX_SEEK_CUR  Current position of file pointer
							- DX_SEEK_END  End of file
							- DX_SEEK_SET  Beginning of file*/
		
		);
	
	/*! This is a convenience macro that can be used when you want to seek to an absolute 
	position in the file */
	#define DX_VOS_FSeek(aFileHandle, aOffset) DX_VOS_FSeekEx(aFileHandle, aOffset, DX_SEEK_SET)


	/*!	Retrieves the current position of a file pointer.
	\note if operation fails aCurPosPtr will be 0.
	*/
	DxStatus DX_VOS_FTell(	DxVosFile aFileHandle, DxUint32* aCurPosPtr);

	/*!	Retrieves file size.
	\note if operation fails aFileSizePtr will be 0.
	*/
	DxStatus DX_VOS_FGetFileSize(DxVosFile aFileHandle, DxUint32* aFileSizePtr);

#ifndef DX_NO_LARGE_FILE_SUPPORT

/*!	Sets the file pointer to a new position.
	\note If the operation fails the file pointer doesn't necessarily stay in the same place.
	*/
	DxStatus DX_VOS_FSeekEx64(
		DxVosFile aFileHandle, /*!< [in] A Pointer to file handle.*/
		DxInt64 aOffset, /*!< [in] The offset in bytes from initial position*/
		DxInt aOrigin /*!< [in] Initial position: may be one of the following:
							- DX_SEEK_CUR  Current position of file pointer
							- DX_SEEK_END  End of file
							- DX_SEEK_SET  Beginning of file*/
		
		);
	
	/*! This is a convenience macro that can be used when you want to seek to an absolute 
	position in the file */
	#define DX_VOS_FSeek64(aFileHandle, aOffset) DX_VOS_FSeekEx64(aFileHandle, aOffset, DX_SEEK_SET)

	/*!	Retrieves the current position of a file pointer.
	\note if operation fails aCurPosPtr will be 0.
	*/
	DxStatus DX_VOS_FTell64(	DxVosFile aFileHandle, DxUint64* aCurPosPtr);



	/*!	Retrieves file size.
	\note if operation fails aFileSizePtr will be 0.
	*/
	DxStatus DX_VOS_FGetFileSize64(DxVosFile aFileHandle, DxUint64* aFileSizePtr);

#endif


	/*!	Tests for end-of-file.
	\return    
	if current position is not end of file, returns DX_SUCCESS. 
	if current position is at end of file, returns DX_VOS_END_OF_FILE.
	if aFileHandle equals NULL, returns DX_BAD_ARGUMENTS.
	if operation fails, returns DX_VOS_FILE_ERROR.
	*/
	DxStatus DX_VOS_FEof(DxVosFile aFileHandle);

	/*!	Retrieves a line ("\n" terminated) from a file. 
	The retrieved line will contain the terminating "\n" (if exists) but will not contain any "\r" characters.
	\note if line is longer than aLength - 1, aLength - 1 characters  will be filled and
	DX_BUFFER_IS_NOT_BIG_ENOUGH will be returned.
	If end of file was reached an empty string will be returned.
	*/
	DxStatus DX_VOS_FGets(
		DxVosFile aFileHandle, /*!< [in] A pointer to file handle.*/
		DxChar *aString, /*!< [out] Storage location for data. */
		DxUint aLength  /*!< [in] Size of the aString buffer in bytes. */
		);

	/*!	Saves all buffered data to disk. */
	DxStatus DX_VOS_FFlush(DxVosFile aFileHandle);

	/*!	Deletes the file. */
	DxStatus DX_VOS_FDelete(const DxChar *aFileName);

	/*!	Renames a file. */
	DxStatus DX_VOS_FRename(const DxChar *aOrigName, const DxChar *aNewName);

	/*!
	Creates a new directory.
	\note If directory already exists, returns DX_SUCCESS.
	All directories in the path will be created if not exist.
	***/
	DxStatus  DX_VOS_CreateDirectory(const DxChar *aDirName);

	/*! Enumerates all files and directories that are in the specified directory 
	\note is DirsBuff or FilesBuff is not big enough to contain all files DX_BUFFER_IS_NOT_BIG_ENOUGH
	will be returned and DirBuffSize & FilesBuffSize will hold the required buffer sizes.
	*/
	DxStatus DX_VOS_EnumerateDir(
		const DxChar *aDirName, /*!< [in] A pointer to the full path of the directory to be enumerated. */
		DxChar* DirsBuff, /*!< [out] pointer to buffer that will hold the list of sub-directories of aDirName.
						  This list will not contain "." and "..". The directories in the list are NULL separated.
						  The list is terminated by an empty string (two adjacent NULLs). You can iterate through
						  the strings in the list using DX_VOS_NextStringInList() & DX_VOS_FindStringInList().
						  Can be NULL if the list of sub-directories in not required */
		DxUint32* DirsBuffSize, /*!< [inout] on entry this parameter indicates the size in bytes of DirsBuff.
								  on return the parameter indicates the number of used bytes in DirsBuff.
								  If the value on return is larger then the value on entry, it means that the
								  supplied buffer was not big enough and the parameter's value indicates what
								  should be the buffer size. In this case DX_BUFFER_IS_NOT_BIG_ENOUGH will be returned.
								  Can be NULL only if DirsBuff is NULL */
		DxUint32* NumOfDirs, /*!< [out] number of sub-directories of aDirName.
							   Can be NULL if the number of sub-directories is not required */
		DxChar* FilesBuff, /*!< [out] pointer to buffer that will hold the list of files in aDirName.
						   behaves exactly as DirsBuff.*/
		DxUint32* FilesBuffSize, /*!< [inout] on entry this parameter indicates the size in bytes of FilesBuff.
								  on return the parameter indicates the number of used bytes in FilesBuff. 
								  see: DirsBuffSize for more details.*/
		DxUint32* NumOfFiles  /*!< [out] number of files of aDirName.
								  Can be NULL if the number of files is not required */
		);

    DxStatus DX_VOS_DeepDirCopy(const DxChar* source, const DxChar* dest);

    /*!	Copies the file at the given path to the new path, overwriting an existing file only if the overwrite flag is set. 
        If the overwrite flag is false and the destination file exists the function will fail.
    */
    DxStatus DX_VOS_CopyFile (const DxChar *ExistingFileName, /*!< [in] the path to copy the file from*/
                              const DxChar *NewFileName,/*!< [in] the new path to copy the file to */
                              DxBool bOverwrite); /*!< [in] copy will overwrite an existing file at the new path if true*/

	/*!	Deletes an existing directory. 
	\note if the directory contains sub-directories or files all sub-tree is deleted.
	*/
	DxStatus  DX_VOS_RemoveDirectory(const DxChar *aDirName);

	/*!
	Concatenates a new name to a file path using the correct separator for the current system.
	For example, if aDirName1 = "/ABC" and aDirName2 = "123", the routine returns 
	"/ABC/123" for a Unix system. 
	\note if the aDirName1 is not big enough DX_BUFFER_IS_NOT_BIG_ENOUGH will be returned
	and aDirName1 we be left unchanged.
	*/                          

	DxStatus DX_VOS_DirNCat(
		DxChar *aDirName1,			/*!< [in] Directory name in a buffer that is large enough to include the new path name.*/
		DxUint32 aBuf_size,		/*!< [in] Size of the aDirName1 buffer.*/
		const DxChar *aDirName2		/*!< [in] String to append. */ 
		);

    const DxChar* DX_VOS_GetBaseFileName(const DxChar* fileName);

	/*!
	*\brief  formats a directory path into the specified buffer.
	*/
	DxStatus DX_VOS_BuildPathStr(		
		DxChar* Buff,		/*!< [out] Buffer for the formatted string */
		DxUint BuffSize,	/*!< [in] Size of buffer in bytes */
		const DxChar * fmt, /*!< [in] A format string similar to printf that accepts the following values: 
								- %s   Insert the string.
								- %d   Insert a divider for this system.
								- %c   insert a string representation of 8-bit int.
								- %ul  insert a string representation of 32-bit int.
								- %uh  insert a string representation of 16-bit int.
							*/
		...);

	const DxChar* DX_VOS_FullPathToFileName(const DxChar *aFullPathName);

#ifdef DX_USE_LEGACY_VOS
typedef DxVosFile DxVosFileHandle_ptr;
/*	DxStatus DX_VOS_FileSessionDelete(void); */
DxVosFile DX_VOS_Fopen(const DxChar* fileName, const DxChar* mode, DxStatus* errorCode);
DxStatus  DX_VOS_GetDirectoryFilesNumber(const DxChar *aDirName,DxUint32* aFilesNumber);
DxStatus  DX_VOS_GenericGetDirectoryListFiles(const DxInt8 *aDirName_ptr,
						       DxInt8       *aBuffer_ptr,
						       DxUint32     *aBufferSize_ptr);
DxChar *  DX_VOS_BuildPath(DxChar * fmt, void * ptr0, void * ptr1, void * ptr2 ,void * ptr3, 
									void * ptr4, void * ptr5, void * ptr6,void * ptr7 );
const DxInt8* DX_VOS_Convert_FullPath_ToName(const DxChar *aFullPathName);

#define DX_VOS_GetDividerSize()	1
#define DX_VOS_Fclose DX_VOS_FClose 
#define DX_VOS_Fread(aBuf, aSize, aNitems, aFileHandle) DX_VOS_FReadEx(aFileHandle, aBuf, (aSize) * (aNitems), (DxUint32*)DX_NULL)
DxStatus DX_VOS_FreadEx(void* aBuf, DxUint32 aSize, DxUint32 aNitems, DxUint* aReadBytesPtr, DxVosFile aFileHandle);
#define DX_VOS_Fdelete DX_VOS_FDelete 
#define DX_VOS_Fwrite(aBuf, aSize, aNitems, aFileHandle) DX_VOS_FWrite(aFileHandle, aBuf, (aSize) * (aNitems))
#define DX_VOS_Fdelete DX_VOS_FDelete 
#define DX_VOS_Fseek DX_VOS_FSeek 
#define DX_VOS_FseekEx DX_VOS_FSeekEx 
#define DX_VOS_Fflush DX_VOS_FFlush 
#define DX_VOS_FgetFileSize DX_VOS_FGetFileSize
#define DX_VOS_Feof DX_VOS_FEof
#define DX_VOS_Ftell DX_VOS_FTell
#define DX_VOS_Fgets(aString, aLength, aFileHandle) DX_VOS_FGets(aFileHandle, aString, aLength)
#define DX_VOS_DirCat(str1, str2, len) DX_VOS_DirNCat(str1, len, str2)

#endif
#ifdef DX_USE_INSECURE_LEGACY_VOS
DxStatus  DX_VOS_GetDirectoryListFiles( const DxChar *aDirName,
						 DxChar       *aBuffer );
#endif
#ifdef __cplusplus
}
#endif
#endif /*_DX_VOS_FILE_H*/
