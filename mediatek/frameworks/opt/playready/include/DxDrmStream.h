#ifndef DX_DRM_STREAM_H
#define DX_DRM_STREAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "DxRightsObject.h"

//! \file DxDrmStream.h
//! \brief DxDrmStream is used to perform operations on DRM files.
//!
//! A HDxDrmStream handle is associated with an opened DRM file. It is created by calling DxDrmClient_OpenDrmStream().
//! A DRM stream may include several contents (tracks). By default the first content is the active content. The active content is the content
//! that is currently consumed. The active content may be changed using DxDrmStream_SetActiveContent(). 
//! HDxDrmStream provides the following features:
//! - Extraction of content related attributes such as:  Title, Author, Description, etc. 
//! - Query about DRM status of the content (i.e. Is the content encrypted? Do rights exists?)
//! - Decryption of packets by calling DxDrmStream_ProcessPacket().
//! - Consumption of content rights.
//! - Extraction of content's right information.


//! HDxDrmStream is a handle associated with an opened DRM stream. It is created by calling DxDrmClient_OpenDrmStream().
typedef void* HDxDrmStream;

//! NOT SUPPORTED in PLAYREADY
//! Returns the number of DRM protected tracks in the DRM stream.
//! Tracks which are not DRM protected will be ignored by the DxDrmStream module.
//! \param[in] drmStream		A handle previously created by DxDrmClient_OpenDrmStream()
DX_FUNC DRM_C_API DxUint32 DxDrmStream_GetNumOfContents(HDxDrmStream drmStream);

//! Returns the value of the specified text attribute.
//! The returned pointer points to memory that exists until the next call to GetTextAttribute().
//! The caller MUST NOT free this memory.
//! If the attribute could not be found or invalid contentIndex was specified, NULL will be returned.
//! \param[in] drmStream	A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[in] textAttr     Specifies the attribute to retrieve. 
//! \param[in] contentIndex The index of the content whose attribute should be retrieved.
//!                         If the value is DX_ACTIVE_CONTENT the active content will be used.
DX_FUNC DRM_C_API const DxChar* DxDrmStream_GetTextAttribute(HDxDrmStream drmStream, EDxTextAttribute textAttr, DxUint32 contentIndex);

//! Returns the value of the text attribute specified by attrName.
//! The returned pointer points to memory that exists until the next call to GetTextAttribute().
//! The caller MUST NOT free this memory.
//! If the attribute could not be found or invalid contentIndex was specified, NULL will be returned.
//! \param[in] drmStream	A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[in] attrName     The name of the attribute to retrieve. This name is DRM scheme specific.
//! \param[in] contentIndex The index of the content whose attribute should be retrieved.
//!                         If the value is DX_ACTIVE_CONTENT the active content will be used.
DX_FUNC DRM_C_API const DxChar* DxDrmStream_GetTextAttributeByName(HDxDrmStream drmStream, const DxChar* attrName, DxUint32 contentIndex);

//! Returns the number of text attributes that exists for the specified content.
//! If invalid contentIndex was specified, 0 will be returned.
//! \param[in] drmStream	A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[in] contentIndex The index of the content whose number of attributes should be retrieved.
//!                         If the value is DX_ACTIVE_CONTENT the active content will be used.
DX_FUNC DRM_C_API DxUint32 DxDrmStream_GetNumOfTextAttributes(HDxDrmStream drmStream, DxUint32 contentIndex);

//! Retrieves the name & value of the attribute that is placed in the specified index in the
//! attribute list of the specified content.
//! \param[in]  drmStream		A handle previously created by DxDrmClient_OpenDrmStream()
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
//! - DX_ERROR_BAD_ARGUMENTS - attrIndex or contentIndex are invalid, or drmStream is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_GetTextAttributeByIndex(HDxDrmStream drmStream, DxUint32 attrIndex, const DxChar** attrName, const DxChar** attrValue, DxUint32 contentIndex);

//! NOT SUPPORTED in PLAYREADY
//! \return True if the value of the attribute specified by textAttr can be set using SetTextAttribute().
DX_FUNC DRM_C_API DxBool DxDrmStream_IsAttributeEditable(HDxDrmStream drmStream, EDxTextAttribute textAttr);

//! NOT SUPPORTED in PLAYREADY
//! Sets the value of the specified text attribute.
//! \param[in] drmStream		A handle previously created by DxDrmClient_OpenDrmStream()
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
//! - DX_ERROR_BAD_ARGUMENTS - contentIndex is invalid, or drmStream is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_SetTextAttribute(HDxDrmStream drmStream, EDxTextAttribute textAttr, const DxChar* attrValue, const DxChar* languageCode, DxUint32 contentIndex);

//! Retrieve set of flags that are available for the specified content.
//! \param[in]  drmStream		A handle previously created by DxDrmClient_OpenDrmStream()
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
//! - DX_ERROR_BAD_ARGUMENTS - If drmStream is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_GetFlags(HDxDrmStream drmStream, DxUint32 flagsToCheck, DxUint32* activeFlags, DxUint32 permissions, DxUint32 contentIndex);

//! NOT SUPPORTED in PLAYREADY
//! Indicates if rights for the current content and intent are going to be expired.
//! The rights are not considered to be expired if the remaining count exceeds countThreshold or
//! if the number of seconds left until rights expire exceeds timeThreshold.
//! If countsLeft and timeLeft are not null, then on exit they will hold the values of the total counts and time left.
//! Only valid rights (and no future rights) of the current intent will be taken into account in this calculation.
//! \param[in]  drmStream			A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[in]  countThreshold		The count threshold
//! \param[in]  timeThreshold		The time threshold
//! \param[out] isGoingToExpire		On exit will point to boolean value with the result
//! \param[out] countsLeft			On exit will hold the value of total counts left.
//! \param[out] timeLeft			On exit will hold the value of seconds left.
//! \return 
//! - DX_SUCCESS - On Success
//! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
//! - DX_ERROR_BAD_ARGUMENTS - If drmStream is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_IsSoonToBeExpired(HDxDrmStream drmStream, DxUint32 countThreshold, DxUint32 timeThreshold,
												   DxBool* isGoingToExpire, DxUint32* countsLeft, DxUint32* timeLeft);
//! NOT SUPPORTED in PLAYREADY
//! Change the active content of the file to the specified content.
//! \param[in] drmStream		A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[in] contentIndex		The index of the content whose attribute should be retrieved.
//!                             If the value is DX_ACTIVE_CONTENT the active content will be used.
//! \return
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_BAD_ARGUMENTS - If contentIndex is not a valid content index, or drmStream is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_SetActiveContent(HDxDrmStream drmStream, DxUint32 contentIndex);

//! NOT SUPPORTED in PLAYREADY
//! Change the active content of the stream to the content which matches the given trackId.
//! \param[in] drmStream		A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[in] trackId			A unique ID which represents a track in the stream object.
//! \return
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_BAD_ARGUMENTS - If contentIndex is not a valid content index, or drmStream is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_SetActiveContentByTrackId(HDxDrmStream drmStream, DxUint32 trackId);

//! NOT SUPPORTED in PLAYREADY
//! Returns the currently active content index (zero based)
//! \param[in] drmStream		A handle previously created by DxDrmClient_OpenDrmStream()
DX_FUNC DRM_C_API DxUint32 DxDrmStream_GetActiveContent(HDxDrmStream drmStream);

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
//! \param[in]  drmStream		A handle previously created by DxDrmClient_OpenDrmStream()
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
//! - DX_ERROR_BAD_ARGUMENTS - If drmStream is null.
//! - DX_ERROR_OPERATION_CANCELED_BY_USER
//! - DX_ERROR_OPERATION_FAILED
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_SetIntent(HDxDrmStream drmStream, EDxIntent intentVal, EDxAutoMode autoMode);


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
//! \param[in]  drmStream			A handle previously created by DxDrmClient_OpenDrmStream()
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
//! - DX_ERROR_BAD_ARGUMENTS - If drmStream is null.
//! - DX_ERROR_OPERATION_CANCELED_BY_USER
//! - DX_ERROR_OPERATION_FAILED
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_SetIntentEx(HDxDrmStream drmStream, EDxIntent intentVal, EDxAutoMode autoMode, HDxUserMessage* userMessageToReturn);

//! Retrieves the current DRM clock relevant for the open content. Secure clock is unique per riId.
//! \param[in]  drmStream		A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[out] timeStruct  On entry, empty struct. On exit, the current value of the 
//!                         DRM clock of the right issuer linked to this content.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - The DRM clock of the content is not set, or no riId linked to the content (when no rights).
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_GetSecureClock(HDxDrmStream drmStream, DxTimeStruct* timeStruct);

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
//! The DxDrmStream_HandleConsumptionEvent() function should be called only after the intent was set using DxDrmStream_SetIntent().
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
//! - DX_ERROR_BAD_ARGUMENTS - If drmStream is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_HandleConsumptionEvent(HDxDrmStream drmStream, EDxConsumptionEvent eventVal);

//! NOT SUPPORTED in PLAYREADY
//! Retrieve a pointer to the rights object handle that is currently being used by the DRM system for
//! the active content. The calling application MUST NOT delete this object.
//! The object is valid as long as the active RO does not change.
//! \param[in]  drmStream	A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[out] activeRo	On success will point to the active RO handle. 
//! \return 
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_NO_RIGHTS - There are no rights for the active content.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - The rights for the active content are time based and the secure clock is not set.
//! - DX_ERROR_INTENT_WAS_NOT_SET - The function was called before the intent was set.
//! - DX_ERROR_BAD_ARGUMENTS - If drmStream or activeRo are null.
DX_FUNC DRM_C_API HDxRightsObject DxDrmStream_GetActiveRO(HDxDrmStream drmStream);

//! Returns a handle to one of the rights objects that are stored in the secure storage for
//! the active content. The rights object is identified by its zero-based index. 
//! If index is greater or equal to the value returned by DxDrmStream_GetNumOfROs() NULL will be returned.
//! The calling application MUST NOT delete this object.
//! The object is valid until the next call to DxDrmStream_GetRO() (i.e. you need to process the RO info
//! before moving to the next RO).
//! Note: It is necessary to call DxDrmStream_PrepareForROIteration() before calling this function.
//! \param[in] drmStream	A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[in] roIndex		Zero-based index of the requested rights object.
DX_FUNC DRM_C_API HDxRightsObject DxDrmStream_GetRO(HDxDrmStream drmStream, DxUint32 roIndex);

//! Returns the combined rights information of the given stream from all the ROs related to the content.
//! \param[out] countsLeft			The times left to play this content.
//! \param[out] intervalTimeLeft	The interval time left to play this content.
//! \param[out] accumulatedTimeLeft	The accumulated time left to play this content.
//! \param[out] timeLeft			The time left to play this content.
//! \param[out] hasUnlimitedRights	Indicates if this content has unlimited rights.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_GetRightsInformation(HDxDrmStream drmStream, DxUint32* countsLeft, DxUint32* intervalTimeLeft, DxUint32* accumulatedTimeLeft, DxUint32* timeLeft, DxBool* hasUnlimitedRights);

//! Prepares the content for an iteration over it's ROs. It is mandatory to call this function before calling DxDrmStream_GetRO().
//! It sets the number of available ROs in the numOfROs parameter.
//! \param[in]  drmStream	   A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[in]  iterationMode  Can be one of the following values:
//!                            - DX_RO_ITERATION_DISPLAY: Prepares iteration only over the ROs which aren't expired.
//!                                Also, if similar ROs are found, their information is combined for preview purposes.
//!                            - DX_RO_ITERATION_FULL_INFO: Prepares iteration over all rights objects that are stored
//!                                in the secure storage for the active content.
//! \param[out] numOfROs	   On success will hold the number of ROs available for iteration. 
//! \return 
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile or activeRo are null, or an invalid iterationMode given.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_PrepareForROIteration(HDxDrmStream drmStream, EDxROIterationMode iterationMode, DxUint32* numOfROs);

//! Deletes all the ROs of the active content from the Secure Storage.
//! \param[in] drmStream	A handle previously created by DxDrmClient_OpenDrmStream()
//! \return 
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - If drmStream is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_DeleteRights(HDxDrmStream drmStream);

//! Deletes all the expired ROs of the active content from the Secure Storage.
//! \param[in] drmStream		A handle previously created by DxDrmClient_OpenDrmStream()
//! \return 
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - If drmStream is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_DeleteObsoleteRights(HDxDrmStream drmStream);

//! This function acquires rights for the content specified by contentIndex 
//! using the method specified by ackMethod.
//! \param[in] drmStream			A handle previously created by DxDrmClient_OpenDrmStream()
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
//! - DX_ERROR_BAD_ARGUMENTS - If drmStream is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_AcquireRights(HDxDrmStream drmStream, EDxRightsAcquisitionMethod acqMethod, DxUint32 contentIndex);

//! NOT SUPPORTED in PLAYREADY
//! This function acquires rights for the content specified by contentIndex from the rights issuer url (DX_ATTR_RI_URL).
//! It prompts the user when needed according to specification.
//! \param[in] drmStream			A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[in] contentIndex		The index of the content whose attribute should be retrieved.
//!                             If the value is DX_ACTIVE_CONTENT the active content will be used.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_CANNOT_ACQUIRE_RIGHTS - Rights can not be acquired for this content.
//! - DX_ERROR_OPERATION_CANCELED - The user canceled the rights acquisition process.
//! - DX_ERROR_OPERATION_FAILED - The rights acquisition failed.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - If drmStream is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_AcquireRightsEx(HDxDrmStream drmStream, DxUint32 contentIndex);

//! This function acquires rights for the content specified by contentIndex 
//! using the method specified by ackMethod.
//! \param[in] drmStream			A handle previously created by DxDrmClient_OpenDrmStream()
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
//! - DX_ERROR_BAD_ARGUMENTS - If drmStream is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_AcquireRightsExtended(HDxDrmStream drmStream, EDxRightsAcquisitionMethod acqMethod, DxUint32 contentIndex, HDxUserMessage* userMessageToReturn);

//! Prepare the content for super distribution or backup.
//! This usually involve embedding of rights in the content.
//! For OMA V2 this operation also updates the Transaction ID that is in the (P)DCF (if exists).
//! \param[in] drmStream		A handle previously created by DxDrmClient_OpenDrmStream()
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_BAD_ARGUMENTS - If drmStream is null.
//! - DX_ERROR_FILE_IS_NOT_FORWARDABLE - If content cannot be forwarded.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_PrepareForSuperDistribution(HDxDrmStream drmStream);

//! This function adjust the clock of the specified stream.
//! \param[in] drmStream	A handle previously created by DxDrmClient_OpenDrmStream()
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
//! - DX_ERROR_BAD_ARGUMENTS - If drmStream is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_AdjustClock(HDxDrmStream drmStream, EDxAutoMode autoMode);

//! NOT SUPPORTED in PLAYREADY
//! This functions prepares the stream for preview.
//! If the stream has "Preview-Rights" it acquires preview rights silently.
//! If the stream has instant preview it sets the preview content as the active content.
//! If the stream has no preview url/uri DX_ERROR_NOT_SUPPORTED is returned
//! \param[in] drmStream	A handle previously created by DxDrmClient_OpenDrmStream()
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_NOT_SUPPORTED - Stream has no preview.
//! - DX_ERROR_CANNOT_ACQUIRE_RIGHTS - Preview rights can not be acquired for this content silently.
//! - DX_ERROR_OPERATION_FAILED - Acquisition of preview rights failed.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - If drmStream is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_StartPreview(HDxDrmStream drmStream, EDxAutoMode autoMode);

//! NOT SUPPORTED in PLAYREADY
//! Installs the rights that are embedded in the content.
//! This operation may require communication with the Rights Issuer.
//! The DRM agent will try to perform contact the Rights Issuer (if required)
//! silently (i.e. without asking the user). If the operation cannot be performed
//! silently the user will be asked for its consent unless it is not permitted by autoMode level.
//! \param[in] drmStream	A handle previously created by DxDrmClient_OpenDrmStream()
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
//! - DX_ERROR_BAD_ARGUMENTS - If drmStream is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_InstallEmbeddedRights(HDxDrmStream drmStream, EDxAutoMode autoMode, DxBool* rightsWereInstalled);

//! The ProcessPacket() function receives trackId and the packet data. It decrypts the packet and updates its new size.
//! \param[in]     drmStream	     A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[in]	   trackId           Id of the track in the stream that is to be decrypted.
//! \param[in]	   packetData        Points to the encrypted packet data.
//! \param[in]	   packetSize        Size of packetData.
//! \param[in]     decryptedPacket   A pointer to an allocated buffer that will be filled with the decrypted data.
//!                                  Can be the same address as packetData.
//! \param[in/out] outputPacketSize  Size of the decryptedPacket buffer. On exit will hold the size of the decrypted data
//!	                                 (might be smaller than the encrypted data).
//! \return
//! - DX_SUCCESS - On Success
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NO_RIGHTS - If there are no rights for the active content.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - If the rights for the active content are time based and the secure clock is not set.
//! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
//! - DX_ERROR_BAD_ARGUMENTS - If invalid arguments were passed.
//! - DX_NOT_AVAILABLE - Track or AUFormatBox was not found.
//! - DX_ERROR_INVALID_FORMAT - Packet data has invalid format.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_ProcessPacket(HDxDrmStream drmStream, DxUint32 trackId, const void* packetData, DxUint32 packetSize, void* decryptedPacket, DxUint32* outputPacketSize);

//! The ProcessPiffPacket() function receives trackId and the packet data. It decrypts the packet and updates its new size.
//! \param[in]     drmStream	     A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[in]	   trackId           Id of the track in the stream that is to be decrypted.
//! \param[in]	   sampleData        Points to the encrypted packet data.
//! \param[in]	   sampleDataSize    Size of packetData.
//! \param[in]	   samlpeEncryptBoxData    Points to the data of the sample encryption box for this sample in the piff file
//! \param[in]	   sampleEcryptBoxDataSize    Size of samlpeEncryptBoxData.
//! \param[in]     sampleIndex	   index of the sample within the mdat box
//! \return
//! - DX_SUCCESS - On Success
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NO_RIGHTS - If there are no rights for the active content.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - If the rights for the active content are time based and the secure clock is not set.
//! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
//! - DX_ERROR_BAD_ARGUMENTS - If invalid arguments were passed.
//! - DX_NOT_AVAILABLE - Track or AUFormatBox was not found.
//! - DX_ERROR_INVALID_FORMAT - Packet data has invalid format.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_ProcessPiffPacket(HDxDrmStream drmStream, void* sampleData,DxUint32 sampleDataSize, 
															  const void* encryptBoxData ,  DxUint32 encryptBoxDataSize,
															  DxUint32 sampleIndex,  DxUint32 trackId);

//! The ProcessPayload() function receives the trackId and the payload data. It decrypts the payload in place.
//! \param[in]     drmStream	     A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[in]	   trackId           Id of the track in the stream that is to be decrypted.
//!									 This has importance if each track is encrypted by a different key.
//! \param[in]	   iv		         Initial vector required for decryption.
//! \param[in]	   ivSize	         Size of the initial vector. For PlayReady it should be 64 bits.
//! \param[in]	   blockOffset	     Block offset within the payload (16 bytes per block).
//! \param[in]	   byteOffset        Byte offset within the last payload block.
//! \param[in]	   payloadData       Points to the encrypted payload data. Decryption is done in-place.
//! \param[in]	   payloadDataSize   Size of the payloadData.
//! \param[out]	   decryptedPayloadSize	 On exit will hold the size of the decrypted data.
//!	                                 (might be smaller than the encrypted payload size in case of padding).
//! \param[in]	   last15bytes		 Last 15 bytes of the payload required for WMDRM payload decryption.
//! \param[in]	   last15bytesSize	 Size of the last 15 bytes (which might be less than 15 in small payloads).
//! \param[in]	   paddingScheme     Padding scheme used for encryption. If null then no padding is assumed.
//! \return
//! - DX_SUCCESS - On Success
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NO_RIGHTS - If there are no rights for the active content.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - If the rights for the active content are time based and the secure clock is not set.
//! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
//! - DX_ERROR_BAD_ARGUMENTS - If invalid arguments were passed.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_ProcessPayload(
	HDxDrmStream drmStream,
	DxUint32 trackId,
	const void* iv,
	DxUint32 ivSize,
	DxUint32 blockOffset,
	DxUint32 byteOffset,
	void* payloadData,
	DxUint32 payloadDataSize,
	DxUint32* decryptedPayloadSize,
	const void* last15bytes,
	DxUint32 last15bytesSize,
	const DxChar* paddingScheme );


//! The ProcessPayloadEx() function receives the trackId and the payload data. It decrypts the payload in place.
//! \param[in]     drmStream	     A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[in]	   trackId           Id of the track in the stream that is to be decrypted.
//!									 This has importance if each track is encrypted by a different key.
//! \param[in]	   iv		         Initial vector required for decryption.
//! \param[in]	   ivSize	         Size of the initial vector. For PlayReady it should be 64 bits.
//! \param[in]	   blockOffset	     Block offset within the payload (16 bytes per block).
//! \param[in]	   byteOffset        Byte offset within the last payload block.
//! \param[in]	   payloadData       Points to the encrypted payload data.
//! \param[in]	   payloadDataSize   Size of the payloadData.
//! \param[out]	   decryptedPayloadData       Points to the decrypted payload data. 
//! \param[out]	   decryptedPayloadSize	 On exit will hold the size of the decrypted data.
//!	                                 (might be smaller than the encrypted payload size in case of padding).
//! \param[in]	   last15bytes		 Last 15 bytes of the payload required for WMDRM payload decryption.
//! \param[in]	   last15bytesSize	 Size of the last 15 bytes (which might be less than 15 in small payloads).
//! \param[in]	   paddingScheme     Padding scheme used for encryption. If null then no padding is assumed.
//! \param[in]	   isSecureBuffer     indicator if the input buffer is HW secured buffer 
//! \return
//! - DX_SUCCESS - On Success
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NO_RIGHTS - If there are no rights for the active content.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - If the rights for the active content are time based and the secure clock is not set.
//! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
//! - DX_ERROR_BAD_ARGUMENTS - If invalid arguments were passed.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_ProcessPayloadEx(
	HDxDrmStream drmStream,
	DxUint32 trackId,
	const void* iv,
	DxUint32 ivSize,
	DxUint32 blockOffset,
	DxUint32 byteOffset,
	const void* payloadData,
	DxUint32 payloadDataSize,
	void* decryptedPayloadData,
	DxUint32* decryptedPayloadSize,
	const void* last15bytes,
	DxUint32 last15bytesSize,
	const DxChar* paddingScheme,
	DxBool isSecureBuffer );
//! Use this function to set a UI context which might be needed by the MessageBox implementation
//! (e.g. A handle to the owner window of the message box in winCE). If UI context is set, it will be passed to all DRM Objects
//! created by this module.
//! Note: Some platforms do not require this parameter to be set.
//! \param[in] drmStream	A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[in] uiContext	A pointer to the UI context to be set.
DX_FUNC DRM_C_API void DxDrmStream_SetUiContext(HDxDrmStream drmStream, void* uiContext);

//! Retrieves the UI Context previously set.
DX_FUNC DRM_C_API void* DxDrmStream_GetUiContext(HDxDrmStream drmStream);

//! Closes the stream handle.
DX_FUNC DRM_C_API void DxDrmStream_Close(HDxDrmStream* drmStream);

//! Gets a license challenge for the file handle given (WMDRM ONLY). this challenge is part of the License Acquisition process in WMDRM this challenge should be sent to the license server and the returning license file imported to the DRM via DxWmDrmDLA_StoreLicense.
//! \param[in] drmFile	     A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[out] challenge    A pointer to the buffer to hold the License challenge data. if not null the challenge data will be copied to this buffer.
//! \param[in, out] size     A pointer to the size of the challenge buffer. this will hold the size of the license challenge. May not be null.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile or size is null.
//! - DX_ERROR_BUFFER_IS_TOO_SMALL - buffer pointer is null or buffer size given is too small, in this case the needed size will be returned in the size parameter.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_GetLicenseChallenge(HDxDrmStream drmStream,  void* challenge, DxUint32* size);

//! Process License Response.
//! \param[in] drmFile	     A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[in]	serverResponse		Buffer that contains the server response.
//! \param[in]	serverResponseLen	serverResponse buffer length
//! \param[out]	responseResult 		HDxResponseResult handler that contains the process response result. (needed for the acknowledgment challenge) .
//!									in case the (*isAckRequired == DX_TRUE) , the user must deallocate this handle to prevent memory leaks
//! \param[out]	isAckRequired		DxBool pointer that indicates if the server needs acknowledgment for the license or not.
//!									If (*isAckRequired == DX_TRUE) then there is a need to call DxDrmClient_GetLicenseAcq_GenerateAck() with the license response result
//!									to create a challenge that should been sent to the server back. isAckRequired may be NULL.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_ProcessLicenseResponse(HDxDrmStream drmStream, const void* serverResponse, DxUint32 serverResponseSize, void** serverResponseResult , DxBool* isAckRequired);

//! Retrieves the Maximal output protection levels for the given content. this is only relevant for WMDRm contents.
//! \param[in] drmStream					A handle previously created by DxDrmClient_OpendrStream()
//! \param[out] audioCompressedLevel		On success will return the compressed audio protection level
//! \param[out] audioUncompressedLevel		On success will return the uncompressed audio protection level
//! \param[out] videoCompressedLevel		On success will return the compressed video protection level
//! \param[out] videoUncompressedLevel		On success will return the uncompressed video protection level
//! \param[out] videoAnalogLevel			On success will return the analog video protection level
//! \return
//! - DX_SUCCESS - Operation completed successfully. 
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_GetCurrentMaxProtectionLevel(HDxDrmStream drmStream, DxUint32* audioCompressedLevel, DxUint32* audioUncompressedLevel, DxUint32* videoCompressedLevel, DxUint32* videoUncompressedLevel, DxUint32* videoAnalogLevel);

//! Retrieves whether the Content license contains the Miracast Enabler object (permission to send decrypted content over HDCP 2.1 and above).
//! \param[in] drmStream					A handle previously created by DxDrmClient_OpendrStream()
DX_FUNC DRM_C_API DxBool DxDrmStream_HasMiracastEnabler(HDxDrmStream drmStream);

#ifdef __cplusplus
}
#endif

#endif

