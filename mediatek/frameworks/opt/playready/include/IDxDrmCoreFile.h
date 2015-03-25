#ifndef IDX_DRM_CORE_FILE_H
#define IDX_DRM_CORE_FILE_H

#include "IDxDrmCoreContent.h"

typedef DxUint32 DxNotificationId;

//! IDxDrmCoreFile represents an opened DRM file. A DRM file may include several contents.
//! By default the first content is the active content. The active content may be changed
//! using SetActiveContent(). The IDxDrmCoreFile provides the following features:
//! - Extraction of content related attributes such as:  Title, Author, Description, etc. 
//! - Query about DRM status of the content (i.e. Is the content encrypted? Do rights exists?)
//! - Decryption of file content (i.e. reading decrypted data as if the file is not encrypted)
//! - Consumption of content rights
//! - Extraction of content's right information.
class IDxDrmCoreFile : public IDxDrmCoreContent
{
public:
    IDxDrmCoreFile();
    virtual ~IDxDrmCoreFile();

	virtual EDxDrmContentType GetDRMContentType()	{return DX_CONTENT_TYPE_DRM_FILE;	}

    //! Retrieves the size in byte of the active content. 
    //! If SetIntent was not called successfully the retrieved value may include padding that is
    //! required for encryption.
    //! If SetIntent was called successfully the retrieved value will be of the plain-text version
    //! of the content.
    //! \return
    //! - DX_SUCCESS - On Success. In this case contentSize will contain the content size of the plain-text
    //!     version (i.e. after padding was removed.
    //! - DX_ERROR_FILE_IS_BEING_DOWNLOADED - File is being downloaded and the content size is not known at this time.
    //!     In this case the contentSize will include the size that was downloaded so far (may include padding).
    //! - DX_ERROR_FILE_SIZE_INCLUDES_PADDING - If file is encrypted and padding size is unknown.
    //!     In this case the contentSize will include the size of the content including the padding.
    virtual EDxDrmStatus GetContentSize(DxUint32& contentSize) = 0;

    virtual EDxDrmStatus GetFullContentSize(DxUint32& contentSize) = 0;

    //! Reads data from the active content.
    //! The Read() function should be called only after the intent was set using SetIntent().
    //! If the intent was set to DX_INTENT_PEEK and the active content has no rights the function will 
    //! retrieve the content data without decryption.
    //! If the intent was not set to DX_INTENT_PEEK and the active content has no rights the function will
    //! fail (DX_ERROR_NO_RIGHTS).
    //! If Time or Interval constraint have expired the function will look for other valid Rights Object.
    //! If no alternative Rights object is found the function will fail.
    //! \param[out] data                a pointer to an empty buffer that will be filled with retrieved data.
    //! \param[in]  dataSize            The size of the empty buffer in bytes. Number of bytes to read from file.
    //! \param[out] dataActuallyRead    On entry, points to an empty DxUint32 variable. MAY be NULL.
    //!     On exit, if not NULL, the number of bytes that were actually read from the file.
    //!     If dataActuallyRead is NULL and not all bytes could be read DX_ERROR_NOT_AVAILABLE or 
    //!     DX_ERROR_FILE_IS_BEING_DOWNLOADED will be returned.
    //! \return
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    //! - DX_ERROR_NO_RIGHTS - If there are no rights for the active content.
    //! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - If the rights for the active content are time based and the secure clock is not set.
    //! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
    //! - DX_ERROR_FILE_ACCESS_ERROR - If read from file operation failed.
    //! - DX_ERROR_NOT_AVAILABLE - If not all the files could be read and file is not being downloaded.
    //! - DX_ERROR_FILE_IS_BEING_DOWNLOADED - If not all the files could be read and file is being downloaded.
    virtual EDxDrmStatus Read(void* data, DxUint32 dataSize, DxUint32* dataActuallyRead) = 0;

    //! Moves the file pointer by the specified offset relative to the point specified by origin.
    //! The origin parameter can have one of the following values:
    //! - DX_CONTENT_SEEK_SET - offset is relative to active content beginning. offset must be positive.
    //! - DX_CONTENT_SEEK_CUR - offset is relative to current position in active content.
    //! - DX_CONTENT_SEEK_END - offset is relative to active content end. offset must be negative.
    //! \note - The function will not allow seeking to a position outside the active content boundaries.
    //! \note - The seek origin is relative to the active content beginning and end and not necessarily to the
    //! container file beginning and end.
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
    //! - DX_ERROR_NOT_AVAILABLE - If the offset could not be reached and the file is not being downloaded.
    //! - DX_ERROR_FILE_IS_BEING_DOWNLOADED - If the offset could not be reached and the file is being downloaded.
    virtual EDxDrmStatus Seek(DxInt32 offset, EDxContentSeek origin) = 0;

    //! Queries if end of active content has been reached. In case the file is being downloaded 
    //! (i.e. Progressive Download) the function will indicate end of content only when the 
    //! active content (not necessarily the entire container) was fully downloaded and its end
    //! was reached.
    //! \return
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
    virtual DxBool IsEof() = 0;

    //! Retrieve the current position in the active content. The returned value is relative to the
    //! active content beginning (and not necessarily to the container file beginning).
    //! \return
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
    virtual DxUint32 GetPos() = 0;

#ifndef DX_NO_LARGE_FILE_SUPPORT

	//! Retrieves the size in byte of the active content. 
	//! If SetIntent was not called successfully the retrieved value may include padding that is
	//! required for encryption.
	//! If SetIntent was called successfully the retrieved value will be of the plain-text version
	//! of the content.
	//! \return
	//! - DX_SUCCESS - On Success. In this case contentSize will contain the content size of the plain-text
	//!     version (i.e. after padding was removed.
	//! - DX_ERROR_FILE_IS_BEING_DOWNLOADED - File is being downloaded and the content size is not known at this time.
	//!     In this case the contentSize will include the size that was downloaded so far (may include padding).
	//! - DX_ERROR_FILE_SIZE_INCLUDES_PADDING - If file is encrypted and padding size is unknown.
	//!     In this case the contentSize will include the size of the content including the padding.
	virtual EDxDrmStatus GetContentSize64(DxUint64& contentSize) = 0;

	virtual EDxDrmStatus GetFullContentSize64(DxUint64& contentSize) = 0;

	//! Moves the file pointer by the specified offset relative to the point specified by origin.
	//! The origin parameter can have one of the following values:
	//! - DX_CONTENT_SEEK_SET - offset is relative to active content beginning. offset must be positive.
	//! - DX_CONTENT_SEEK_CUR - offset is relative to current position in active content.
	//! - DX_CONTENT_SEEK_END - offset is relative to active content end. offset must be negative.
	//! \note - The function will not allow seeking to a position outside the active content boundaries.
	//! \note - The seek origin is relative to the active content beginning and end and not necessarily to the
	//! container file beginning and end.
	//! - DX_SUCCESS - On Success
	//! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
	//! - DX_ERROR_NOT_AVAILABLE - If the offset could not be reached and the file is not being downloaded.
	//! - DX_ERROR_FILE_IS_BEING_DOWNLOADED - If the offset could not be reached and the file is being downloaded.
	virtual EDxDrmStatus Seek64(DxInt64 offset, EDxContentSeek origin) = 0;

	//! Retrieve the current position in the active content. The returned value is relative to the
	//! active content beginning (and not necessarily to the container file beginning).
	//! \return
	//! - DX_SUCCESS - On Success
	//! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
	virtual DxUint64 GetPos64() = 0;

#endif

#ifndef DX_ONLY_CONSUMPTION_SUPPORT

    //! Adds notification which are related to a content in the IDxDrmCoreFile container.
    //! \param[out] notificationId  On exit an identifier that uniquely identifies the notification
    //!                             in the IDxDrmCoreFile context. This identifier can be used when calling
    //!                             WaitForNotification() and RemoveNotification().
    //! \param[in]  notificationType indicates if notification is required on rights arrival or rights expiry.
    //! \param[in]  requiredRights  The rights that the notification relates to.
    //! \param[in]  contentIndex    The index of the content that the notification relates to.
    //! If DX_ACTIVE_CONTENT is used the required notification is related to
    //! the active content.
    //! \note A notification that was added by this function SHOULD be removed explicitly by RemoveNotification().
    //! If it is not removed by a call to RemoveNotification() it will be removed automatically when the 
    //! IDrmFile object is destructed.
    //! \return
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    //virtual EDxDrmStatus AddNotification(DxNotificationId& notificationId, EDxNotificationType notificationType,
    //    EDxPermission requiredRights, DxUint32 contentIndex = DX_ACTIVE_CONTENT) = 0;

    //! Waits until notification identified by notificationId occurs or until timeout expired
    //! (timeout is given in milliseconds).
    //! If the notification has already occurred before calling the function the function will
    //! return immediately with DX_SUCCESS.
    //! \return
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_TIMEOUT_EXPIRED - If timeout expired before notification occurred.
    //! - DX_ERROR_NOT_AVAILABLE - The notificationId cannot be found in the server
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    //! 
    //virtual EDxDrmStatus WaitForNotification(DxNotificationId notificationId, DxUint32 timeout) = 0;

    //! Removes a notification that was previously added by AddNotification().
    //! \return
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_NOT_AVAILABLE - If notificationId refers to a notification that was not previously added
    //! or already removed.
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    //virtual EDxDrmStatus RemoveNotification(DxNotificationId notificationId) = 0;
#endif
};

#endif

