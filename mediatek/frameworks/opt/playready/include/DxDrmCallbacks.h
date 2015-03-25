#ifndef DX_DRM_CALLBACKS_H
#define DX_DRM_CALLBACKS_H

#include "DxDrmDefines.h"
#ifdef __cplusplus
extern "C" {
#endif

//! Retrieve a destination file full path based on the received information.
//! Function must verify that existing file with the same name does not exist. If such a file exists it will be overwritten.
//! The function may display UI to the user in order to determine file name.
//! The function must verify that the retrieved file name contains only characters that are legal for file names.
//! If the file path points to a directory that does not currently exists, this directory will be created.
//! \param[in]  importStream        An Import-Stream handle of the content being downloaded. Download information can be
//!									obtained using DxImportStream_GetTextAttribute() and DxImportStream_GetDataItem().
//! \param[in]  uiContext           Pointer to context provided by application. This information can be used in order to 
//!                                 display UI to the user (e.g. Parent HWND in Windows OS).
//! \param[in]  contentMimeType     MimeType of the encrypted cotnent (e.g. image/jpeg, audio/mp3).
//! \param[in]  fileMimeType        MimeType of the DRM container (e.g. application/vnd.oma.drm.content, application/vnd.oma.drm.dcf).
//! \param[in]  suggestedFileName   The suggested file name based on response HTTP Headers, Content Title, Download URL, etc.
//!                                 Suggested file name does not include path or extension.
//! \param[in]  classificationInfo  Classification of the content based on HTTP headers or Classification box. (e.g. ringtone, music).
//! \param[out] destFileName        On entry, empty allocated buffer. On exit, holds the selected filename UTF8 encoded.
//! \param[in]  destFileNameSize    The size in bytes of fileNameBuff buffer. Function should not write beyond buffer size.
//! \return
//! True if filename was retrieved successfully.
//! False if operation failed or user canceled.
typedef DxBool (*DxGetDestinationFileNameFunc)(void* importStream, void* uiContext,
                                                const DxChar* contentMimeType, const DxChar* fileMimeType,
                                                const DxChar* suggestedFileName, const DxChar* classificationInfo,
                                                DxChar* destFileName, DxUint32 destFileNameSize );

//! Registers the callback and returns the callback that was registered before (may be NULL).
//! The return value can be useful if the application wish to restore the state before registration.
DxGetDestinationFileNameFunc DxRegisterGetDestinationFileNameCallback(DxGetDestinationFileNameFunc func);

//! Displays a message box to the user with the specified title and message.
//! The function should return after the message is closed by the user and retrieve the user response.
//! \param[in]  uiContext           Pointer to context provided by application. This information can be used in order to 
//!                                 display UI to the user (e.g. Parent HWND in Windows OS).
//! \param[in]  msgCode		        Message code of the message to display. This info enables the function to behave differently
//!                                 based on the message code (e.g. Filter some of the messages)
//! \param[in]  msgText		        Text of the message.
//! \param[in]  msgTitle		    Title of the message.
//! \param[in]  msgType			    Inidicates what are the buttons that should be displayed in the message.
//! \param[in]  msgSeverity			This items can be used in order to display icon inside the message box.
//! \param[out] userResponse		On exit, holds the user response.
//!									This value may be NULL on entry. This means that the application is not interested in user response.
//!                                 In this case the function may choose to return immidiately without getting the actual user response.
typedef void (*DxMessageBoxFunc)(void* uiContext, EDxMessageCode msgCode,
                                 const DxChar* msgText, const DxChar* msgTitle,
                                 EDxMessageType msgType, EDxMessageSeverity msgSeverity,
                                 EDxUserResponse* userResponse);

//! Registers the callback and returns the callback that was registered before (may be NULL).
//! The return value can be useful if the application wish to restore the state before registration.
DX_FUNC DRM_C_API DxMessageBoxFunc DxRegisterMessageBoxCallback(DxMessageBoxFunc func);

//! Open given URL using the default web browser.
//! \param[in]  uiContext           Pointer to context provided by application. This information can be used in order to 
//!                                 display UI to the user (e.g. Parent HWND in Windows OS).
//! \param[in]  browserUrl          UTF8 encoded URL to open.
//! \return
//! True if web browser was opened successfully.
//! False if web browser could not be openned.
typedef DxBool (*DxOpenWebBrowserFunc)(void* uiContext, const DxChar* browserUrl);

//! Registers the callback and returns the callback that was registered before (may be NULL).
//! The return value can be useful if the application wish to restore the state before registration.
DxOpenWebBrowserFunc DxRegisterOpenWebBrowserCallback(DxOpenWebBrowserFunc func);

//! Launch the given file using an appropriate program.
//! \param[in]  uiContext           Pointer to context provided by application. This information can be used in order to 
//!                                 display UI to the user (e.g. Parent HWND in Windows OS).
//! \param[in]  fileName          filename to open.
//! \return
//! True if file was launched successfully.
//! False if file could not be launched.
typedef DxBool (*DxLaunchFileFunc)(void* uiContext, const DxChar* fileName);

//! Registers the callback and returns the callback that was registered before (may be NULL).
//! The return value can be useful if the application wish to restore the state before registration.
DxLaunchFileFunc DxRegisterLaunchFileCallback(DxLaunchFileFunc func);

//! The entire import process can be divided to 3 phases:
//! - PreProcess – From Download start until enough information was collected in order to select file name (e.g. DD Processing)
//! - Download – Download of the content itself.
//! - PostProcess – Operations that are required after download completed (e.g. Installation of embedded rights, DD Install Notification, waiting for OMA V1 rights)
//! 
//! All downloads have the PreProcess phase however not all of them have the Download phase (e.g. Rights download) or the PostProcess phase (e.g. FL download)
//! In order to minimize integration effort, Discretix takes full responsibility for the complex import process.
//! To allow the application to control the UI the DRM agent call callbacks during import process in order to notify 
//! the application about download details and progress. The application is responsible to display this information to 
//! the user. This enum list all callbacks and when they are called.
typedef enum
{
    //! Called periodically during PreProcess phase.
    DX_CALLBACK_START_PRE_PROCESS,
    //! Called when PreProcess phase ends.
    DX_CALLBACK_END_PRE_PROCESS,

    //! Called when content download starts
    DX_CALLBACK_START_DOWNLOAD,
    //! Called periodically during content download to update download progress.
    DX_CALLBACK_UPDATE_DOWNLOAD,
    //! Called when download phase ends.
    DX_CALLBACK_END_DOWNLOAD,

    //! Called periodically during PostProcess phase.
    DX_CALLBACK_START_POST_PROCESS,
    //! Called periodically when PostProcess phase ends.
    DX_CALLBACK_END_POST_PROCESS,
    
    //! Called in all cases when the entire import process is finished.
    DX_CALLBACK_END_IMPORT,
    DX_NUM_OF_DOWNLOAD_CALLBACKS
} EDxDownloadCallbackId;

typedef enum
{
    DX_IMPORT_STATUS_PRE_PROCESS,
    DX_IMPORT_STATUS_DOWNLOAD_IN_PROGRESS,
    DX_IMPORT_STATUS_DOWNLOAD_COMPLETED,
    DX_IMPORT_STATUS_FAILED,
    DX_IMPORT_STATUS_CANCELED,
    DX_IMPORT_STATUS_FINISHED,
} EDxImportStatus;

//! This structure contains information regarding active import process. 
//! This structure is passed to all Download Progress callbacks and should 
//! be used in order to reflect download progress and status to the user.
typedef struct 
{
    //! Indicates the type of operation being done (e.g. Downloading, Registering Device)
    //! This data member is available during the entire import process
    DxChar*				m_OperationName;
    //! The destination file path to which the content is downloaded.
    //! This data member is available during download and post process phase (not available during pre-process phase).
    DxChar*				m_FileName;
    //! Mime Type of the downloaded content. (e.g. image/jpeg, audio/mp3)
    //! This data member is available during download and post process phase (not available during pre-process phase).
    DxChar*				m_MimeType;
    //! Total size of downloaded content in bytes.
    //! This data member is available during download and post process phase (not available during pre-process phase).
    DxUint32			m_FileSizeInBytes;
    //! Number of bytes that were already downloaded out of the total number of bytes specified by m_FileSizeInBytes.
    //! This data member is available during download and post process phase (not available during pre-process phase).
    DxUint32			m_BytesDownloaded;
    //! Indicates the current import status.
    //! This data member is available during the entire import process
    EDxImportStatus		m_ImportStatus;
	//! Indicates if progressive download feature is available. If this flag is true, content may be played during download.
	//! This data member is available during download and post process phase (not available during pre-process phase).
	DxBool				m_CanPlay;
    //! This flag should be set if the file has been launched by the callback function.
    //! This affects the UIs that appear at the end of the download, therefore it is important the flag has the proper value.
	DxBool				m_WasFilePlayed;
	//! Pointer that can be assigned by the callback in order to keep state information between callbacks.
	//! Data that is assigned in this member in one callback can be accessed by all later callbacks.
	//! It is callbacks responsibility to release this data member (if required) when import process is finished.
    void*				m_DownloadHandle;
    //! Pointer to context provided by application. This information can be used in order to 
    //! display UI to the user (e.g. Parent HWND in Windows OS).
    void*               m_UiContext;
} DxImportContext;

//! This signature is identical for all Download progress functions.
//! These functions receives importContext with relevant download details 
//! and they should use these details in order to reflect download progress
//! and status to the user.
//! \return
//! True if download should continue.
//! False if user canceled the download and download should be canceled.
typedef DxBool (*DxDownloadProgressFunc)(DxImportContext* importContext);

//! Registers the callback for the specified id and returns the callback that was registered before (may be NULL).
//! The return value can be useful if the application wish to restore the state before registration.
DX_FUNC DRM_C_API DxDownloadProgressFunc DxRegisterDownloadProgressCallback(EDxDownloadCallbackId id, DxDownloadProgressFunc func);

//! The download process may require issuing HTTP request.
//! The HTTP callbacks are used to inform the application when data connection
//! should be opened and when it should be closed.
typedef enum
{
    //! Called before first HTTP request in the download process.
    //! The callback should verify if data connection is open. 
    //! If data connection is not open the callback should open a data 
    //! connection. The callback should return information regarding the 
    //! IP address of the information that should be used and about HTTP
    //! proxy server if available. The callback may display UI to the user
    //! in order to decide which type of Data connection should be opened.
    //! Callback should return true is connection is open or false if
    //! connection could not be opened.
    DX_HTTP_CALLBACK_OPEN,
    //! Called during HTTP activity (after the connection was opened) 
    //! in order to verify that connection is still active. The information
    //! in the DxHttpConnction context is identical to the the information 
    //! that was set in the DX_HTTP_CALLBACK_OPEN callback.
    //! If the connection is active the callback should return true.
    //! If the connection is not active the callback should return false.
    //! In this case the DRM agent may call the DX_HTTP_CALLBACK_OPEN callback
    //! again in order to open a new connection.
    DX_HTTP_CALLBACK_IS_ACTIVE,
    //! Called after last HTTP communication of the download process.
    //! This callback may close the connection that is specified in the 
    //! DxHttpConnction context.
    DX_HTTP_CALLBACK_CLOSE,
    DX_NUM_OF_HTTP_CALLBACKS
} EDxHttpCallbackId;

//! This struct holds connection information.
//! This context is passed to all HTTP callbacks.
typedef struct DxHttpConnection_t
{
    //! IP address of the interface that will be used for the connection
    DxUint32 m_InterfaceIpAddress;
    //! HTTP Proxy server IP address. If proxy does not exist this value should be set to 0.
    DxUint32 m_ProxyIpAddress;
    //! HTTP Proxy server port number. If proxy does not exist this value should be set to 0.
    DxUint32 m_ProxyPort;
    //! Pointer to context provided by application. This information can be used in order to 
    //! display UI to the user (e.g. Parent HWND in Windows OS).
    void*    m_UiContext;
} DxHttpConnection;

typedef DxBool (*DxHttpConnectionFunc)(DxHttpConnection* importContext);

DxHttpConnectionFunc DxRegisterHttpCallback(EDxHttpCallbackId id, DxHttpConnectionFunc func);



#ifdef __cplusplus
}
#endif

#endif