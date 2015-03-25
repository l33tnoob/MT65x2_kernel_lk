#ifndef DX_DRM_CLIENT_H
#define DX_DRM_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "DxDrmFile.h"
#include "DxDrmStream.h"
#include "DxDrmContent.h"
#include "DxImportStream.h"

//! HDxResponseResult is a handle associated with License Response. It is needed in case of required acknowledgment from the server side.
typedef void* HDxResponseResult; 


//! \file DxDrmClient.h
//! \brief DxDrmClient is the main module of the Discretix Multi-Scheme DRM.
//!
//! The provided functions are DRM scheme agnostic and they allow the calling application 
//! to treat all DRM schemes the same way.
//! The functions listed here are used to perform system level operations (such as:
//! Provisioning, DB Maintenance, etc...) and to create other handles that 
//! are used by other modules that provide additional functionality.
//! In order to use this module the DRM Client has to be initialized by calling DxDrmClient_Init().
//! At the end of use DxDrmClient_Terminate() must be called.
//! Most of the methods of the DRM Client return EDxDrmStatus as error code. 
//! This error code can be one of these generic codes:
//! - DX_SUCCESS - On Success
//! - DX_ERROR_BAD_ARGUMENTS - If at least one of the provided argument is invalid.
//! - DX_ERROR_MEMORY_ALLOCATION_FAILURE - If a memory allocation failed.
//! - DX_ERROR_GENERAL_FAILURE - Some internal error occurred.
//!
//! Any other specific error codes that are returns by the function will be listed 
//! in the function documentation.
//! When a function receives parameter of type "const DxChar*" it expects NULL terminated string. 
//! All the strings that are accepted or returned by the Discretix client are UTF8 encoded.



//! This function should be used to initialize the DRM Client.
//! It is mandatory to call it before any of the DRM Client APIs are called.
//! \note Use DxDrmClient_Terminate() at the end of use.
//! \return
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_MEMORY_ALLOCATION_FAILURE - if memory allocation failed.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_Init();

//! This function should be called when finished using the DRM Client.
DX_FUNC DRM_C_API void DxDrmClient_Terminate();

//! Determines whether the given mimeType is recognized by the DRM client.
//! If so, the content might be imported using CreateImportStream().
//! \param[in]  mimeType	    The mimeType to be checked.
//! \param[out] drmScheme	    If not Null it will hold the value of the DrmScheme to which the mimeType belongs (OmaV1, OmaV2...).
//! \return
//! - DX_TRUE - If mimeType was recognized by the DRM client.
//! - DX_FALSE - If mimeType was NOT recognized by the DRM client.
DX_FUNC DRM_C_API DxBool DxDrmClient_IsDrmMimeType(const DxChar* mimeType, EDxDrmScheme* drmScheme);


//! Identifies the DRM Scheme of the content in the file which is identified by fileName.
//! \param[in]  fileName            name of the content file 
//! \param[out] drmScheme           On exit, holds the value of the identified DRM Scheme.
//! \param[out] fileMimeType        On entry, empty buffer. On exit, holds the file MIME type.
//!                                 The file MIME type is the DRM container mime type.
//!                                 (i.e. application/vnd.oma.drm.content for OMA V1 content,
//!                                 application/vnd.oma.drm.dcf for OMA V2 DCF)
//!                                 This value may be NULL if file mime type is not required.
//! \param[in]  fileMimeTypeSize    the size in bytes of fileMimeType buffer.
//! \param[out] contentMimeType     On entry, empty buffer. On exit, holds the content mime type. 
//!                                 The content mime type is the protected content mime type
//!                                 (i.e. image/jpeg, audio/mp3, etc...). For OMA V2 multipart
//!                                 DCF this value will be the MIME type of the first content in
//!                                 the DCF.
//!                                 This value may be NULL if content mime type is not required.
//! \param[in]  contentMimeTypeSize The size in bytes of contentMimeType buffer
//! \param[out] rightsStatus        Indicated if the content have no rights, valid rights or unlimited rights.
//! \param[out] drmContentType      On exit will indicate whether content is drmFile, drmStream or neither. 
//! \note If fileMimeType or contentMimeType are too small to contain
//!     the entire mime type, they will contain a truncated version of the mime type.
//!     buffers with size DX_MAX_MIME_TYPE_SIZE should be enough to contain the entire mime type.
//! \note If a certain piece of information is not necessary (i.e. file MIME type, content MIME type
//!     or rights status) it is recommended to pass NULL in the appropriate parameter to avoid
//!     unnecessary computation.
//! \return
//! - DX_SUCCESS - Operation succeeded. If the file is not a DRM file the value of 
//!     drmScheme on exit will be DX_SCHEME_UNKNOWN.
//! - DX_ERROR_FILE_ACCESS_ERROR - The file does not exist or the file could not be opened or read.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
//! - DX_ERROR_BAD_ARGUMENTS - drmScheme parameter passed is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_IsDrmContent(const DxChar* fileName, EDxDrmScheme* drmScheme,
                                   DxChar* fileMimeType, DxUint32 fileMimeTypeSize, 
                                   DxChar* contentMimeType, DxUint32 contentMimeTypeSize,
                                   EDxRightsStatus* rightsStatus, EDxDrmContentType* drmContentType);

//! Identifies the DRM Scheme of the content that is in the contentData buffer.
//! \param[in]  contentData         Buffer that holds content data (which is read from the beginning of a content file)
//! \param[in]  dataSize            Size in bytes of the contentData buffer.
//! \param[out] drmScheme           On exit, holds the value of the identified DRM Scheme.
//! \param[out] fileMimeType        On entry, empty buffer. On exit, holds the file MIME type.
//!                                 The file MIME type is the DRM container mime type.
//!                                 (i.e. application/vnd.oma.drm.content for OMA V1 content,
//!                                 application/vnd.oma.drm.dcf for OMA V2 DCF)
//!                                 This value may be NULL if file mime type is not required.
//! \param[in]  fileMimeTypeSize    The size in bytes of fileMimeType buffer.
//! \param[out] contentMimeType     On entry, empty buffer. On exit, holds the content mime type. 
//!                                 The content mime type is the protected content mime type
//!                                 (i.e. image/jpeg, audio/mp3, etc...). For OMA V2 multipart
//!                                 DCF this value will be the MIME type of the first content in
//!                                 the DCF.
//!                                 This value may be NULL if content mime type is not required.
//! \param[in]  contentMimeTypeSize The size in bytes of contentMimeType buffer
//! \param[out] drmContentType      On exit will indicate whether content is drmFile, drmStream or neither. 
//! \return
//! - DX_SUCCESS - Operation succeeded. If the file is not a DRM file the value of 
//!     drmScheme on exit will be DX_SCHEME_UNKNOWN.
//! - DX_ERROR_BUFFER_IS_TOO_SMALL -  The data in the contentData buffer is not enough to 
//!     complete the identification. A 264 bytes should be enough to assure identification.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
//! - DX_ERROR_BAD_ARGUMENTS - drmScheme parameter passed is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_IsDrmContentByData(const void* contentData, DxUint32 dataSize, EDxDrmScheme* drmScheme,
                                   DxChar* fileMimeType, DxUint32 fileMimeTypeSize, 
                                   DxChar* contentMimeType, DxUint32 contentMimeTypeSize,
								   EDxDrmContentType* drmContentType);

//! NOT SUPPORTED in PLAYREADY
//! Returns the rights status of the given DRM file.
//! \param[in]  fileName	    The name of the DRM file.
//! \return
//! - DX_RIGHTS_STATUS_NOT_PROTECTED - If not a DRM content.
//! - DX_RIGHTS_STATUS_NOT_VALID - If the content has no valid rights.
//! - DX_RIGHTS_STATUS_VALID - If the content has valid rights.
//! - DX_RIGHTS_STATUS_FORWARD_LOCK - If the content has valid rights and is forward locked.
DX_FUNC DRM_C_API EDxRightsStatus DxDrmClient_GetRightsStatus(const DxChar* fileName);

//! Opens a DRM content file that is specified by fileName for usage (and read its structure headers).
//! \param[out] newDrmFile  On entry, SHOULD be NULL. 
//!                         On exit, points to a newly allocated DRM file object. 
//!                         This object MUST be terminated at the end of use by calling DxDrmFile_Close().
//! \param[in]  fileName    The name of the content file. MUST NOT be NULL.
//! \param[in]  openMode	Specifies if the file should be opened for read, write or both.
//!                         If the file is opened only for read some operations (such as:
//!                         DxDrmFile_PrepareForSuperDistribution(), DxDrmFile_SetTextAttribute())
//!                         will fail.
//! \return
//! - DX_SUCCESS - Operation succeeded. New DRM file object was created.
//! - DX_ERROR_CONTENT_NOT_RECOGNIZED - The specified file is not protected by 
//!     one of the supported DRM schemes.
//! - DX_ERROR_FILE_ACCESS_ERROR - The file does not exist or file could not be opened or read.
//! - DX_ERROR_FILE_IS_BEING_DOWNLOADED -(NOT for playReady) The file is being downloaded and there is not enough
//!     data to begin using it. You may try to open the file a again after a few seconds. 
//! - DX_NOT_AVAILABLE - only for PlayReady - there was not enough data in the file as request. 
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
//! - DX_ERROR_BAD_ARGUMENTS - newDrmFile parameter passed is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_OpenDrmFile(HDxDrmFile* newDrmFile, const DxChar* fileName, EDxFileOpenMode openMode);



//! Opens a DRM content file that is specified by osFileHandle for usage.
//! \param[out] newDrmFile  On entry, SHOULD be NULL. 
//!                         On exit, points to a newly allocated DRM file object. 
//!                         This object MUST be deleted at the end of use (using the delete operator).
//! \param[in] osFileHandle Opened file handle for the file. This file handle will
//!                         be closed when the drmFile object will be deleted. MUST NOT be NULL.
//!                         For Win32/WinCE this value's type should be FILE*.
//!                         For Symbian this value's type should be RFile.
//!                         For Linux this value's type should be a file descriptor.
//! \param[in] fileName     The name of the content file. MAY be NULL.
//! \return
//! - DX_SUCCESS - Operation succeeded. New DRM file object was created.
//! - DX_ERROR_CONTENT_NOT_RECOGNIZED - The specified file is not protected by 
//!     one of the supported DRM schemes.
//! - DX_ERROR_FILE_ACCESS_ERROR - The file could not be read.
//! - DX_ERROR_FILE_IS_BEING_DOWNLOADED - The file is being downloaded and there is not enough
//!     data to begin using it. You may try to open the file a again after a few seconds.

DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_OpenDrmFileByHandle(HDxDrmFile* newDrmFile, DxOSFileHandle osFileHandle, const DxChar* fileName);

//! Opens a DRM content stream that is specified by fileName for usage.
//! \param[out] newDrmStream  On entry, SHOULD be NULL. 
//!                           On exit, points to a newly allocated DRM stream object. 
//!                           This object MUST be terminated at the end of use by calling DxDrmStream_Close().
//! \param[in]  fileName      The name of the content file. MUST NOT be NULL.
//! \param[in]  openMode	  Specifies if the file should be opened for read, write or both.
//!                           If the file is opened only for read some operations (such as:
//!                           DxDrmStream_PrepareForSuperDistribution(), DxDrmStream_SetTextAttribute())
//!                           will fail.
//! \return
//! - DX_SUCCESS - Operation succeeded. New DRM file object was created.
//! - DX_ERROR_CONTENT_NOT_RECOGNIZED - The specified file is not protected by 
//!     one of the supported DRM schemes.
//! - DX_ERROR_FILE_ACCESS_ERROR - The file does not exist or file could not be opened or read.
//! - DX_ERROR_FILE_IS_BEING_DOWNLOADED - The file is being downloaded and there is not enough
//!     data to begin using it. You may try to open the file a again after a few seconds.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
//! - DX_ERROR_BAD_ARGUMENTS - newDrmFile parameter passed is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_OpenDrmStream(HDxDrmStream* newDrmStream, const DxChar* fileName, EDxFileOpenMode openMode);

//! Opens a DRM content stream that is specified by data in a memory buffer.
//! \param[out] newDrmStream  On entry, SHOULD be NULL. 
//!                           On exit, points to a newly allocated DRM stream object. 
//!                           This object MUST be terminated at the end of use by calling DxDrmStream_Close().
//! \param[in] data			Data buffer in memory containing the data needed to open a stream.
//! \param[in] dataSize		size of the given data buffer.
//! \return
//! - DX_SUCCESS - Operation succeeded. New DRM file object was created.
//! - DX_ERROR_CONTENT_NOT_RECOGNIZED - The specified file is not protected by 
//!     one of the supported DRM schemes.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
//! - DX_ERROR_BAD_ARGUMENTS - newDrmFile parameter passed is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_OpenDrmStreamFromData(HDxDrmStream* newDrmStream, const void* data, DxUint32 dataSize);

//! Opens a DRM content that is specified by fileName. Content may be a file or a stream.
//! The DxDrmContent API offers limited functionality and should be used only when content type is unknown.
//! Otherwise, DxDrmClient_OpenDrmFile or DxDrmClient_OpenDrmStream should be used.
//! \param[out] newDrmContent  On entry, SHOULD be NULL. 
//!                         On exit, points to a newly allocated DRM content object. 
//!                         This object MUST be terminated at the end of use by calling DxDrmContent_Close().
//! \param[in]  fileName    The name of the content file. MUST NOT be NULL.
//! \param[in]  openMode	Specifies if the file should be opened for read, write or both.
//!                         If the file is opened only for read some operations (such as:
//!                         DxDrmFile_PrepareForSuperDistribution(), DxDrmFile_SetTextAttribute())
//!                         will fail.
//! \return
//! - DX_SUCCESS - Operation succeeded. New DRM file object was created.
//! - DX_ERROR_CONTENT_NOT_RECOGNIZED - The specified file is not protected by 
//!     one of the supported DRM schemes.
//! - DX_ERROR_FILE_ACCESS_ERROR - The file does not exist or file could not be opened or read.
//! - DX_ERROR_FILE_IS_BEING_DOWNLOADED - The file is being downloaded and there is not enough
//!     data to begin using it. You may try to open the file a again after a few seconds.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
//! - DX_ERROR_BAD_ARGUMENTS - newDrmFile parameter passed is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_OpenDrmContent(HDxDrmContent* newDrmContent, const DxChar* fileName, EDxFileOpenMode openMode);

// Shai: We should add ContentIndex string parameter
//! Retrieves the value of the specified text attribute in the given file.
//! The value of the text attribute is set in the preallocated buffer - attrValue.
//! If the attribute could not be found or invalid contentIndex was specified, NULL will be returned.
//! \param[in]  fileName		Specifies DRM content fileName from which to retrieve the text attribute. 
//! \param[in]  textAttr		Specifies the attribute to retrieve.
//! \param[out] attrValue		Preallocated buffer. On exit will hold the value of the text attribute.
//! \param[out] attrValueSize   Size of the preallocated buffer.
//! \return
//! - DX_SUCCESS - if DRM Scheme could be identified.
//! - DX_ERROR_CONTENT_NOT_RECOGNIZED - if DRM Scheme could not be identified.
//! - DX_ERROR_FILE_ACCESS_ERROR - If file not exist or file could not be opened or read.
//! - DX_ERROR_FILE_IS_BEING_DOWNLOADED - If the file is being downloaded and there is not enough data to begin using it.
//! - DX_ERROR_BAD_ARGUMENTS - If the fileName or attrValue parameters are Null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_GetTextAttribute(const DxChar* fileName, EDxTextAttribute textAttr, DxChar* attrValue, DxUint32 attrValueSize);


//! Retrieves the value of the specified text attribute.
//! The value of the text attribute is set in the preallocated buffer - attrValue.
//! If the attribute could not be found or invalid contentIndex was specified, NULL will be returned.
//! \param[in]  textAttr		Specifies the attribute to retrieve.
//! \param[out] attrValue		attribute value buffer. On exit will hold the value of the text attribute.
//! \param[in] attrValueSize   Size of the buffer.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_GetGlobalTextAttribute(EDxTextAttribute textAttr, DxChar* attrValue, DxUint32 attrValueSize);

// Shai: We should add ContentIndex string parameter
//! Retrieve set of flags that are available to the specified content.
//! \param[in]	fileName		Specifies DRM content fileName from which to retrieve the set of flags. 
//! \param[in]  flagsToCheck    OR-ed combination of flags to be retrieved. 
//!                             The OR-ed values should be taken from the EDxContentFlag enumeration.
//! \param[out] activeFlags     On exit, Only the flags that were specified in flagsToCheck and are active will be on.
//! \param[in]  permissions     OR-ed combination of permissions taken from the EDxPermission enumeration.
//!                             The flags that will be retrieved will be relevant for the specified permissions.
//! \return
//! - DX_SUCCESS - On Success
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - If the flags that were asked cannot be computed correctly because the secure clock is not set.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_GetFlags(const DxChar* fileName, DxUint32 flagsToCheck, DxUint32* activeFlags, DxUint32 permissions);

//! Prepare the content for super distribution or backup.
//! For OMA V2 this operation usually involves embedding of rights in the content,
//! and also updating the Transaction ID that is in the (P)DCF (if exists). The file itself needs to be modified.
//! \param[in] fileName			The filename to be super-distributed.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
//! - DX_ERROR_FILE_IS_NOT_FORWARDABLE - If content cannot be forwarded.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_PrepareForSuperDistribution(const DxChar* fileName);

//! NOT SUPPORTED in PLAYREADY
//! Checks if the specified file has suitable rights for the specified intent.
//! If rights are not available the function may try to obtain new rights
//! based on the autoMode parameter. To learn more about possible values for  
//! autoMode variable please see DxDrmContent_SetIntent().
//! \return
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NO_RIGHTS - There are no rights for the active content.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - The rights for the active content are time based and the secure clock is not set.
//! - DX_ERROR_BAD_ARGUMENTS - If one of the argument is invalid.
//! - DX_ERROR_OPERATION_CANCELED_BY_USER - user canceled the rights acquisition operation.
//! - DX_ERROR_OPERATION_FAILED - rights acquisition operation failed.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_CheckRights(const DxChar* fileName, EDxIntent intentVal, EDxAutoMode autoMode);

//! Creates an import stream that will be used to introduce new DRM objects (content, rights, etc...)
//! to the DRM system. Import streams are useful for introducing DRM object that  
//! reach the device through internet download, WAP push, MMS, Bluetooth, PC synchronization, etc...
//! \param[out] newImportStream On entry, SHOULD be NULL. 
//!                             On exit, points to a newly allocated Import Stream object. 
//!                             This object MUST be terminated at the end of use by calling DxImportStream_Destroy().
//! \param[in]  mimeType        MIME type of the DRM object.
//! \return
//! - DX_SUCCESS - Operation succeeded. New import stream was created.
//! - DX_ERROR_MIME_TYPE_NOT_RECOGNIZED - If MIME type is not recognized as DRM object MIME type.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
//! - DX_ERROR_BAD_ARGUMENTS - newImportStream parameter passed is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_CreateImportStream(HDxImportStream* newImportStream, const DxChar* mimeType);

//! An automation of the import process, useful for handling ROAP sessions or introducing DRM objects to the DRM system.
//! This function follows this logic:
//!     - Sends a request to the given URL.
//!     - Creates an import stream according to the content-type returned.
//!     - Handles the entire import process.
//! For more flexibility it is better to do these operations manually instead of calling ImportUrl().
//! \param[in]  url           The url to be imported.
//! \param[in]  destFileName  Name of the file to which the DRM object should be stored
//!                           (in case it is a content object).
//!                           If the file already exists it will be overwritten.
//!                           destFileName MAY be NULL. If destFileName is NULL and 
//!                           destination file name is required it will be retrieved
//!                           using the normal import stream process (i.e. the user
//!                           will be asked to enter a file name or the file name may
//!                           be deduced from file information).
//!
//! \return
//! - DX_SUCCESS - Operation succeeded. New import stream was created.
//! - DX_ERROR_MIME_TYPE_NOT_RECOGNIZED - If MIME type is not recognized as DRM object MIME type.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
//! - DX_ERROR_BAD_ARGUMENTS - url parameter passed is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_ImportURL(const DxChar* url, const DxChar* destFileName);

//! Introduce new DRM object (content, rights, triggers) from a buffer to the DRM system.
//! Note: It requires you to know the mimeType of the object being imported.
//! \param[in]  data            Buffer that holds the data to import.
//! \param[in]  dataSize        Size in bytes of the data buffer.
//! \param[in]  mimeType        MIME type of the DRM object.
//!                             (i.e. The file MIME type and not the content MIME type)
//! \param[in]  destFileName    Name of the file to which the DRM object should be stored
//!                             (in case it is a content object).
//!                             If the file already exists it will be overwritten.
//!                             destFileName MAY be NULL. If destFileName is NULL and 
//!                             destination file name is required it will be retrieved
//!                             using the normal import stream process (i.e. the user
//!                             will be asked to enter a file name or the file name may
//!                             be deduced from file information).
//! \return
//! - DX_SUCCESS - Operation succeeded. New import stream was created.
//! - DX_ERROR_MIME_TYPE_NOT_RECOGNIZED - If MIME type is not recognized as DRM object MIME type.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
//! - DX_ERROR_BAD_ARGUMENTS - data or mimeType parameters passed are null.
//! - DX_ERROR_DEST_FILE_NAME_REQUIRED - Destination file name is required, it was not supplied and could
//!     not be retrieved by any means.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_ImportDrmBuffer(const void* data, DxUint32 dataSize, const DxChar* mimeType, const DxChar* destFileName, const DxChar* httpHeaders);

//! Introduce new DRM object (content, rights, triggers) that is stored in
//! sourceFileName to the DRM system.
//! Note: It requires you to know the mimeType of the object being imported.
//! \param[in]  mimeType        MIME type of the DRM object.
//!                             (i.e. The file MIME type and not the content MIME type)
//! \param[in]  sourceFileName  Name of the file that contains the DRM object.
//! \param[in]  destFileName    Name of the file to which the DRM object should be stored
//!                             (in case it is a content object).
//!                             If the file already exists it will be overwritten.
//!                             destFileName MAY be NULL. If destFileName is NULL and 
//!                             destination file name is required it will be retrieved
//!                             using the normal import stream process (i.e. the user
//!                             will be asked to enter a file name or the file name may
//!                             be deduced from file information).
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_MIME_TYPE_NOT_RECOGNIZED - The MIME type is not recognized as DRM object MIME type.
//! - DX_ERROR_INVALID_FORMAT - THe DRM object's format does not match the provided MIME type.
//! - DX_ERROR_FILE_ACCESS_ERROR - sourceFileName does not exist or could not be opened or read or 
//!     destFileName (if provided) cannot be opened for write.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_DEST_FILE_NAME_REQUIRED - Destination file name is required, it was not supplied and could
//!     not be retrieved by any means.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_ImportDrmMessage(const DxChar* sourceFileName, const DxChar* mimeType, const DxChar* destFileName, const DxChar* httpHeaders);


//! Introduce new DRM object (content, rights, triggers) from a buffer to the DRM system.
//! Note: It requires you to know the mimeType of the object being imported.
//! \param[in]  data            Buffer that holds the data to import.
//! \param[in]  dataSize        Size in bytes of the data buffer.
//! \param[in]  mimeType        MIME type of the DRM object.
//!                             (i.e. The file MIME type and not the content MIME type)
//! \param[in]  destFileName    Name of the file to which the DRM object should be stored
//!                             (in case it is a content object).
//!                             If the file already exists it will be overwritten.
//!                             destFileName MAY be NULL. If destFileName is NULL and 
//!                             destination file name is required it will be retrieved
//!                             using the normal import stream process (i.e. the user
//!                             will be asked to enter a file name or the file name may
//!                             be deduced from file information).
//! \param[out] userMessageToReturn a message return when the license acquisition failed.
//!									if the result != DX_SUCCESS and the userMessage != DX_NULL
//!									it is important to call  DxDrmClient_DestroyMessage() after 
//!									getting the desired parameters to prevent memory leaks.
//! \return
//! - DX_SUCCESS - Operation succeeded. New import stream was created.
//! - DX_ERROR_MIME_TYPE_NOT_RECOGNIZED - If MIME type is not recognized as DRM object MIME type.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
//! - DX_ERROR_BAD_ARGUMENTS - data or mimeType parameters passed are null.
//! - DX_ERROR_DEST_FILE_NAME_REQUIRED - Destination file name is required, it was not supplied and could
//!     not be retrieved by any means.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_ImportDrmBufferEx(const void* data, DxUint32 dataSize, const DxChar* mimeType, const DxChar* destFileName, const DxChar* httpHeaders, HDxUserMessage* userMessageToReturn);

//! Introduce new DRM object (content, rights, triggers) that is stored in
//! sourceFileName to the DRM system.
//! Note: It requires you to know the mimeType of the object being imported.
//! \param[in]  mimeType			MIME type of the DRM object.
//!									(i.e. The file MIME type and not the content MIME type)
//! \param[in]  sourceFileName		Name of the file that contains the DRM object.
//! \param[in]  destFileName		Name of the file to which the DRM object should be stored
//!									(in case it is a content object).
//!									If the file already exists it will be overwritten.
//!									destFileName MAY be NULL. If destFileName is NULL and 
//!									destination file name is required it will be retrieved
//!									using the normal import stream process (i.e. the user
//!									will be asked to enter a file name or the file name may
//!									be deduced from file information).
//! \param[out] userMessageToReturn a message return when the license acquisition failed.
//!									if the result != DX_SUCCESS and the userMessage != DX_NULL
//!									it is important to call  DxDrmClient_DestroyMessage() after 
//!									getting the desired parameters to prevent memory leaks.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_MIME_TYPE_NOT_RECOGNIZED - The MIME type is not recognized as DRM object MIME type.
//! - DX_ERROR_INVALID_FORMAT - THe DRM object's format does not match the provided MIME type.
//! - DX_ERROR_FILE_ACCESS_ERROR - sourceFileName does not exist or could not be opened or read or 
//!     destFileName (if provided) cannot be opened for write.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_DEST_FILE_NAME_REQUIRED - Destination file name is required, it was not supplied and could
//!     not be retrieved by any means.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_ImportDrmMessageEx(const DxChar* sourceFileName, const DxChar* mimeType, const DxChar* destFileName, const DxChar* httpHeaders, HDxUserMessage* userMessageToReturn);

//! Deletes the assets specified by assetBitMask from the secure storage.
//! \param[in]  assetBitMask    an OR-ed combination of assets to delete. 
//!                             Asset values should be taken from EDxAsset enumeration.
//! \return
//! - DX_SUCCESS - Operation completed successfully
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - Secure storage is found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_DeleteAssets(DxUint32 assetBitMask);

//! Deletes expired Rights objects & Licenses from the secure storage.
//! Expired Rights objects are consumable objects (i.e. count, timed count, interval, accumulated)
//! that were fully consumed or time based objects whose validity period is over.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - Secure storage is found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_DeleteObsoleteRights();

//! Stores an asset in the secure storage as part of the provisioning process. The format of the
//! data buffer should match the asset type described in assetType.
//! The following table list the asset types that can be specified and the matching data format.
//! - DX_STORE_OMA_V2_CERTIFICATE             DER encoded Certificate
//! - DX_STORE_OMA_V2_PROVISION_PACKAGE       Discretix generated provisioning package.
//! - DX_STORE_WM_CERTIFICATE_TEMPLATE        UTF16 encoded XML with MS Device Certificate Template
//! - DX_STORE_WM_CERTIFICATE,                UTF16 encoded XML with MS Device Certificate
//! - DX_STORE_WM_GROUP_PRIVATE_KEY           20 binary bytes that hold the WMDRM Group Private Key.
//! - DX_STORE_WM_FALLBACK_KEYS               20 binary bytes that hold the WMDRM fall-back keys.
//! \param[in] assetType    The type of asset (see table)
//! \param[in] assetTag     A tag that is associated with the stored asset. 
//!                         The tag is used to group several assets that are linked together
//!                         (i.e. certificates in a certificate chain, device certificate and
//!                         matching private key, etc...) and to identify the assets that
//!                         are provisioned on the device (i.e. "CMLA", "Non-CMLA", "Vodafone", etc...)
//! \param[in] data         Buffer that holds asset data. Data format is dependent on assetType (see table).
//! \param[in] dataSize     Size in bytes of the data buffer.
//! \return
//! - DX_SUCCESS - Operation completed successfully
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - Secure storage is found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_INVALID_FORMAT - The format of the asset to be stored does not match the asset type.
//! - DX_ERROR_TAG_ALREADY_EXIST -  The asset cannot be stored because similar asset is already provisioned
//!     on the device. Usually a call to ProvisioningDelete() with the specified tag will remove the asset
//!     from the secure storage.
//! - DX_ERROR_ASSET_ALREADY_EXIST - A similar asset (i.e. with the same assetType) already exists in the
//!     secure storage (even if it was stored with different tag) and prevent the storage of the current asset.
//!     A call to DeleteAssets() with the appropriate value will remove the asset from the secure storage. 
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_ProvisioningStore(EDxStoreAsset assetType, 
                                           const DxChar* assetTag, const void* data, DxUint32 dataSize);

//DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_ProvisionPackage(const void* data, DxUint32 dataSize);

//! Stores OMA V2 device certificate and a matching device private key in the secure storage.
//! \param[in] assetTag                 A tag that is associated with the stored asset. 
//!                                     The tag is used to group several assets that are linked together
//!                                     (i.e. certificates in a certificate chain, device certificate and
//!                                     matching private key, etc...) and to identify the assets that
//!                                     are provisioned on the device (i.e. "CMLA", "Non-CMLA", "Vodafone", etc...)
//! \param[in] deviceCert               DER encoded certificate of the device.
//! \param[in] deviceCertSize           Size in bytes of the deviceCert buffer.
//! \param[in] privateKeyModulus        The modulus component of the device private key.
//! \param[in] privateKeyModulusSize    Size in bytes of the privateKeyModulus buffer.
//! \param[in] privateKeyExponent       The exponent component of the device private key.
//! \param[in] privateKeyExponentSize   Size in bytes of the privateKeyExponent buffer.
//! \return
//! - DX_SUCCESS - Operation completed successfully
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - Secure storage is found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_TAG_ALREADY_EXIST -  The asset cannot be stored because similar asset is already provisioned
//!     on the device. Usually a call to ProvisioningDelete() with the specified tag will remove the asset
//!     from the secure storage.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
//! - DX_ERROR_BAD_ARGUMENTS - assetTag is null or points to null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_ProvisioningDeviceCredentials(const DxChar* assetTag,
                                                       const void* deviceCert, DxUint32 deviceCertSize,
                                                       const void* privateKeyModulus, DxUint32 privateKeyModulusSize, 
                                                       const void* privateKeyExponent, DxUint32 privateKeyExponentSize);

//! Stores OMA V2 device certificate and a matching device private key in the secure storage.
//! \param[in] assetTag                 A tag that is associated with the stored asset. 
//!                                     The tag is used to group several assets that are linked together
//!                                     (i.e. certificates in a certificate chain, device certificate and
//!                                     matching private key, etc...) and to identify the assets that
//!                                     are provisioned on the device (i.e. "CMLA", "Non-CMLA", "Vodafone", etc...)
//! \param[in] deviceCert               DER encoded certificate of the device.
//! \param[in] deviceCertSize           Size in bytes of the deviceCert buffer.
//! \param[in] privateKeyP              The prime number P that constructs the modulus of the device private key.
//! \param[in] privateKeyPSize          Size in bytes of the privateKeyP buffer.
//! \param[in] privateKeyQ              The prime number Q that constructs the modulus of the device private key.
//! \param[in] privateKeyQSize          Size in bytes of the privateKeyQ buffer.
//! \param[in] privateKeyExponent       The exponent component of the device private key.
//! \param[in] privateKeyExponentSize   Size in bytes of the privateKeyExponent buffer.
//! \return
//! - DX_SUCCESS - Operation completed successfully
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - Secure storage is found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_TAG_ALREADY_EXIST -  The asset cannot be stored because similar asset is already provisioned
//!     on the device. Usually a call to ProvisioningDelete() with the specified tag will remove the asset
//!     from the secure storage.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
//! - DX_ERROR_BAD_ARGUMENTS - assetTag is null or points to null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_ProvisioningDeviceCredentials2(const DxChar* assetTag,
                                                                         const void* deviceCert, DxUint32 deviceCertSize,
                                                                         const void* privateKeyP, DxUint32 privateKeyPSize, 
                                                                         const void* privateKeyQ, DxUint32 privateKeyQSize, 
                                                                         const void* privateKeyExponent, DxUint32 privateKeyExponentSize);

//! Verifies that the provisioning process of the assets that were stored with the specified tag 
//! was completed successfully and these assets can be successfully used by the DRM scheme specified
//! by drmScheme.
//! \param[in] drmScheme    DRM Scheme related to the asset tag.
//! \param[in] assetTag     A tag that is associated with the stored asset. The tag is used to group several assets
//!							that are linked together (i.e. certificates in a certificate chain, device certificate
//!                         and matching private key, etc...) and to identify the assets that are provisioned
//!                         on the device (i.e. "CMLA", "Non-CMLA", "Vodafone", etc...)
//! The following table lists the verification that is being done for every scheme.
//! - DX_SCHEME_OMA_V2      The function will verify that a device certificate exists and a matching
//!                         device private key exist. It will also verify that a certificate chain
//!                         that leads to a self signed certificate was stored on the device.
//! - DX_SCHEME_WMDRM       The function will verify that a device certificate exists. 
//!                         If device certificate does not exist the function will try to create
//!                         it from the certificate template and group private key (if they exist).
//!                         The function will verify that the group private key can be used to decrypt
//!                         the device private key which appears in the device certificate.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - Secure storage is found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_VERIFICATION_FAILURE - The provisioning was not completed successfully (i.e. assets are missing, 
//!     asset format is not valid, assets don't match each other)
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
//! - DX_ERROR_BAD_ARGUMENTS - assetTag is null or points to null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_ProvisioningVerify(EDxDrmScheme drmScheme, const DxChar* assetTag);

//! Deletes from secure storage all the assets of the specified DRM scheme that were stored 
//! with the specified tag. If tag was not found the function will still succeed.
//! \return
//! - DX_SUCCESS - Operation completed successfully
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - Secure storage is found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
//! - DX_ERROR_BAD_ARGUMENTS - assetTag is null or points to null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_ProvisioningDelete(EDxDrmScheme drmScheme, const DxChar* assetTag);

//! Sets the value of the configuration flag specified by settingsItem to the value specified in settingsItemValue.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - Secure storage is found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_SetDrmSettingsValue(EDxDrmSettingsItem settingsItem, DxUint32 settingsItemValue);

//! Retrieves the value of the configuration flag specified by settingsItem. 
//! The retrieved value is stored in settingsItemValue on exit.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - Secure storage is found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_GetDrmSettingsValue(EDxDrmSettingsItem settingsItem, DxUint32* settingsItemValue);

//! Deletes a DRM content from the file-system, along with any obsolete rights which still might be stored in the
//! secure storage DB.
//! \param[in]  fileName      name of the content file
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - Secure storage is found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
//! - DX_ERROR_FILE_ACCESS_ERROR - The file does not exist or the file could not be opened or read.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_DeleteFile(const DxChar* fileName);

//! Notifies the DRM client that a file is about to be deleted - 
//! The client will delete the file name from the content cache 
//! \param[in]  fileName            file name to delete from content cache
//! \note The function should be called before the file was actually deleted	
//!		fileName MUST NOT be null
//! \return
//! - DX_DRM_SUCCESS - if operation succeeded. 
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
//! - DX_ERROR_BAD_ARGUMENTS - fileName parameter passed is DX_NULL
//! - DX_ERROR_FILE_ACCESS_ERROR - If file (fileName) not exist or file could not be opened or read.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_OnFileDelete(const DxChar* fileName);

//! Notifies the DRM client that a new file was created/moved - 
//! The client will add the fileName to the content cache 
//! \param[in]  fileName            file name to add to the content cache
//! \param[in]  oldFileName         when the new content was created by a move operation
//!					then oldFileName should indicate the content old name - can be null
//! \note The function should be called after the file was created/moved	
//! \return
//! - DX_DRM_SUCCESS - if operation succeeded.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
//! - DX_ERROR_BAD_ARGUMENTS - fileName parameter passed is DX_NULL
//! - DX_ERROR_FILE_ACCESS_ERROR - If file (fileName) not exist or file could not be opened or read.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_OnNewFile(const DxChar* fileName, const DxChar* oldFileName);

//! Unset the secure clock of all the supported DRM schemes. This function should be
//! used only if the DRM has no dedicated clock and the clock that the DRM clock is
//! derived clock was changed.
//! \return
//! - DX_SUCCESS - Operation completed successfully
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - Secure storage is found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_MasterClockHasChanged();

//! Sets the DRM time for a given scheme by synchronizing with a time server (allows the re-setting of the DRM time to a valid value in cases of clock rollback or if system time has changed in an extreme way).
//! \param[in]  drmScheme            the scheme for which to set the clock.
//! \return
//! - DX_SUCCESS - Operation completed successfully
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - Secure storage is found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_SynchronizeDrmTime(EDxDrmScheme drmScheme);

//! Retrieves the current DRM clock of the specified DRM scheme.
//! \param[in]  drmScheme   The DRM scheme whose clock should be retrieved.
//! \param[out] timeStruct  On entry, empty struct. On exit, the current value of the 
//!                         DRM clock of the specified DRM scheme.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - The DRM clock of the specified DRM scheme is not currently set.
//DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_GetSecureClock(const DxChar* clockId, DxTimeStruct* timeStruct);

//! NOT SUPPORTED in PLAYREADY
//! processes an MTP command buffer (WMDRM only).
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_ProcessMtpCommand(const void* inData, DxUint32 inSize, DxDrmBuffer* outData);

//! Use this function to set a UI context which might be needed by the MessageBox implementation
//! (e.g. A handle to the owner window of the message box in winCE). If UI context is set, it will be passed to all DRM Objects
//! created by this module.
//! Note: Some platforms do not require this parameter to be set.
DX_FUNC DRM_C_API void DxDrmClient_SetUiContext(void* uiContext);

//! Retrieves the UI Context previously set.
DX_FUNC DRM_C_API void* DxDrmClient_GetUiContext();

//! A function to deallocate the MtpBufferStruct
DRM_C_API void DxDestroyMtpBuffer(DxDrmBuffer * buffer);


//! Generate JoinDomain challenge.
//! \param[in]	service_Id		Buffer that contains the service ID in base64.
//! \param[in]	serviceLen		service_Id buffer length
//! \param[in]	accound_Id		Buffer that contains the account ID in base64.
//! \param[in]	accountLen		account_Id buffer length
//! \param[in]	domainRevision	DxUint32*
//! \param[in]	friendlyName	NOT_SUPPORTED in this stage , it may be DX_NULL.
//! \param[in]	friendlyNameLen friendlyName buffer length
//! \param[in]	customData		Buffer that contains the custom data to be passed to the domain.
//! \param[in]	customDataLen	customData buffer length
//! \param[out]	challenge		Buffer that will contain the generated challenge and the challenge size.
//! \param[out]	size			the challenge size
//! \return
//! - DX_SUCCESS - Operation completed successfully.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_GetJoinDomainChallenge( void* service_Id , DxUint32 serviceLen, void* account_Id, DxUint32 accountLen, DxUint32* domainRevision, void* friendlyName, DxUint32 friendlyNameLen, void* customData, DxUint32 customDataLen, void* challenge, DxUint32* size);

//! Generate LeaveDomain challenge.
//! \param[in]	service_Id		Buffer that contains the service ID in base64.
//! \param[in]	serviceLen		service_Id buffer length
//! \param[in]	accound_Id		Buffer that contains the account ID in base64.
//! \param[in]	accountLen		account_Id buffer length
//! \param[in]	domainRevision	DxUint32*
//! \param[in]	customData		Buffer that contains the custom data to be passed to the domain.
//! \param[in]	customDataLen	customData buffer length
//! \param[out]	challenge		Buffer that will contain the generated challenge and the challenge size.
//! \param[out]	size			the challenge size
//! \return
//! - DX_SUCCESS - Operation completed successfully.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_GetLeaveDomainChallenge( void* service_Id , DxUint32 serviceLen, void* account_Id, DxUint32 accountLen, DxUint32* domainRevision, void* customData, DxUint32 customDataLen, void* challenge, DxUint32* size);

//! Generate License Acquisition Acknowledgment challenge.
//! \param[in]	responseResult 		HDxResponseResult handler that contains the process response result. . PLEASE see the IPORTANT note below.
//! \param[out]	challenge			Buffer that will contain the generated challenge and the challenge size.
//! \param[out]	size				the challenge size
//! IMPORTANT!! Parameter LicenseResponse is the result of the DxPlayReady_LicenseAcq_ProcessResponse() function.
//! therefore first we need to process the license response , take the result and then 
//! we will be able to generate Acknowledgment.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_GetLicenseAcq_GenerateAck(  HDxResponseResult* responseResult, void* challenge, DxUint32* size);

//! Generate Metering Certificate challenge.
//! \param[in]	metering_Id		Buffer that contains the metering ID in base64.
//! \param[in]	meteringLen		metering_Id buffer length
//! \param[in]	customData		Buffer that contains the custom data.
//! \param[in]	customDataLen	customData buffer length
//! \param[out]	challenge		Buffer that will contain the generated challenge and the challenge size.
//! \param[out]	size			the challenge size
//! \return
//! - DX_SUCCESS - Operation completed successfully.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_GetMeterCertChallenge( void* metering_Id, DxUint32 meteringLen, void* customData,  DxUint32 customDataLen, void* challenge, DxUint32* size);

//! Generate Metering Data challenge.
//! \param[in]	metering_Id		Buffer that contains the metering ID in base64.
//! \param[in]	meteringLen		metering_Id buffer length
//! \param[in]	customData			Buffer that contains the custom data.
//! \param[in]	customDataLen		customData buffer length
//! \param[out]	meterServerUrl		Buffer that will contain the metering server URL.
//! \param[in-out]	meterServerUrlLen	meterServerUrl buffer length
//! \param[out]	challenge			Buffer that will contain the generated challenge and the challenge size.
//! \param[out]	size				the challenge size
//! \return
//! - DX_SUCCESS - Operation completed successfully.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_GetMeteringChallenge( void* metering_Id, DxUint32 meteringLen,  void* customData, DxUint32 customDataLen, void* meterServerUrl, DxUint32* meterServerUrlLen, void* challenge, DxUint32* size);

//! Process Server Response.
//! \param[in]	serverResponse		Buffer that contains the server response.
//! \param[in]	serverResponseLen	serverResponse buffer length
//! \param[in]	type				allow the function to recognize the response type (license acquisition , join domain etc) - please see EDxServerResponse definition
//! \param[out]	responseResult 		HDxResponseResult handler that contains the process response result. (needed for the acknowledgment challenge) , may be NULL except 
//!									the case that the type is DX_RESPONSE_LICENSE_ACQ.
//! \param[out]	isAckRequired		DxBool pointer that indicates if the server needs acknowledgment for the license or not.
//!									If (*isAckRequired == DX_TRUE) then there is a need to call DxDrmClient_GetLicenseAcq_GenerateAck() with the license response result
//!									to create a challenge that should been sent to the server back. isAckRequired may be NULL.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_ProcessServerResponse( const void* serverResponse, DxUint32 serverResponseLen, EDxServerResponse type, HDxResponseResult* responseResult, DxBool* isAckRequired);


//! Get a Message Parameter from a user message.
//! after you finish to get the parameters you want , and there is no need with the userMessage , it is important to call 
//! DxDrmClient_DestroyMessage() , else there will be memory leaks.
//! \param[in] userMessage	This is the user message that returned from failed SetIntentEx / acquireRightsExtended / ImportDrmMessageEx / ImportDrmBufferEx
//! \param[in] messageParam	The desired parameter to return from the user message ,see the definition of EDxSoapResult 
//!
//! \return 				The desired parameter in case it exist , DX_NULL otherwise
DX_FUNC DRM_C_API const DxChar* DxDrmClient_GetMessageParam(HDxUserMessage userMessage, EDxSoapResult messageParam);

//! Destroy the User Message
//! destrory and free the user message.
//! \param[in] userMessage	The user message return from the license acquisition process.
//! \return DX_SUCCESS
DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_DestroyMessage(HDxUserMessage* userMessage);
#ifdef __cplusplus
}
#endif

#endif

