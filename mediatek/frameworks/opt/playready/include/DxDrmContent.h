#ifndef DX_DRM_CONTENT_H
#define DX_DRM_CONTENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "DxRightsObject.h"

//! \file DxdrmContent.h
//! \brief DxdrmContent is used to perform operations on DRM files.
//!
//! A HDxDrmContent handle is associated with an opened DRM file. It is created by calling DxDrmClient_OpendrmContent().
//! A DRM stream may include several contents (tracks). By default the first content is the active content. The active content is the content
//! that is currently consumed. The active content may be changed using DxDrmContent_SetActiveContent(). 
//! HDxDrmContent provides the following features:
//! - Extraction of content related attributes such as:  Title, Author, Description, etc. 
//! - Query about DRM status of the content (i.e. Is the content encrypted? Do rights exists?)
//! - Decryption of packets by calling DxDrmContent_ProcessPacket().
//! - Consumption of content rights.
//! - Extraction of content's right information.


//! HDxDrmContent is a handle associated with an opened DRM stream. It is created by calling DxDrmClient_OpendrmContent().
typedef void* HDxDrmContent;

DX_FUNC DRM_C_API EDxDrmScheme DxDrmContent_GetDRMScheme(HDxDrmContent drmContent);

DX_FUNC DRM_C_API EDxDrmContentType DxDrmContent_GetDRMContentType(HDxDrmContent drmContent);

//! NOT SUPPORTED in PLAYREADY
//! Returns the number of DRM protected tracks in the DRM stream.
//! Tracks which are not DRM protected will be ignored by the DxdrmContent module.
//! \param[in] drmContent		A handle previously created by DxDrmClient_OpendrmContent()
DX_FUNC DRM_C_API DxUint32 DxDrmContent_GetNumOfContents(HDxDrmContent drmContent);

//! Returns the value of the specified text attribute.
//! The returned pointer points to memory that exists until the next call to GetTextAttribute().
//! The caller MUST NOT free this memory.
//! If the attribute could not be found or invalid contentIndex was specified, NULL will be returned.
//! \param[in] drmContent	A handle previously created by DxDrmClient_OpendrmContent()
//! \param[in] textAttr     Specifies the attribute to retrieve. 
//! \param[in] contentIndex The index of the content whose attribute should be retrieved.
//!                         If the value is DX_ACTIVE_CONTENT the active content will be used.
DX_FUNC DRM_C_API const DxChar* DxDrmContent_GetTextAttribute(HDxDrmContent drmContent, EDxTextAttribute textAttr, DxUint32 contentIndex);

//! Returns the value of the text attribute specified by attrName.
//! The returned pointer points to memory that exists until the next call to GetTextAttribute().
//! The caller MUST NOT free this memory.
//! If the attribute could not be found or invalid contentIndex was specified, NULL will be returned.
//! \param[in] drmContent	A handle previously created by DxDrmClient_OpendrmContent()
//! \param[in] attrName     The name of the attribute to retrieve. This name is DRM scheme specific.
//! \param[in] contentIndex The index of the content whose attribute should be retrieved.
//!                         If the value is DX_ACTIVE_CONTENT the active content will be used.
DX_FUNC DRM_C_API const DxChar* DxDrmContent_GetTextAttributeByName(HDxDrmContent drmContent, const DxChar* attrName, DxUint32 contentIndex);

//! Returns the number of text attributes that exists for the specified content.
//! If invalid contentIndex was specified, 0 will be returned.
//! \param[in] drmContent	A handle previously created by DxDrmClient_OpendrmContent()
//! \param[in] contentIndex The index of the content whose number of attributes should be retrieved.
//!                         If the value is DX_ACTIVE_CONTENT the active content will be used.
DX_FUNC DRM_C_API DxUint32 DxDrmContent_GetNumOfTextAttributes(HDxDrmContent drmContent, DxUint32 contentIndex);

//! Retrieves the name & value of the attribute that is placed in the specified index in the
//! attribute list of the specified content.
//! \param[in]  drmContent		A handle previously created by DxDrmClient_OpendrmContent()
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
//! - DX_ERROR_BAD_ARGUMENTS - attrIndex or contentIndex are invalid, or drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_GetTextAttributeByIndex(HDxDrmContent drmContent, DxUint32 attrIndex, const DxChar** attrName, const DxChar** attrValue, DxUint32 contentIndex);

//! NOT SUPPORTED in PLAYREADY
//! \return True if the value of the attribute specified by textAttr can be set using SetTextAttribute().
DX_FUNC DRM_C_API DxBool DxDrmContent_IsAttributeEditable(HDxDrmContent drmContent, EDxTextAttribute textAttr);

//! Sets the value of the specified text attribute.
//! \param[in] drmContent		A handle previously created by DxDrmClient_OpendrmContent()
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
//! - DX_ERROR_BAD_ARGUMENTS - contentIndex is invalid, or drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_SetTextAttribute(HDxDrmContent drmContent, EDxTextAttribute textAttr, const DxChar* attrValue, const DxChar* languageCode, DxUint32 contentIndex);

//! Retrieve set of flags that are available for the specified content.
//! \param[in]  drmContent		A handle previously created by DxDrmClient_OpendrmContent()
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
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_GetFlags(HDxDrmContent drmContent, DxUint32 flagsToCheck, DxUint32* activeFlags, DxUint32 permissions, DxUint32 contentIndex);

//! NOT SUPPORTED in PLAYREADY
//! Indicates if rights for the current content and intent are going to be expired.
//! The rights are not considered to be expired if the remaining count exceeds countThreshold or
//! if the number of seconds left until rights expire exceeds timeThreshold.
//! If countsLeft and timeLeft are not null, then on exit they will hold the values of the total counts and time left.
//! Only valid rights (and no future rights) of the current intent will be taken into account in this calculation.
//! \param[in]  drmContent			A handle previously created by DxDrmClient_OpendrmContent()
//! \param[in]  countThreshold		The count threshold
//! \param[in]  timeThreshold		The time threshold
//! \param[out] isGoingToExpire		On exit will point to boolean value with the result
//! \param[out] countsLeft			On exit will hold the value of total counts left.
//! \param[out] timeLeft			On exit will hold the value of seconds left.
//! \return 
//! - DX_SUCCESS - On Success
//! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_IsSoonToBeExpired(HDxDrmContent drmContent, DxUint32 countThreshold, DxUint32 timeThreshold,
												   DxBool* isGoingToExpire, DxUint32* countsLeft, DxUint32* timeLeft);

//! Retrieves the current DRM clock relevant for the open content. Secure clock is unique per riId.
//! \param[in]  drmContent			A handle previously created by DxDrmClient_OpendrmContent()
//! \param[out] timeStruct  On entry, empty struct. On exit, the current value of the 
//!                         DRM clock of the right issuer linked to this content.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - The DRM clock of the content is not set, or no riId linked to the content (when no rights).
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_GetSecureClock(HDxDrmContent drmContent, DxTimeStruct* timeStruct);

//! NOT SUPPORTED in PLAYREADY
//! Change the active content of the file to the specified content.
//! \param[in] drmContent		A handle previously created by DxDrmClient_OpendrmContent()
//! \param[in] contentIndex		The index of the content whose attribute should be retrieved.
//!                             If the value is DX_ACTIVE_CONTENT the active content will be used.
//! \return
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_BAD_ARGUMENTS - If contentIndex is not a valid content index, or drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_SetActiveContent(HDxDrmContent drmContent, DxUint32 contentIndex);

//! Returns the currently active content index (zero based)
//! \param[in] drmContent		A handle previously created by DxDrmClient_OpendrmContent()
DX_FUNC DRM_C_API DxUint32 DxDrmContent_GetActiveContent(HDxDrmContent drmContent);

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
//! \param[in]  drmContent		A handle previously created by DxDrmClient_OpendrmContent()
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
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
//! - DX_ERROR_OPERATION_CANCELED_BY_USER - user canceled the rights acquisition operation.
//! - DX_ERROR_OPERATION_FAILED - rights acquisition operation failed.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_SetIntent(HDxDrmContent drmContent, EDxIntent intentVal, EDxAutoMode autoMode);

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
//! \param[in]  drmContent			A handle previously created by DxDrmClient_OpendrmContent()
//! \param[in]  intentVal   		The intent to be used.
//! \param[in]  autoMode			Indicates if the DRM agent is allowed to display user interface
//!									and communicate with a server during the process (if necessary).
//!									There are several modes of operation:
//!									- DX_AUTO_DISABLED - Don't perform any additional action.
//!									- DX_AUTO_NO_UI_AND_COMMUNICATION - if user interface or communication is needed the operation will fail.
//!									- DX_AUTO_NO_UI - if user interface is needed the operation will fail.
//!									- DX_AUTO_ENABLED - if user interface and communication are allowed.
//! \param[out] userMessageToReturn	contains error message to the user.
//! \return
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NO_RIGHTS - There are no rights for the active content.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - The rights for the active content are time based and the secure clock is not set.
//! - DX_ERROR_INVALID_STATE - Intent cannot be changed during consumption (i.e. between DX_EVENT_START & DX_EVENT_STOP)
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
//! - DX_ERROR_OPERATION_CANCELED_BY_USER - user canceled the rights acquisition operation.
//! - DX_ERROR_OPERATION_FAILED - rights acquisition operation failed.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_SetIntentEx(HDxDrmContent drmContent, EDxIntent intentVal, EDxAutoMode autoMode, HDxUserMessage userMessageToReturn);

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
//! The DxDrmContent_HandleConsumptionEvent() function should be called only after the intent was set using DxDrmContent_SetIntent().
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
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_HandleConsumptionEvent(HDxDrmContent drmContent, EDxConsumptionEvent eventVal);

//! NOT SUPPORTED in PLAYREADY
//! Retrieve a pointer to the rights object handle that is currently being used by the DRM system for
//! the active content. The calling application MUST NOT delete this object.
//! The object is valid as long as the active RO does not change.
//! \param[in]  drmContent	A handle previously created by DxDrmClient_OpendrmContent()
//! \param[out] activeRo	On success will point to the active RO handle. 
//! \return 
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_NO_RIGHTS - There are no rights for the active content.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - The rights for the active content are time based and the secure clock is not set.
//! - DX_ERROR_INTENT_WAS_NOT_SET - The function was called before the intent was set.
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent or activeRo are null.
DX_FUNC DRM_C_API HDxRightsObject DxDrmContent_GetActiveRO(HDxDrmContent drmContent);

//! Returns a handle to one of the rights objects that are stored in the secure storage for
//! the active content. The rights object is identified by its zero-based index. 
//! If index is greater or equal to the value returned by DxDrmContent_GetNumOfROs() NULL will be returned.
//! The calling application MUST NOT delete this object.
//! The object is valid until the next call to DxDrmContent_GetRO() (i.e. you need to process the RO info
//! before moving to the next RO).
//! Note: It is necessary to call DxDrmContent_PrepareForROIteration() before calling this function.
//! \param[in] drmContent	A handle previously created by DxDrmClient_OpendrmContent()
//! \param[in] roIndex		Zero-based index of the requested rights object.
DX_FUNC DRM_C_API HDxRightsObject DxDrmContent_GetRO(HDxDrmContent drmContent, DxUint32 roIndex);

//! NOT SUPPORTED in PLAYREADY
//! Returns the combined rights information of the given content from all the ROs related to the content.
//! \param[out] countsLeft			The times left to play this content.
//! \param[out] intervalTimeLeft	The interval time left to play this content.
//! \param[out] accumulatedTimeLeft	The accumulated time left to play this content.
//! \param[out] timeLeft			The time left to play this content.
//! \param[out] hasUnlimitedRights	Indicates if this content has unlimited rights.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_GetRightsInformation(HDxDrmContent drmContent, DxUint32* countsLeft, DxUint32* intervalTimeLeft, DxUint32* accumulatedTimeLeft, DxUint32* timeLeft, DxBool* hasUnlimitedRights);

//! Prepares the content for an iteration over it's ROs. It is mandatory to call this function before calling DxDrmContent_GetRO().
//! It sets the number of available ROs in the numOfROs parameter.
//! \param[in]  drmContent	   A handle previously created by DxDrmClient_OpendrmContent()
//! \param[in]  iterationMode  Can be one of the following values:
//!                            - DX_RO_ITERATION_DISPLAY: Prepares iteration only over the ROs which aren't expired.
//!                                Also, if similar ROs are found, their information is combined for preview purposes.
//!                            - DX_RO_ITERATION_FULL_INFO: Prepares iteration over all rights objects that are stored
//!                                in the secure storage for the active content.
//! \param[out] numOfROs	   On success will hold the number of ROs available for iteration. 
//! \return 
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_BAD_ARGUMENTS - If drmFile or activeRo are null, or an invalid iterationMode given.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_PrepareForROIteration(HDxDrmContent drmContent, EDxROIterationMode iterationMode, DxUint32* numOfROs);

//! Deletes all the ROs of the active content from the Secure Storage.
//! \param[in] drmContent	A handle previously created by DxDrmClient_OpendrmContent()
//! \return 
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_DeleteRights(HDxDrmContent drmContent);

//! Deletes all the expired ROs of the active content from the Secure Storage.
//! \param[in] drmContent		A handle previously created by DxDrmClient_OpendrmContent()
//! \return 
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_DeleteObsoleteRights(HDxDrmContent drmContent);

//! This function acquires rights for the content specified by contentIndex 
//! using the method specified by ackMethod.
//! \param[in] drmContent			A handle previously created by DxDrmClient_OpendrmContent()
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
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_AcquireRights(HDxDrmContent drmContent, EDxRightsAcquisitionMethod acqMethod, DxUint32 contentIndex);

//! NOT SUPPORTED in PLAYREADY
//! This function acquires rights for the content specified by contentIndex from the rights issuer url (DX_ATTR_RI_URL).
//! It prompts the user when needed according to specification.
//! \param[in] drmContent			A handle previously created by DxDrmClient_OpendrmContent()
//! \param[in] contentIndex		The index of the content whose attribute should be retrieved.
//!                             If the value is DX_ACTIVE_CONTENT the active content will be used.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_CANNOT_ACQUIRE_RIGHTS - Rights can not be acquired for this content.
//! - DX_ERROR_OPERATION_CANCELED - The user canceled the rights acquisition process.
//! - DX_ERROR_OPERATION_FAILED - The rights acquisition failed.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_AcquireRightsEx(HDxDrmContent drmContent, DxUint32 contentIndex);

//! Prepare the content for super distribution or backup.
//! This usually involve embedding of rights in the content.
//! For OMA V2 this operation also updates the Transaction ID that is in the (P)DCF (if exists).
//! \param[in] drmContent		A handle previously created by DxDrmClient_OpendrmContent()
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
//! - DX_ERROR_FILE_IS_NOT_FORWARDABLE - If content cannot be forwarded.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_PrepareForSuperDistribution(HDxDrmContent drmContent);

//! This function adjust the clock of the specified stream.
//! \param[in] drmContent	A handle previously created by DxDrmClient_OpendrmContent()
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
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_AdjustClock(HDxDrmContent drmContent, EDxAutoMode autoMode);

//! NOT SUPPORTED in PLAYREADY
//! This functions prepares the stream for preview.
//! If the stream has "Preview-Rights" it acquires preview rights silently.
//! If the stream has instant preview it sets the preview content as the active content.
//! If the stream has no preview url/uri DX_ERROR_NOT_SUPPORTED is returned
//! \param[in] drmContent	A handle previously created by DxDrmClient_OpendrmContent()
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_NOT_SUPPORTED - Stream has no preview.
//! - DX_ERROR_CANNOT_ACQUIRE_RIGHTS - Preview rights can not be acquired for this content silently.
//! - DX_ERROR_OPERATION_FAILED - Acquisition of preview rights failed.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - The secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_StartPreview(HDxDrmContent drmContent, EDxAutoMode autoMode);

//! NOT SUPPORTED in PLAYREADY
//! Installs the rights that are embedded in the content.
//! This operation may require communication with the Rights Issuer.
//! The DRM agent will try to perform contact the Rights Issuer (if required)
//! silently (i.e. without asking the user). If the operation cannot be performed
//! silently the user will be asked for its consent unless it is not permitted by autoMode level.
//! \param[in] drmContent	A handle previously created by DxDrmClient_OpendrmContent()
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
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_InstallEmbeddedRights(HDxDrmContent drmContent, EDxAutoMode autoMode, DxBool* rightsWereInstalled);

//! Use this function to set a UI context which might be needed by the MessageBox implementation
//! (e.g. A handle to the owner window of the message box in winCE). If UI context is set, it will be passed to all DRM Objects
//! created by this module.
//! Note: Some platforms do not require this parameter to be set.
//! \param[in] drmContent	A handle previously created by DxDrmClient_OpendrmContent()
//! \param[in] uiContext	A pointer to the UI context to be set.
void DxDrmContent_SetUiContext(HDxDrmContent drmContent, void* uiContext);

//! Retrieves the UI Context previously set.
void* DxDrmContent_GetUiContext(HDxDrmContent drmContent);

//! Closes the stream handle.
DX_FUNC DRM_C_API void DxDrmContent_Close(HDxDrmContent* drmContent);

DX_FUNC DRM_C_API EDxDrmContentType DxDrmContent_GetType(HDxDrmContent drmContent);

//! Retrieves the Maximal output protection levels for the given content. this is only relevant for WMDRm contents.
//! \param[in] drmContent					A handle previously created by DxDrmClient_OpendrmContent()
//! \param[out] audioCompressedLevel		On success will return the compressed audio protection level
//! \param[out] audioUncompressedLevel		On success will return the uncompressed audio protection level
//! \param[out] videoCompressedLevel		On success will return the compressed video protection level
//! \param[out] videoUncompressedLevel		On success will return the uncompressed video protection level
//! \param[out] videoAnalogLevel			On success will return the analog video protection level
//! \return
//! - DX_SUCCESS - Operation completed successfully. 
//! - DX_ERROR_BAD_ARGUMENTS - If drmContent is null.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmContent_GetCurrentMaxProtectionLevel(HDxDrmContent drmContent, DxUint32* audioCompressedLevel, DxUint32* audioUncompressedLevel, DxUint32* videoCompressedLevel, DxUint32* videoUncompressedLevel, DxUint32* videoAnalogLevel);

//! Retrieves whether the Content license contains the Miracast Enabler object (permission to send decrypted content over HDCP 2.1 and above).
//! \param[in] drmContent					A handle previously created by DxDrmClient_OpendrmContent()
DX_FUNC DRM_C_API DxBool	   DxDrmContent_HasMiracastEnabler(HDxDrmContent drmContent);

#ifdef __cplusplus
}
#endif

#endif

