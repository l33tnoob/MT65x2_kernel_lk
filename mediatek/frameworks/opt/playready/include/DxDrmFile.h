#ifndef DX_DRM_FILE_H
#define DX_DRM_FILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "DxRightsObject.h"


//! \file DxDrmFile.h
//! \brief DxDrmFile is used to perform operations on DRM files.
//!
//! A HDxDrmFile handle is associated with an opened DRM file. It is created by calling DxDrmClient_OpenDrmFile().
//! A DRM file may include several contents. By default the first content is the active content. The active content is the content
//! that is currently consumed. The active content may be changed using DxDrmFile_SetActiveContent(). 
//! HDxDrmFile provides the following features:
//! - Extraction of content related attributes such as:  Title, Author, Description, etc. 
//! - Query about DRM status of the content (i.e. Is the content encrypted? Do rights exists?)
//! - Decryption of file content (i.e. reading decrypted data as if the file is not encrypted)
//! - Consumption of content rights
//! - Extraction of content's right information.


//! HDxDrmFile is a handle associated with an opened DRM file. It is created by calling DxDrmClient_OpenDrmFile().
typedef void* HDxDrmFile;

DX_FUNC DRM_C_API EDxDrmScheme DxDrmFile_GetDRMScheme(HDxDrmFile drmFile);

//! NOT SUPPORTED in PLAYREADY
//! returns the number of contents in the DRM file.
//! In OMA V2 there may be multiple contents. 
//! In case of progressive download the function will return only the number of contents
//! whose download has started (i.e. the contents which are fully downloaded or partially downloaded.)
//! \param[in] drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
DX_FUNC DRM_C_API DxUint32 DxDrmFile_GetNumOfContents(HDxDrmFile drmFile);

//! Returns the value of the specified text attribute.
//! The returned pointer points to memory that exists until the next call to GetTextAttribute().
//! The caller MUST NOT free this memory.
//! If the attribute could not be found or invalid contentIndex was specified, NULL will be returned.
//! \param[in] drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in] textAttr     Specifies the attribute to retrieve. 
//! \param[in] contentIndex The index of the content whose attribute should be retrieved.
//!                         If the value is DX_ACTIVE_CONTENT the active content will be used.
DX_FUNC DRM_C_API const DxChar* DxDrmFile_GetTextAttribute(HDxDrmFile drmFile, EDxTextAttribute textAttr, DxUint32 contentIndex);

//! Returns the value of the text attribute specified by attrName.
//! The returned pointer points to memory that exists until the next call to GetTextAttribute().
//! The caller MUST NOT free this memory.
//! If the attribute could not be found or invalid contentIndex was specified, NULL will be returned.
//! \param[in] drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in] attrName     The name of the attribute to retrieve. This name is DRM scheme specific.
//! \param[in] contentIndex The index of the content whose attribute should be retrieved.
//!                         If the value is DX_ACTIVE_CONTENT the active content will be used.
DX_FUNC DRM_C_API const DxChar* DxDrmFile_GetTextAttributeByName(HDxDrmFile drmFile, const DxChar* attrName, DxUint32 contentIndex);

//! Returns the number of text attributes that exists for the specified content.
//! If invalid contentIndex was specified, 0 will be returned.
//! \param[in] drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in] contentIndex The index of the content whose number of attributes should be retrieved.
//!                         If the value is DX_ACTIVE_CONTENT the active content will be used.
DX_FUNC DRM_C_API DxUint32 DxDrmFile_GetNumOfTextAttributes(HDxDrmFile drmFile, DxUint32 contentIndex);

//! Retrieves the name & value of the attribute that is placed in the specified index in the
//! attribute list of the specified content.
//! \param[in]  drmFile			A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in]  attrIndex       The index of the attribute to retrieve in the attribute list.
//! \param[out] attrName        On entry, SHOULD be NULL. On exit, points to the attribute name NULL terminated string.
//!                             The returned pointer points to memory that exists until the next call to GetTextAttributeByIndex().
//!                             The caller MUST NOT free this memory.
//! \param[out] attrValue       On entry, SHOULD be NULL. On exit, points to the attribute value NULL terminated string.
//!                             The returned pointer points to memory that exists until the next call to GetTextAttributeByIndex().
//!                             The caller MUST NOT free this memory.
//! \param[in]  contentIndex    The index of the content whose attribute should be retrieved.
//!                             If the value is DX_ACTIVE_CONTENT the active content will be used.
//! \return
//! - DX_SUCCESS - Operation succeeded. Attribute was found.
//! - DX_ERROR_BAD_ARGUMENTS - attrIndex or contentIndex are invalid, or drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_GetTextAttributeByIndex(HDxDrmFile drmFile, DxUint32 attrIndex, const DxChar** attrName, const DxChar** attrValue, DxUint32 contentIndex);

//! NOT SUPPORTED in PLAYREADY
//! \return True if the value of the attribute specified by textAttr can be set using SetTextAttribute().
DX_FUNC DRM_C_API DxBool DxDrmFile_IsAttributeEditable(HDxDrmFile drmFile, EDxTextAttribute textAttr);

//! NOT SUPPORTED in PLAYREADY
//! Sets the value of the specified text attribute.
//! \param[in] drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in] textAttr		Specifies the attribute to set.
//! \param[in] attrValue	Specifies the attribute value (can be NULL). 
//!                         If the attribute value is NULL previous edition 
//!                         of the specified attribute is deleted.
//! \param[in] languageCode	Specifies the language of the attribute value (represented by 3 chars). 
//!                         If NULL is used "eng" will be used. 
//! \param[in] contentIndex The index of the content whose attribute should be set.
//!                         If the value is DX_ACTIVE_CONTENT the active content will be used.
//! \return
//! - DX_SUCCESS - Attribute was set successfully.
//! - DX_ERROR_NOT_SUPPORTED - Attribute cannot be set.
//! - DX_ERROR_BAD_ARGUMENTS - contentIndex is invalid, or drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_SetTextAttribute(HDxDrmFile drmFile, EDxTextAttribute textAttr, const DxChar* attrValue, const DxChar* languageCode, DxUint32 contentIndex);

//! Retrieve set of flags that are available for the specified content.
//! \param[in]  drmFile			A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in]  flagsToCheck    OR-ed combination of flags to be retrieved. 
//!                             The OR-ed values should be taken from the EDxContentFlag enumeration.
//! \param[out] activeFlags     On exit, Only the flags that were specified in flagsToCheck and are active will be on.
//! \param[in]  permissions     OR-ed combination of permissions taken from the EDxPermission enumeration.
//!                             The flags that will be retrieved will be relevant for the specified permissions.
//! \param[in]  contentIndex    The index of the content whose attribute should be retrieved.
//!                             If the value is DX_ACTIVE_CONTENT the active content will be used.
//! \note Retrieving several flags in a single call to GetFlags() is more efficient than
//!     calling GetFlags() several times (with 1 flag every time).
//! \note Retrieve only the flags that interest you to get best performance. Retrieval
//!     of unnecessary flags may require unnecessary computation time. 
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - The flags that were asked cannot be computed 
//!     correctly because the secure clock is not set.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_GetFlags(HDxDrmFile drmFile, DxUint32 flagsToCheck, DxUint32* activeFlags, DxUint32 permissions, DxUint32 contentIndex);

//! NOT SUPPORTED in PLAYREADY
//! Indicates if rights for the current content and intent are going to be expired.
//! The rights are not considered to be expired if the remaining count exceeds countThreshold or
//! if the number of seconds left until rights expire exceeds timeThreshold.
//! If countsLeft and timeLeft are not null, then on exit they will hold the values of the total counts and time left.
//! Only valid rights (and no future rights) of the current intent will be taken into account in this calculation.
//! \param[in]  drmFile				A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in]  countThreshold		The count threshold
//! \param[in]  timeThreshold		The time threshold
//! \param[out] isGoingToExpire		On exit will point to boolean value with the result
//! \param[out] countsLeft			On exit will hold the value of total counts left.
//! \param[out] timeLeft			On exit will hold the value of seconds left.
//! \return 
//! - DX_SUCCESS - On Success
//! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_IsSoonToBeExpired(HDxDrmFile drmFile, DxUint32 countThreshold, DxUint32 timeThreshold,
												 DxBool* isGoingToExpire, DxUint32* countsLeft, DxUint32* timeLeft);

//! NOT SUPPORTED in PLAYREADY
//! Change the active content of the file to the specified content.
//! \param[in] drmFile			A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in] contentIndex		The index of the content whose attribute should be retrieved.
//!                             If the value is DX_ACTIVE_CONTENT the active content will be used.
//! \return
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_BAD_ARGUMENTS - If contentIndex is not a valid content index, or drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_SetActiveContent(HDxDrmFile drmFile, DxUint32 contentIndex);

//! NOT SUPPORTED in PLAYREADY
//! Change the active content of the file to the specified CID.
//! \param[in] drmFile			A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in] contentID		The CID of the content whose attribute should be retrieved.
//!          
//! \return
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_BAD_ARGUMENTS - If contentIndex is not a valid content index, or drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_SetActiveContentById(HDxDrmFile drmFile, const DxChar* contentID);

//! NOT SUPPORTED in PLAYREADY
//! Returns the currently active content index (zero based)
//! \param[in] drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
DX_FUNC DRM_C_API DxUint32 DxDrmFile_GetActiveContent(HDxDrmFile drmFile);

//! Sets the purpose of usage of the active content.
//! The appropriate RO will be used according to the intent that was set in this call.
//! The function will do the following if valid rights are not available:
//! - If time-based rights exists and cannot be used because the secure clock is not set, 
//!     the function will adjust the secure clock.
//! - The function will install any rights that are embedded in the content.
//! - The function will try to acquire rights silently.
//! - The function will check if preview is available and if so it will offer the user to preview the content.
//! - The function will offer the user to acquire new rights for the content and if it chooses to do so
//!     it will initiate rights acquisition.
//! \param[in]  drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in]  intentVal   The intent to be used.
//! \param[in]  autoMode	Indicates if the DRM agent is allowed to display user interface
//!							and communicate with a server during the process (if necessary).
//!							There are several modes of operation:
//!							- DX_AUTO_DISABLED - Don't perform any additional action.
//!							- DX_AUTO_NO_UI_AND_COMMUNICATION - if user interface or communication is needed the operation will fail.
//!							- DX_AUTO_NO_UI - if user interface is needed the operation will fail.
//!							- DX_AUTO_ENABLED - if user interface and communication are allowed.
//! \return
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NO_RIGHTS - There are no rights for the active content.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - The rights for the active content are time based and the secure clock is not set.
//! - DX_ERROR_INVALID_STATE - Intent cannot be changed during consumption (i.e. between DX_EVENT_START & DX_EVENT_STOP)
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
//! - DX_ERROR_OPERATION_CANCELED_BY_USER
//! - DX_ERROR_OPERATION_FAILED
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_SetIntent(HDxDrmFile drmFile, EDxIntent intentVal, EDxAutoMode autoMode);

//! Sets the purpose of usage of the active content.
//! The appropriate RO will be used according to the intent that was set in this call.
//! The function will do the following if valid rights are not available:
//! - If time-based rights exists and cannot be used because the secure clock is not set, 
//!     the function will adjust the secure clock.
//! - The function will install any rights that are embedded in the content.
//! - The function will try to acquire rights silently.
//! - The function will check if preview is available and if so it will offer the user to preview the content.
//! - The function will offer the user to acquire new rights for the content and if it chooses to do so
//!     it will initiate rights acquisition.
//! \param[in]  drmFile				A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in]  intentVal   		The intent to be used.
//! \param[in]  autoMode			Indicates if the DRM agent is allowed to display user interface
//!									and communicate with a server during the process (if necessary).
//!									There are several modes of operation:
//!									- DX_AUTO_DISABLED - Don't perform any additional action.
//!									- DX_AUTO_NO_UI_AND_COMMUNICATION - if user interface or communication is needed the operation will fail.
//!									- DX_AUTO_NO_UI - if user interface is needed the operation will fail.
//!									- DX_AUTO_ENABLED - if user interface and communication are allowed.
//! \param[out] userMessageToReturn a message return when the license acquisition failed.
//!									if the result != DX_SUCCESS and the userMessage != DX_NULL
//!									it is important to call  DxDrmClient_DestroyMessage() after 
//!									getting the desired parameters to prevent memory leaks.
//! \return
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NO_RIGHTS - There are no rights for the active content.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - The rights for the active content are time based and the secure clock is not set.
//! - DX_ERROR_INVALID_STATE - Intent cannot be changed during consumption (i.e. between DX_EVENT_START & DX_EVENT_STOP)
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
//! - DX_ERROR_OPERATION_CANCELED_BY_USER
//! - DX_ERROR_OPERATION_FAILED
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_SetIntentEx(HDxDrmFile drmFile, EDxIntent intentVal, EDxAutoMode autoMode, HDxUserMessage* userMessageToReturn);

//! Retrieves the current DRM clock relevant for the open content. Secure clock is unique per riId.
//! \param[in]  drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[out] timeStruct  On entry, empty struct. On exit, the current value of the 
//!                         DRM clock of the right issuer linked to this content.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - The DRM clock of the content is not set, or no riId linked to the content (when no rights).
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_GetSecureClock(HDxDrmFile drmFile, DxTimeStruct* timeStruct);

//! Notifies the DRM agent about a consumption event. Rights will be consumed according
//! to the events that are notified using this function. The consumption events effects only consumable rights
//! (i.e. Count, Timed Count, Interval & Accumulated)
//! The following table lists the available events and their effect on rights consumption.
//! - DX_EVENT_START    Decrements Count constraint counter by 1.
//!                     Activates a not already active interval constraint.
//!                     Start usage timer for Timed Count & Accumulated constraints.
//!                     This event is valid only if consumption is not already active.
//! - DX_EVENT_STOP     Decrements Timed Count by 1 if the time passed according to the usage timer
//!                     exceeds the Timed Count threshold and the Timed Count was not decremented yet.
//!                     Decrements an Accumulated constraints by the time passed according to the 
//!                     usage timer.
//!                     Stops the Usage Timer.
//!                     This event is always valid.
//! - DX_EVENT_PAUSE    Pauses the Usage Timer.
//!                     This event is valid only if DX_EVENT_START event occurred earlier.
//! - DX_EVENT_RESUME   Resumes the Usage Timer.
//!                     This event is valid only if DX_EVENT_PAUSE event occurred earlier.
//! - DX_EVENT_TICK     Decrements Timed Count by 1 if the time passed according to the usage timer
//!                     exceeds the Timed Count threshold and the Timed Count was not decremented yet.
//!                     Decrements an Accumulated constraints by the time passed according to the 
//!                     usage timer.
//!                     This event is always valid. If consumption is not active or paused (i.e. before
//!                     DX_EVENT_START, after DX_EVENT_STOP and between DX_EVENT_PAUSE & DX_EVENT_RESUME)
//!                     the Tick has no effect.
//! 
//! The DxDrmFile_HandleConsumptionEvent() function should be called only after the intent was set using DxDrmFile_SetIntent().
//! If the active RO expires during the consumption DX_ERROR_NO_RIGHTS will be returned.
//! If the function fails, consumption is stopped automatically (i.e. no need for STOP event).
//! \return
//! - DX_SUCCESS - Event was handled successfully.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NO_RIGHTS - There are no rights for the active content.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - The rights for the active content are time based and the secure clock is not set.
//! - DX_ERROR_INTENT_WAS_NOT_SET - The function is called before the intent was set.
//! - DX_ERROR_INVALID_STATE - If the event specified is not consistent with the current consumption state
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_HandleConsumptionEvent(HDxDrmFile drmFile, EDxConsumptionEvent eventVal);

//! Retrieves the size in byte of the active content. If possible this value will be the size
//! of the plaintext version of the content. If plaintext size can not be determined the content size
//! will be returned include the encryption padding. If file is being downloaded, the function
//! will return the size of the content downloaded so far.
//! \param[in]  drmFile			A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[out] contentSize		On success will point to the content size.
//! \return
//! - DX_SUCCESS - On Success. In this case contentSize will contain the content size of the plain-text
//!     version (i.e. after padding was removed).
//! - DX_ERROR_FILE_SIZE_INCLUDES_PADDING - If file is encrypted and padding size is unknown.
//!     In this case the contentSize will include the size of the content including the padding.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_GetContentSize(HDxDrmFile drmFile, DxUint32* contentSize);

#ifndef DX_NO_LARGE_FILE_SUPPORT
//! Retrieves the size in byte of the active content. If possible this value will be the size
//! of the plaintext version of the content. If plaintext size can not be determined the content size
//! will be returned include the encryption padding. If file is being downloaded, the function
//! will return the size of the content downloaded so far.
//! \param[in]  drmFile			A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[out] contentSize		On success will point to the content size.
//! \return
//! - DX_SUCCESS - On Success. In this case contentSize will contain the content size of the plain-text
//!     version (i.e. after padding was removed).
//! - DX_ERROR_FILE_SIZE_INCLUDES_PADDING - If file is encrypted and padding size is unknown.
//!     In this case the contentSize will include the size of the content including the padding.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_GetContentSize64(HDxDrmFile drmFile, DxUint64* contentSize);
#endif

//! Retrieves the size in byte of the active content. If possible this value will be the size
//! of the plaintext version of the content. If plaintext size can not be determined the content size
//! will be returned include the encryption padding. If file is being downloaded, the function
//! will return the full size of the content.
//! \param[in]  drmFile			A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[out] contentSize		On success will point to the content size.
//! \return
//! - DX_SUCCESS - On Success. In this case contentSize will contain the content size of the plain-text
//!     version (i.e. after padding was removed).
//! - DX_ERROR_FILE_SIZE_INCLUDES_PADDING - If file is encrypted and padding size is unknown.
//!     In this case the contentSize will include the size of the content including the padding.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_GetFullContentSize(HDxDrmFile drmFile, DxUint32* contentSize);

#ifndef DX_NO_LARGE_FILE_SUPPORT
//! Retrieves the size in byte of the active content. If possible this value will be the size
//! of the plaintext version of the content. If plaintext size can not be determined the content size
//! will be returned include the encryption padding. If file is being downloaded, the function
//! will return the full size of the content.
//! \param[in]  drmFile			A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[out] contentSize		On success will point to the content size.
//! \return
//! - DX_SUCCESS - On Success. In this case contentSize will contain the content size of the plain-text
//!     version (i.e. after padding was removed).
//! - DX_ERROR_FILE_SIZE_INCLUDES_PADDING - If file is encrypted and padding size is unknown.
//!     In this case the contentSize will include the size of the content including the padding.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_GetFullContentSize64(HDxDrmFile drmFile, DxUint64* contentSize);
#endif

//! Reads data from the active content.
//! The Read() function should be called only after the intent was set using SetIntent().
//! If the intent was set to DX_INTENT_PEEK and the active content has no rights the function will 
//! retrieve the content data without decryption.
//! If the intent was not set to DX_INTENT_PEEK and the active content has no rights the function will
//! fail (DX_ERROR_NO_RIGHTS).
//! If the active RO expires during the reading the function will fail (DX_ERROR_NO_RIGHTS).
//! \param[in]  drmFile				A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[out] data                Empty buffer that will be filled with retrieved data.
//! \param[in]  dataSize            Size in bytes of data buffer. Number of bytes to read from file.
//! \param[out] dataActuallyRead    On entry, points to an empty DxUint32 variable. MAY be NULL.
//!     On exit, if not NULL, the number of bytes that were actually read from the file.
//!     If dataActuallyRead is NULL and not all bytes could be read DX_ERROR_NOT_AVAILABLE or 
//!     DX_ERROR_FILE_IS_BEING_DOWNLOADED will be returned.
//! \return
//! - DX_SUCCESS - On Success
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NO_RIGHTS - There are no rights for the active content.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - The rights for the active content are time based and the secure clock is not set.
//! - DX_ERROR_INTENT_WAS_NOT_SET - The function was called before the intent was set.
//! - DX_ERROR_FILE_ACCESS_ERROR - read from file operation failed.
//! - DX_ERROR_NOT_AVAILABLE - for PlayReady - Not all the bytes could be read. 
//!                            otherwise, Not all the bytes could be read and also the file is not being downloaded.
//! - DX_ERROR_FILE_IS_BEING_DOWNLOADED - (NOT for PlayReady) Not all the bytes could be read and file is being downloaded.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_Read(HDxDrmFile drmFile, void* data, DxUint32 dataSize, DxUint32* dataActuallyRead);

//! Moves the file pointer by the specified offset relative to the point specified by origin.
//! The origin parameter can have one of the following values:
//! - DX_CONTENT_SEEK_SET - offset is relative to active content beginning. offset must be positive.
//! - DX_CONTENT_SEEK_CUR - offset is relative to current position in active content.
//! - DX_CONTENT_SEEK_END - offset is relative to active content end. offset must be negative.
//! \note - The function will not allow seeking to a position outside the active content boundaries.
//! \note - The seek origin is relative to the active content beginning and end and not necessarily to the
//!     container file beginning and end.
//! \param[in] drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in] offset		The value by which to move the current file position relative to the origin.
//! \param[in] origin		The position in the content from which to perform the seek.
//! \return
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_INTENT_WAS_NOT_SET - The function is called before the intent was set.
//! - DX_ERROR_NOT_AVAILABLE - The offset could not be reached and the file is not being downloaded.
//! - DX_ERROR_FILE_IS_BEING_DOWNLOADED - The offset could not be reached and the file is being downloaded.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_Seek(HDxDrmFile drmFile, DxInt32 offset, EDxContentSeek origin);

#ifndef DX_NO_LARGE_FILE_SUPPORT

//! Moves the file pointer by the specified offset relative to the point specified by origin.
//! The origin parameter can have one of the following values:
//! - DX_CONTENT_SEEK_SET - offset is relative to active content beginning. offset must be positive.
//! - DX_CONTENT_SEEK_CUR - offset is relative to current position in active content.
//! - DX_CONTENT_SEEK_END - offset is relative to active content end. offset must be negative.
//! \note - The function will not allow seeking to a position outside the active content boundaries.
//! \note - The seek origin is relative to the active content beginning and end and not necessarily to the
//!     container file beginning and end.
//! \param[in] drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in] offset		The value by which to move the current file position relative to the origin.
//! \param[in] origin		The position in the content from which to perform the seek.
//! \return
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_INTENT_WAS_NOT_SET - The function is called before the intent was set.
//! - DX_ERROR_NOT_AVAILABLE - The offset could not be reached and the file is not being downloaded.
//! - DX_ERROR_FILE_IS_BEING_DOWNLOADED - The offset could not be reached and the file is being downloaded.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_Seek64(HDxDrmFile drmFile, DxInt64 offset, EDxContentSeek origin);
#endif

//! Queries if end of active content has been reached. In case the file is being downloaded 
//! (i.e. Progressive Download) the function will indicate end of content only when the 
//! active content (not necessarily the entire container) was fully downloaded and its end
//! was reached.
//! \param[in]  drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \return
//! - DX_TRUE - If file end was reached or data cannot be read from file.
//! - DX_FALSE - If file end was not reached.
DX_FUNC DRM_C_API DxBool DxDrmFile_IsEof(HDxDrmFile drmFile);

//! Retrieve the current position in the active content. The returned value is relative to the
//! active content beginning (and not necessarily to the container file beginning).
//! \param[in]  drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \return
//!     The current position in the active content. In case of error DX_INVALID_VALUE will be returned.
DX_FUNC DRM_C_API DxUint32 DxDrmFile_GetPos(HDxDrmFile drmFile);

#ifndef DX_NO_LARGE_FILE_SUPPORT
//! Retrieve the current position in the active content. The returned value is relative to the
//! active content beginning (and not necessarily to the container file beginning).
//! \param[in]  drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \return
//!     The current position in the active content. In case of error DX_INVALID_VALUE will be returned.
DX_FUNC DRM_C_API DxUint64 DxDrmFile_GetPos64(HDxDrmFile drmFile);
#endif

//! NOT SUPPORTED in PLAYREADY
//! Retrieve a pointer to the rights object handle that is currently being used by the DRM system for
//! the active content. The calling application MUST NOT delete this object.
//! The object is valid as long as the active RO does not change.
//! \param[in]  drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \return 
//!     Handle to the active RO. If there is not active RO (i.e. the intent was not set yet) NULL is returned.
DX_FUNC DRM_C_API HDxRightsObject DxDrmFile_GetActiveRO(HDxDrmFile drmFile);

//! Prepares the content for an iteration over it's ROs. It is mandatory to call this function before calling DxDrmFile_GetRO().
//! It sets the number of available ROs in the numOfROs parameter.
//! \param[in]  drmFile		   A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in]  iterationMode  Can be one of the following values:
//!                            - DX_RO_ITERATION_DISPLAY: Prepares iteration only over the ROs which aren't expired.
//!                                Also, if similar ROs are found, their information is combined for preview purposes.
//!                            - DX_RO_ITERATION_FULL_INFO: Prepares iteration over all rights objects that are stored
//!                                in the secure storage for the active content.
//! \param[out] numOfROs	   On success will hold the number of ROs available for iteration. 
//! \return 
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile or activeRo are null, or an invalid iterationMode given.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_PrepareForROIteration(HDxDrmFile drmFile, EDxROIterationMode iterationMode, DxUint32* numOfROs);

//! Returns a handle to one of the rights objects that are stored in the secure storage for
//! the active content. The rights object is identified by its zero-based index. 
//! If index is greater or equal to the value returned by DxDrmFile_PrepareForROIteration() NULL will be returned.
//! The calling application MUST NOT delete this object.
//! The object is valid until the next call to DxDrmFile_GetRO() (i.e. you need to process the RO info
//! before moving to the next RO).
//! Note: It is necessary to call DxDrmFile_PrepareForROIteration() before calling this function.
//! \param[in] drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in] roIndex		Zero-based index of the requested rights object.
DX_FUNC DRM_C_API HDxRightsObject DxDrmFile_GetRO(HDxDrmFile drmFile, DxUint32 roIndex);

//! Returns the combined rights information of the given file from all the ROs related to the content.
//! \param[out] countsLeft			The times left to play this content.
//! \param[out] intervalTimeLeft	The interval time left to play this content.
//! \param[out] accumulatedTimeLeft	The accumulated time left to play this content.
//! \param[out] timeLeft			The time left to play this content.
//! \param[out] hasUnlimitedRights	Indicates if this content has unlimited rights.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_GetRightsInformation(HDxDrmFile drmFile, DxUint32* countsLeft, DxUint32* intervalTimeLeft, DxUint32* accumulatedTimeLeft, DxUint32* timeLeft, DxBool* hasUnlimitedRights);

//! Deletes all the ROs of the active content from the Secure Storage.
//! \param[in] drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \return 
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_DeleteRights(HDxDrmFile drmFile);

//! Deletes all the expired ROs of the active content from the Secure Storage.
//! \param[in] drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \return 
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_DeleteObsoleteRights(HDxDrmFile drmFile);

//! This function acquires rights for the content specified by contentIndex 
//! using the method specified by ackMethod.
//! \param[in] drmFile			A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in] acqMethod		The method by which to acquire rights for content:
//!								- DX_RIGHT_ACQ_METHOD_NORMAL - Use the rights issuer url (DX_ATTR_RI_URL) to acquire rights.
//!								- DX_RIGHT_ACQ_METHOD_SILENT - Acquire rights silently. Only if the silent attribute
//!								  and silent url exist (DX_ATTR_SILENT_METHOD).
//!								- DX_RIGHT_ACQ_METHOD_PREVIEW - Acquire preview rights for the content. Only if the preview
//!								  attribute and url exist (DX_ATTR_PREVIEW_METHOD).					
//! \param[in] contentIndex		The index of the content whose attribute should be retrieved.
//!                             If the value is DX_ACTIVE_CONTENT the active content will be used.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_CANNOT_ACQUIRE_RIGHTS - Rights can not be acquired for this content using the specified method.
//! - DX_ERROR_OPERATION_CANCELED - The user canceled the rights acquisition process.
//! - DX_ERROR_OPERATION_FAILED - The rights acquisition failed.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_AcquireRights(HDxDrmFile drmFile, EDxRightsAcquisitionMethod acqMethod, DxUint32 contentIndex);

//! NOT SUPPORTED in PLAYREADY
//! This function acquires rights for the content specified by contentIndex from the rights issuer url (DX_ATTR_RI_URL).
//! It prompts the user when needed according to specification.
//! \param[in] drmFile			A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in] contentIndex		The index of the content whose attribute should be retrieved.
//!                             If the value is DX_ACTIVE_CONTENT the active content will be used.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_CANNOT_ACQUIRE_RIGHTS - Rights can not be acquired for this content.
//! - DX_ERROR_OPERATION_CANCELED - The user canceled the rights acquisition process.
//! - DX_ERROR_OPERATION_FAILED - The rights acquisition failed.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_AcquireRightsEx(HDxDrmFile drmFile, DxUint32 contentIndex);

//! This function acquires rights for the content specified by contentIndex 
//! using the method specified by ackMethod.
//! \param[in] drmFile			A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in] acqMethod		The method by which to acquire rights for content:
//!								- DX_RIGHT_ACQ_METHOD_NORMAL - Use the rights issuer url (DX_ATTR_RI_URL) to acquire rights.
//!								- DX_RIGHT_ACQ_METHOD_SILENT - Acquire rights silently. Only if the silent attribute
//!								  and silent url exist (DX_ATTR_SILENT_METHOD).
//!								- DX_RIGHT_ACQ_METHOD_PREVIEW - Acquire preview rights for the content. Only if the preview
//!								  attribute and url exist (DX_ATTR_PREVIEW_METHOD).					
//! \param[in] contentIndex		The index of the content whose attribute should be retrieved.
//!                             If the value is DX_ACTIVE_CONTENT the active content will be used.
//! \param[out] userMessageToReturn a message return when the license acquisition failed.
//!									if the result != DX_SUCCESS and the userMessage != DX_NULL
//!									it is important to call  DxDrmClient_DestroyMessage() after 
//!									getting the desired parameters to prevent memory leaks.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_CANNOT_ACQUIRE_RIGHTS - Rights can not be acquired for this content using the specified method.
//! - DX_ERROR_OPERATION_CANCELED - The user canceled the rights acquisition process.
//! - DX_ERROR_OPERATION_FAILED - The rights acquisition failed.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_AcquireRightsExtended(HDxDrmFile drmFile, EDxRightsAcquisitionMethod acqMethod, DxUint32 contentIndex, HDxUserMessage* userMessageToReturn);

//! Prepare the content for super distribution or backup.
//! This usually involve embedding of rights in the content.
//! For OMA V2 this operation also updates the Transaction ID that is in the DCF (if exists).
//! \param[in] drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_PrepareForSuperDistribution(HDxDrmFile drmFile);

//! This function adjust the clock of the specified file.
//! \param[in] drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in] autoMode     Indicates if the DRM agent is allowed to display user interface
//!							and communicate with a server during the process (if necessary).
//!							There are several modes of operation:
//!							- DX_AUTO_DISABLED - Don't perform any additional action.
//!							- DX_AUTO_NO_UI_AND_COMMUNICATION - if user interface or communication is needed the operation will fail.
//!							- DX_AUTO_NO_UI - if user interface is needed the operation will fail.
//!							- DX_AUTO_ENABLED - if user interface and communication are allowed.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_NOT_SUPPORTED - Clock cannot be adjusted for this file.
//! - DX_ERROR_OPERATION_CANCELED - The user canceled the clock adjustment process.
//! - DX_ERROR_OPERATION_FAILED - The clock adjustment failed.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_AdjustClock(HDxDrmFile drmFile, EDxAutoMode autoMode);

//! NOT SUPPORTED in PLAYREADY
//! This functions prepares the file for preview.
//! If the file has "Preview-Rights" it acquires preview rights silently.
//! If the file has instant preview it sets the preview content as the active content.
//! If the file has no preview url/uri DX_ERROR_NOT_SUPPORTED is returned
//! \param[in] drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in] autoMode     Indicates if the DRM agent is allowed to display user interface
//!							and communicate with a server during the process (if necessary).
//!							There are several modes of operation:
//!							- DX_AUTO_DISABLED - Don't perform any additional action.
//!							- DX_AUTO_NO_UI_AND_COMMUNICATION - if user interface or communication is needed the operation will fail.
//!							- DX_AUTO_NO_UI - if user interface is needed the operation will fail.
//!							- DX_AUTO_ENABLED - if user interface and communication are allowed.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_NOT_SUPPORTED - File has no preview.
//! - DX_ERROR_CANNOT_ACQUIRE_RIGHTS - Preview rights can not be acquired for this content silently.
//! - DX_ERROR_OPERATION_FAILED - Acquisition of preview rights failed.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_StartPreview(HDxDrmFile drmFile, EDxAutoMode autoMode);

//! NOT SUPPORTED in PLAYREADY
//! Installs the rights that are embedded in the content.
//! This operation may require communication with the Rights Issuer.
//! The DRM agent will try to perform contact the Rights Issuer (if required)
//! silently (i.e. without asking the user). If the operation cannot be performed
//! silently the user will be asked for its consent unless it is not permitted by autoMode level.
//! \param[in] drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in] autoMode     Indicates if the DRM agent is allowed to display user interface
//!							and communicate with a server during the process (if necessary).
//!							There are several modes of operation:
//!							- DX_AUTO_DISABLED - Don't perform any additional action.
//!							- DX_AUTO_NO_UI_AND_COMMUNICATION - if user interface or communication is needed the operation will fail.
//!							- DX_AUTO_NO_UI - if user interface is needed the operation will fail.
//!							- DX_AUTO_ENABLED - if user interface and communication are allowed.
//! \param[out] rightsWereInstalled If not NULL it will be true on exit only if rights 
//!                                 were actually installed during the process.
//! \note The function succeeds also if no rights were actually installed.
//!     This may happen is there are no embedded rights at all or if the embedded
//!     rights already expired or were already installed.
//! \return
//! - DX_SUCCESS - Operation completed successfully. 
//! - DX_ERROR_OPERATION_CANCELED - The user canceled the installation process.
//! - DX_ERROR_OPERATION_FAILED - Installation operation failed.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_InstallEmbeddedRights(HDxDrmFile drmFile, EDxAutoMode autoMode, DxBool* rightsWereInstalled);

//! Use this function to set a UI context which might be needed by the MessageBox implementation
//! (e.g. A handle to the owner window of the message box in winCE). If UI context is set, it will be passed to all DRM Objects
//! created by this module.
//! Note: Some platforms do not require this parameter to be set.
//! \param[in] drmFile		A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in] uiContext	A pointer to the UI context to be set.
DX_FUNC DRM_C_API void DxDrmFile_SetUiContext(HDxDrmFile drmFile, void* uiContext);

//! Retrieves the UI Context previously set.
DX_FUNC DRM_C_API void* DxDrmFile_GetUiContext(HDxDrmFile drmFile);

//! Closes the file handle.
DX_FUNC DRM_C_API void DxDrmFile_Close(HDxDrmFile* drmFile);

//! Gets a license challenge for the file handle given (WMDRM ONLY). this challenge is part of the License Acquisition process in WMDRM this challenge should be sent to the license server and the returning license file imported to the DRM via DxWmDrmDLA_StoreLicense.
//! \param[in] drmFile	     A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[out] challenge    A pointer to the buffer to hold the License challenge data. if not null the challenge data will be copied to this buffer.
//! \param[in, out] size     A pointer to the size of the challenge buffer. this will hold the size of the license challenge. May not be null.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile or size is null.
//! - DX_ERROR_BUFFER_IS_TOO_SMALL - buffer pointer is null or buffer size given is too small, in this case the needed size will be returned in the size parameter.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_GetLicenseChallenge(HDxDrmFile drmFile,  void* challenge, DxUint32* size);

//! Process License Response.
//! \param[in] drmFile	     A handle previously created by DxDrmClient_OpenDrmFile()
//! \param[in]	serverResponse		Buffer that contains the server response.
//! \param[in]	serverResponseLen	serverResponse buffer length
//! \param[out]	responseResult 		HDxResponseResult handler that contains the process response result. (needed for the acknowledgment challenge) .
//!									in case the (*isAckRequired == DX_TRUE) , the user must deallocate this handle to prevent memory leaks 
//! \param[out]	isAckRequired		DxBool pointer that indicates if the server needs acknowledgment for the license or not.
//!									If (*isAckRequired == DX_TRUE) then there is a need to call DxDrmClient_GetLicenseAcq_GenerateAck() with the license response result
//!									to create a challenge that should been sent to the server back. isAckRequired may be NULL.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_ProcessLicenseResponse(HDxDrmFile drmFile, const void* serverResponse, DxUint32 serverResponseSize, void** serverResponseResult , DxBool* isAckRequired);


//! Retrieves the Maximal output protection levels for the given content. this is only relevant for WMDRm contents.
//! \param[in] drmFile					A handle previously created by DxDrmClient_OpendrmFile()
//! \param[out] audioCompressedLevel		On success will return the compressed audio protection level
//! \param[out] audioUncompressedLevel		On success will return the uncompressed audio protection level
//! \param[out] videoCompressedLevel		On success will return the compressed video protection level
//! \param[out] videoUncompressedLevel		On success will return the uncompressed video protection level
//! \param[out] videoAnalogLevel			On success will return the analog video protection level
//! \return
//! - DX_SUCCESS - Operation completed successfully. 
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmFile_GetCurrentMaxProtectionLevel(HDxDrmFile drmFile, DxUint32* audioCompressedLevel, DxUint32* audioUncompressedLevel, DxUint32* videoCompressedLevel, DxUint32* videoUncompressedLevel, DxUint32* videoAnalogLevel);

//! Retrieves whether the Content license contains the Miracast Enabler object (permission to send decrypted content over HDCP 2.1 and above).
//! \param[in] drmFile					A handle previously created by DxDrmClient_OpendrmFile()
DX_FUNC DRM_C_API DxBool DxDrmFile_HasMiracastEnabler(HDxDrmFile drmFile);

#ifdef __cplusplus
}
#endif

#endif

