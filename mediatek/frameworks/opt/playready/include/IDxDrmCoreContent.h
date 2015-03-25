#ifndef IDX_DRM_CORE_CONTENT_H
#define IDX_DRM_CORE_CONTENT_H

#include "IDxRightsObject.h"
#include "IDxCoreImportStream.h"

//! IDxDrmCoreContent represents a generic interface for handling a DRM content. A DRM content may include several contents.
//! By default the first content is the active content. The active content may be changed
//! using SetActiveContent(). The IDxDrmCoreContent provides the following features:
//! - Extraction of content related attributes such as:  Title, Author, Description, etc. 
//! - Query about DRM status of the content (i.e. Is the content encrypted? Do rights exists?)
//! - Consumption of content rights
//! - Extraction of content's right information.
class IDxDrmCoreContent
{
public:
	virtual ~IDxDrmCoreContent() {};

	//! Allows atomic operation over one or more content APIs in a multi-threaded environment.
	//! Must be followed by a call to Unlock()
	virtual EDxDrmStatus Lock() = 0;
	
	virtual void Unlock() = 0;

	virtual const DxChar* GetFileName() = 0;

    //! returns the DRM scheme of this content.
    virtual EDxDrmScheme GetDRMScheme() = 0;

	//! returns the DRM content type (whether file or stream).
	virtual EDxDrmContentType GetDRMContentType() = 0;

    //! returns the number of contents in the DRM content.
    //! In WMDRM and OMA V1 this value will always be 1.
    //! In OMA V2 there may be multiple contents. 
    //! In case of progressive download the function will return only the number of contents
    //! whose download has started (i.e. the contents which are fully downloaded or partially downloaded.
    virtual DxUint32 GetNumOfContents() = 0;

    //! Returns the value of the specified text attribute.
    //! The returned pointer points to memory that exists until the next call to GetTextAttribute().
    //! The caller MUST NOT free this memory.
    //! If the attribute could not be found or invalid contentIndex was specified, NULL will be returned.
    //! \param[in] textAttr     Specifies the attribute to retrieve. 
    //! \param[in] contentIndex The index of the content whose attribute should be retrieved.
    //! If the value is DX_ACTIVE_CONTENT the active content will be used.
    virtual const DxChar* GetTextAttribute(EDxTextAttribute textAttr, DxUint32 contentIndex = DX_ACTIVE_CONTENT) = 0;

	//! Returns the value of the specified text attribute.
	//! The returned pointer points to memory that exists until the next call to GetTextAttribute().
	//! The caller MUST NOT free this memory.
	//! If the attribute could not be found or invalid contentIndex was specified, NULL will be returned.
	//! \param[in] attrName     The name of the attribute to retrieve. This name is scheme specific.
	//! \param[in] contentIndex The index of the content whose attribute should be retrieved.
	//! If the value is DX_ACTIVE_CONTENT the active content will be used.
	virtual const DxChar* GetTextAttributeByName(const DxChar* attrName, DxUint32 contentIndex = DX_ACTIVE_CONTENT) = 0;

    //! Retrieve set of flags that are available to the specified content.
    //! \param[in]  flagsToCheck    OR-ed combination of flags to be retrieved. 
    //!                             The OR-ed values should be taken from the EDxContentFlag enumeration.
    //! \param[out] activeFlags     On exit, Only the flags that were specified in flagsToCheck and are active will be on.
    //! \param[in]  permissions     OR-ed combination of permissions taken from the EDxPermission enumeration.
    //!                             The flags that will be retrieved will be relevant for the specified permissions.
    //! \param[in]  contentIndex    The index of the content whose attribute should be retrieved.
    //!                             If the value is DX_ACTIVE_CONTENT the active content will be used.
    //! \return
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - If the flags that were asked cannot be computed correctly becuase the secure clock is not set.
    //! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    virtual EDxDrmStatus GetFlags(DxUint32 flagsToCheck, DxUint32& activeFlags, DxUint32 permissions, DxUint32 contentIndex /*= DX_ACTIVE_CONTENT*/) = 0;

	virtual EDxDrmStatus GetSecureClock(DxTimeStruct& timeStruct) = 0;

	//! Change the active content of the DRM content to the specified content.
    //! \return
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_BAD_ARGUMENTS - If contentIndex is not a valid content index.
    virtual EDxDrmStatus SetActiveContent(DxUint32 contentIndex) = 0;

	//! Change the active content of the DRM content to the content part with the specified contentId.
	//! \return
	//! - DX_SUCCESS - On Success
	//! - DX_ERROR_BAD_ARGUMENTS - If contentIndex is Null.
	//! - DX_ERROR_ITEM_NOT_FOUND - If no content with the given contentId exists.
	virtual EDxDrmStatus SetActiveContentById(const DxChar* contentId) = 0;

    //! Returns the currently active content index (zero based)
    virtual DxUint32 GetActiveContent() = 0;

    //! Sets the purpose of usage of the active content.
    //! The appropriate RO will be used according to the intent that was set in this call.
    //! \param[in]  intentVal   The intent to be used.
    //! \return
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    //! - DX_ERROR_NO_RIGHTS - If there are no rights for the active content.
    //! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - If the rights for the active content are time based and the secure clock is not set.
    //! - DX_ERROR_DOMAIN_CONTEXT_IS_MISSING - If the content have domain rights and the device is not member of the domain.
    //! - DX_ERROR_INVALID_STATE - Is the function is called during consumption (i.e. between DX_EVENT_START & DX_EVENT_STOP)
    virtual EDxDrmStatus SetIntent(EDxIntent intentVal) = 0;

    //! Notifies the DRM agent about a consumption event. The content rights will be consumed according
    //! to the events that are notified using this call. The consumption events effects only consumable rights
    //! (i.e. Count, Timed Count, Interval & accumulated)
    //! The following table lists the available events and when there effect on rights consumption.
    //! - DX_EVENT_START    Decrements Count constraint counter by 1.
    //!                     Activates a not already active interval constraint.
    //!                     Start usage timer for Timed Count & Accumulated constraints.
    //!                     This event is valid only if consumption is not already active.
    //! - DX_EVENT_STOP     Decrements Timed Count by 1 is the time passed according to the usage timer
    //!                     exceeds the Timed Count threshold and the Timed Count was not decremented yet.
    //!                     Decrements an Accumulated constraints by the time passed according to the 
    //!                     usage timer.
    //!                     Stops the Usage Timer.
    //!                     This event is always valid.
    //! - DX_EVENT_PAUSE    Pauses the Usage Timer.
    //!                     This event is valid only if DX_EVENT_START event occurred earlier.
    //!                     Stops the Usage Timer.
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
    //! The HandleConsumptionEvent() function should called only after the intent was set using SetIntent().
    //! If the active RO is expired due to the consumption event the DRM agent will look for any other
    //! appropriate RO that is available and will continue the consumption.
    //! If the function fails, consumption is stopped automatically.
    //! \return
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    //! - DX_ERROR_NO_RIGHTS - If there are no rights for the active content.
    //! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - If the rights for the active content are time based and the secure clock is not set.
    //! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
    //! - DX_ERROR_INVALID_STATE - If the event specified is not consistent with the current consumption state
    virtual EDxDrmStatus HandleConsumptionEvent(EDxConsumptionEvent eventVal) = 0;

    //! Retrieve a pointer to the rights object that is currently being used by the DRM system for
    //! the active content. 
    //! \return 
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_NO_RIGHTS - If there are no rights for the active content.
    //! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - If the rights for the active content are time based and the secure clock is not set.
    //! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
    virtual const IDxRightsObject* GetActiveRO() = 0;

    //! Updates the cache with filename and rights.
    virtual EDxDrmStatus UpdateContentInfoInCache() = 0;

	//! returns the license challenge for the content (WMDRM Only - default implementation is not supported)
	virtual  EDxDrmStatus GetLicenseChallenge( void* challenge, DxUint32* size) {size=size; challenge=challenge ; return DX_ERROR_NOT_SUPPORTED;};

	//! process the license response arrived from the server	
	virtual	 EDxDrmStatus ProcessLicenseResponse( const void* serverResponse, DxUint32 serverResponseSize, void** serverResponseResult , DxBool* isAckRequired){serverResponse = serverResponse; serverResponseSize = serverResponseSize; serverResponseResult = serverResponseResult ; isAckRequired = isAckRequired; return DX_ERROR_NOT_SUPPORTED;};
	//! returns the Max output level for the different output types.
	virtual EDxDrmStatus GetCurrentMaxProtectionLevel(DxUint32* audioCompressedLevel, DxUint32* audioUncompressedLevel, DxUint32* videoCompressedLevel, DxUint32* videoUncompressedLevel, DxUint32* videoAnalogLevel) { audioCompressedLevel=audioCompressedLevel; audioUncompressedLevel=audioUncompressedLevel; videoCompressedLevel=videoCompressedLevel; videoUncompressedLevel=videoUncompressedLevel; videoAnalogLevel=videoAnalogLevel; return DX_ERROR_NOT_SUPPORTED;};
	
	//! Retrieves whether the Content license contains the Miracast Enabler object (permission to send decrypted content over HDCP 2.1 and above).
	virtual DxBool		 HasMiracastEnabler(){return DX_FALSE;};


#ifndef DX_ONLY_CONSUMPTION_SUPPORT
    //! Returns the total number of rights objects that are stored in the secure storage for
    //! the active content. 
    virtual DxUint32 GetNumOfROs() = 0;

    //! Returns a pointer to one of the rights objects that are stored in the secure storage for
    //! the active content. The rights object is identifies by its zero-based index. 
    //! If index is greater or equal to the value returned by GetNumOfROs() NULL will be returned.
    virtual const IDxRightsObject* GetRO(DxUint32 roIndex) = 0;

    //! Indicates if rights for the current content & intent are going to be expired.
    //! The right are not considered to be expired if the remaining count exceeds countThreshold or
    //! if the number of seconds left till rights expire exceeds timeThreshold.
    //! Only valid rights (and no future rights) of the current intent will be taken into account in this calculation.
    //! If countsLeft and timeLeft are not null, then on exit they will hold the values of the total counts and time left.
    //! \return 
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
    virtual EDxDrmStatus IsSoonToBeExpired(DxUint32 countThreshold, DxUint32 timeThreshold, DxBool& isGoingToExpire, DxUint32* countsLeft /*= DX_NULL*/, DxUint32* timeLeft /*= DX_NULL*/) = 0;

    virtual EDxDrmStatus GetRightsInformation(DxUint32* countsLeft, DxUint32* intervalTimeLeft, DxUint32* accumulatedTimeLeft, DxUint32* timeLeft, DxBool* hasUnlimitedRights) = 0;

    //! Returns the number of text attributes that exists for the specified content.
    //! If invalid contentIndex was specified, 0 will be returned.
    //! \param[in] contentIndex The index of the content whose number of attributes should be retrieved.
    //! If the value is DX_ACTIVE_CONTENT the active content will be used.
    virtual DxUint32 GetNumOfTextAttributes(DxUint32 contentIndex = DX_ACTIVE_CONTENT) = 0;

    //! Retrieves the name & value of the attribute that is places in the specified index in the
    //! attribute list of the specified content.
    //! \param[in]  attrIndex       The index of the attribute to retrieve in the attribute list.
    //! \param[out] attrName        On entry, SHOULD be NULL. On exit, points to the attribute name NULL terminated string.
    //! The returned pointer points to memory that exists until the next call to GetTextAttributeByIndex().
    //! The caller MUST NOT free this memory.
    //! \param[out] attrValue       On entry, SHOULD be NULL. On exit, points to the attribute value NULL terminated string.
    //! The returned pointer points to memory that exists until the next call to GetTextAttributeByIndex().
    //! The caller MUST NOT free this memory.
    //! \param[in]  contentIndex    The index of the content whose attribute should be retrieved.
    //! If the value is DX_ACTIVE_CONTENT the active content will be used.
    //! \return
    //! - DX_SUCCESS - if attribute is found
    //! - DX_ERROR_BAD_ARGUMENTS - if attrIndex or contentIndex are invalid.
    virtual EDxDrmStatus GetTextAttributeByIndex(DxUint32 attrIndex, const DxChar*& attrName, const DxChar*& attrValue, DxUint32 contentIndex = DX_ACTIVE_CONTENT) = 0;

    virtual DxBool IsAttributeEditable(EDxTextAttribute textAttr) = 0;

    //! Sets the value of the specified text attribute.
    //! \param[in] textAttr		Specifies the attribute to set.
    //! \param[in] attrValue	Specifies the attribute value (can be Null). 
    //! \param[in] languageCode	Specifies the language code used (represented by 3 chars). Use NULL for english code: "eng".
    //! \param[in] contentIndex The index of the content whose attribute should be set.
    //! If the value is DX_ACTIVE_CONTENT the active content will be used.
    //! \return
    //! - DX_SUCCESS - if attribute was set successfully.
    //! - DX_ERROR_NOT_SUPPORTED - if this attribute cannot be set.
    //! - DX_ERROR_BAD_ARGUMENTS - if contentIndex is invalid.
    virtual EDxDrmStatus SetTextAttribute(EDxTextAttribute textAttr, const DxChar* attrValue, const DxChar* languageCode, DxUint32 contentIndex = DX_ACTIVE_CONTENT) = 0;

    //! Deletes all the ROs of the active content from the Secure Storage.
    //! \return 
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    virtual EDxDrmStatus DeleteRights() = 0;

    //! Deletes all the expired ROs of the active content from the Secure Storage.
    //! \return 
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    virtual EDxDrmStatus DeleteObsoleteRights() = 0;

    //! Creates an import stream that will manage the rights acquisition process. 
    //! \param[out] importStream    On entry, SHOULD be NULL. 
    //!                             On exit, points to a newly allocated Import Stream object. 
    //!                             This object MUST be deleted at the end of use using the delete operator.
    //! \return
    //! - DX_SUCCESS - Rights were acquired. No further operation is required. importStream will be NULL on exit.
    //! - DX_ERROR_CANNOT_ACQUIRE_RIGHTS - Rights can not be acquired for this content.
    //! - DX_ERROR_OPERATION_CANCELED - The user canceled the rights acquisition process.
    //! - DX_ERROR_OPERATION_FAILED - The rights acquisition failed.
	//! - DX_USER_MESSAGE_IS_AVAILABLE - In order to perform this operation a message box must be presented to the user.
    //! - DX_HTTP_REQUEST_IS_AVAILABLE - In order to complete the acquisition a HTTP request must be sent to the server.
    //!     More information about the HTTP request can be retrieved by calling the GetHTTPRequest()
    //!     function of the retrieved import stream.
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    virtual EDxDrmStatus StartRightsAcquisition(IDxCoreImportStream*& importStream, EDxRightsAcquisitionMethod acqMethod = DX_RIGHT_ACQ_METHOD_NORMAL, DxUint32 contentIndex = DX_ACTIVE_CONTENT) = 0;

    //! Prepare the content for super distribution or backup.
    //! This usually involve embedding of rights in the content.
    virtual EDxDrmStatus PrepareForSuperDistribution() = 0;

    //! Creates an import stream that will manage the clock adjustment process. 
    //! The retrieved import stream MUST be destroyed at the end of use.
    //! \param[out] importStream    On entry, SHOULD be NULL. 
    //!                             On exit, points to a newly allocated Import Stream object. 
    //!                             This object MUST be deleted at the end of use using the delete operator.
    //! \return
    //! - DX_SUCCESS - Clock was adjusted. No further operation is required. importStream will be NULL on exit.
    //! - DX_ERROR_CANNOT_ADJUST_CLOCK - Clock cannot be adjusted for this content.
    //! - DX_ERROR_OPERATION_CANCELED - The user canceled the clock adjustment process.
    //! - DX_ERROR_OPERATION_FAILED - The clock adjustment failed.
	//! - DX_USER_MESSAGE_IS_AVAILABLE - In order to perform this operation a message box must be presented to the user.
    //! - DX_HTTP_REQUEST_IS_AVAILABLE - In order to complete clock adjustment a HTTP request must be sent to the server.
    //!     More information about the HTTP request can be retrieved by calling the GetHTTPRequest()
    //!     function of the retrieved import stream.
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    virtual EDxDrmStatus AdjustClock(IDxCoreImportStream*& importStream, EDxAutoMode autoMode) = 0;

    virtual EDxDrmStatus InstallEmbeddedRights(IDxCoreImportStream*& importStream, EDxAutoMode autoMode, DxBool* rightsWereInstalled) = 0;

    // Shai: Why this function has different signatures than InstallEmbeddedRights, AdjustClock, etc.
	virtual IDxCoreImportStream* GetOnExpiredRightsImportStream() = 0;

	//! Prepares the content for an iteration over it's ROs. It is mandatory to call this function before calling GetRO().
	//! It sets the number of available ROs in the numOfROs parameter.
	//! \param[in]  iterationMode  Can be one of the following values:
	//!                            - DX_RO_ITERATION_DISPLAY: Prepares iteration only over the ROs which aren't expired.
	//!                                Also, if similar ROs are found, their information is combined for preview purposes.
	//!                            - DX_RO_ITERATION_FULL_INFO: Prepares iteration over all rights objects that are stored
	//!                                in the secure storage for the active content.
	//! \param[out] numOfROs	   On success will hold the number of ROs available for iteration. 
	//! \return 
	//! - DX_SUCCESS - Operation succeeded.
	//! - DX_ERROR_BAD_ARGUMENTS - If drmFile or activeRo are null, or an invalid iterationMode given.
	virtual EDxDrmStatus PrepareForROIteration(EDxROIterationMode iterationMode, DxUint32* numOfROs) = 0;

	//! Provides the IDxDrmCoreContent with the user response to the last consent message.
	//! \return
	//! - DX_DRM_SUCCESS - The operation was successful.
	//! - DX_ERROR_INVALID_STATE - If called not when user consent was not needed.
	//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
	//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
	virtual EDxDrmStatus HandleUserResponse(EDxUserResponse userResponse) = 0;

	//! Creates an import stream that will manage the SetIntent process.
	//! The retrieved import stream MUST be destroyed at the end of use.
	//! The flow of the SetIntent process depends on the content flags and user response, but the general flow is as follows:
	//! - Try to set intent to see if any rights are installed. This call is able to handle errors: 
	//! 	DX_ERROR_SECURE_CLOCK_IS_NOT_SET and DX_ERROR_DOMAIN_CONTEXT_IS_MISSING if the autoMode enables it.
	//! - If there are pending rights, show message and return DX_ERROR_RIGHTS_PENDING.
	//! - Try installing rights embedded in the content.
	//! - Request for OnExpiredUrl - if content has expired rights with "OnExpiredUrl" element in them, than import that url (mandatory).
	//! 	This url MIGHT trigger an RO acquisition, so we try to setIntent after the import.
	//! - Try to silent acquire rights (if flags permit).
	//! - Try to preview content if possible.
	//! - Acquire/renew rights for content.
	//! The appropriate RO will be used according to the intent that was set in this call.
	//! \param[out] importStream    On entry, SHOULD be NULL. 
	//!                             On exit, points to a newly allocated Import Stream object. 
	//!                             This object MUST be deleted at the end of use using the delete operator.
	//! \param[in]  intentVal   The intent to be used.
	//! \return
	//! - DX_SUCCESS - On Success
	//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
	//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
	//! - DX_ERROR_NO_RIGHTS - If there are no rights for the active content.
	//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - If the rights for the active content are time based and the secure clock is not set.
	//! - DX_ERROR_DOMAIN_CONTEXT_IS_MISSING - If the content have domain rights and the device is not member of the domain.
	//! - DX_ERROR_INVALID_STATE - Is the function is called during consumption (i.e. between DX_EVENT_START & DX_EVENT_STOP)
	virtual EDxDrmStatus StartSetIntent(IDxCoreImportStream*& importStream, EDxIntent intentVal, EDxAutoMode autoMode) = 0;
#endif
};

#endif

