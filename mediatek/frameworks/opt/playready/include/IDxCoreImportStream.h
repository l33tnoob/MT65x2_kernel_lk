#ifndef IDX_CORE_IMPORT_STREAM_H
#define IDX_CORE_IMPORT_STREAM_H

#include "IDxHttpRequest.h"
#include "IDxUserMessage.h"

enum EDxOperationType
{
    DX_OP_TYPE_UNKNOWN,
    DX_OP_TYPE_REGISTRATION,
    DX_OP_TYPE_JOIN_DOMAIN,
    DX_OP_TYPE_LEAVE_DOMAIN,
	DX_OP_TYPE_RO_ACQUISITION,
	DX_OP_TYPE_RO_UPLOAD,
    DX_OP_TYPE_EMBEDDED_RO_INSTALLATION,
    DX_OP_TYPE_1_PASS_RO_INSTALLATION,
	DX_OP_TYPE_IDENTIFICATION,
	DX_OP_TYPE_METERING,
	DX_OP_TYPE_CONTENT_DOWNLOAD,
	DX_OP_TYPE_CONTENT_UNLOCK,
	DX_OP_TYPE_CONTENT_PREVIEW,
};

//TODO: fix. error codes have changed
//!  IDxCoreImportStream is used to introduce new content or rights to the DRM system.
//! The IDxCoreImportStream interface is most useful when the information is downloaded
//! from the network (usually using HTTP). In order to create instance of IDxCoreImportStream
//! the IDxDrmCoreClient::CreateImportStream() should be called with the information's mime
//! type (which can be retrieved from the Content-Type HTTP header).
//! After the import stream object was created all the HTTP headers can be provided 
//! to the DRM system by calling AddHTTPHeaders() or AddHTTPHeader() (the Content-Type
//! header can be provided too). Providing of HTTP headers is optional.   
//! The information itself (HTTP body in case of HTTP) should be provided to the DRM system 
//! by calling AddBuffer(). The information can be provided in chunks by sequence of calls
//! to AddBuffer() with consecutive chunks of information. 
//! When the information is finished, Finish() should be called. If the information stream is 
//! interrupted, Cancel() should be called. 
//! If the information should be stored in a file, the destination file name should be
//! provided using SetDestinationFile(). If a destination file is required but was not provided,
//! AddBuffer() or Finish() will return the DX_ERROR_DEST_FILE_NAME_REQUIRED error code.
//! In this case a destination file name must be set using SetDestinationFile() and 
//! afterwards you should retry the operation that failed.
//! In order to complete the processing of the imported content (i.e. When calling Finish()
//! a web browser should be opened. In this case the DX_WEB_BROWSER_REQUIRED 
//! error code will be returned. When this error is returned the url to open in the web browser
//! can be retrieved using GetBrowserUrl().
//! In order to complete the processing of the imported content (i.e. when calling Finish()) 
//! a user consent may be required. In this case the DX_USER_CONSENT_REQUIRED 
//! error code will be returned. When this error is returned the question that should be
//! presented to the user can be retrieved using GetUserMessage().
//! The user response should be provided to the import stream using HandleUserResponse().
//! In order to complete the processing of the imported content (i.e. When calling Finish()
//! or HandleUserResponse()) a HTTP request should be sent back to the server.
//! In this case the DX_HTTP_REQUEST_IS_AVAILABLE error code will be returned. 
//! When this error is returned the rquest that should be sent to the user can be 
//! retrieved using GetHTTPRequest(). The retrieved request should be sent to the server.
//! The server response should be supplied to the same IDxCoreImportStream object. The response
//! headers should be supplied using AddHTTPHeaders() or AddHTTPHeader(). The response body 
//! should be supplied using a sequence of calls to AddBuffer() that will be completed by 
//! a call to Finish(). 
//! Please note that this additional call to Finish() may also result in another 
//! DX_WEB_BROWSER_REQUIRED, DX_USER_CONSENT_REQUIRED or DX_HTTP_REQUEST_IS_AVAILABLE error code.
//! The process of getting user consent and sending requests to the server may continue until
//! Finish() returns DX_DRM_SUCCESS.
//! When the import is over the object should be deleted.
//! 
class IDxCoreImportStream
{
public:
    IDxCoreImportStream();
    virtual ~IDxCoreImportStream();

    virtual EDxOperationType GetOperationType() = 0;
    
	//! Returns the value of the specified text attribute.
    //! The returned pointer points to memory that exists until the next call to GetTextAttribute().
    //! The caller MUST NOT free this memory.
    //! If the attribute could not be found or invalid contentIndex was specified, NULL will be returned.
    //! \param[in] textAttr     Specifies the attribute to retrieve. 
    virtual const DxChar* GetTextAttribute(EDxTextAttribute textAttr) = 0;

	//! Returns the value of the specified text attribute.
	//! The returned pointer points to memory that exists until the next call to GetTextAttribute().
	//! The caller MUST NOT free this memory.
	//! If the attribute could not be found or invalid contentIndex was specified, NULL will be returned.
	//! \param[in] textAttr     Specifies the name of attribute to retrieve. 
	virtual const DxChar* GetTextAttributeByName(const DxChar* attrName) = 0;

    //! Sets the destination file name for the imported content.	
    virtual EDxDrmStatus SetDestinationFile(const DxChar* destFileName) = 0;

    //! Introduces HTTP headers that arrived with the information that is being imported.
    //! \param[in] httpHeaders	a string that includes lines of HTTP header.
    //! every line has the following syntax: "Header-Name: HeaderValue"
    //! (This is the transmission format of the HTTP headers).
    virtual EDxDrmStatus AddHTTPHeaders(const DxChar* httpHeaders) = 0;

    //! Introduces a single HTTP header.
    //! \param[in] headerName	The header name (i.e. "Content-Type")
    //! \param[in] headerValue	The header value (i.e. "image/jpeg")
    virtual EDxDrmStatus AddHTTPHeader(const DxChar* headerName, const DxChar* headerValue) = 0;

    //! Add new chunk of information. On exit the bytesProcessed parameter indicate how many bytes
    //! of the input buffer was processed. This is important especially if DX_ERROR_DEST_FILE_NAME_REQUIRED
    //! is returned. In this case you need to call AddBuffer() again after calling SetDestinationFile()
    //! with the remaining of the buffer that was not processed.
    //! 
    //! \return
    //! - DX_ERROR_INVALID_STATE - If the function was called in invalid state (i.e. after Finish() 
    //!     terminated successfully or after a call to Cancel())
    //! - DX_ERROR_INVALID_FORMAT - If an error was found in the format of the imported information.
    //! - DX_ERROR_DEST_FILE_NAME_REQUIRED - If the imported information should be stored in a file
    //!     and a destination file was not set using SetDestinationFile().
    //! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    //! 
    virtual EDxDrmStatus AddBuffer(const void* data, DxUint32 dataSize, DxUint32* bytesProcessed) = 0;

    //! Cancels the operation. All operations that were done during the import should be reversed.
    //! The Cancel() function may be called also after Finish() was called.
    //! terminated successfully)
    //! 
    virtual EDxDrmStatus Cancel() = 0;

    //! Signals the import stream that all the imported information was supplied and the information
    //! processing should be finalized.
    //! \return
    //! - DX_DRM_SUCCESS - The information was imported successfully.
    //! - DX_ERROR_INVALID_FORMAT - If an error was found in the format of the imported information.
    //! - DX_ERROR_INVALID_STATE - If the function was called in invalid state (i.e. after Finish() 
    //!     terminated successfully or after a call to Cancel())
    //! - DX_ERROR_DEST_FILE_NAME_REQUIRED - If the imported information should be stored in a file
    //!     and a destination file was not set using SetDestinationFile().
    //! - DX_USER_MESSAGE_IS_AVAILABLE - In order to process the imported information
    //!     a message box must be presented to the user.
    //! - DX_HTTP_REQUEST_IS_AVAILABLE - In order to process the imported information
    //!     a HTTP request must be sent to the server.
    //! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    virtual EDxDrmStatus Finish() = 0;

    //! Retrieve an object that contains the HTTP request information that should be sent to the server.
    //! The object is available only after Finish() returned DX_HTTP_REQUEST_IS_AVAILABLE.
    //! The object is destroyed by the IDxCoreImportStream on the next call to AddBuffer() (i.e. when server
    //! response is supplied). You should NOT delete this object.
    //! \return
    //! - DX_SUCCESS - On Success.
    //! - DX_ERROR_NOT_AVAILABLE - If called not after Finish() returned DX_HTTP_REQUEST_IS_AVAILABLE.
    //! 
    virtual EDxDrmStatus GetHTTPRequest(const IDxHttpRequest*& httpRequest) = 0;

    //! Retrieve the message that should be displayed to the user in order to get its consent.
    //! The object is destroyed by the IDxCoreImportStream on the next call to HandleUserResponse(). 
    //! You should NOT delete this object.
    //! \return
    //! - DX_SUCCESS - On Success.
    //! - DX_ERROR_NOT_AVAILABLE - If called when no message is available.
    //! 
    virtual const IDxUserMessage* GetUserMessage() = 0;

	//! Provides the IDxCoreImportStream with the HTTP result to the last HTTP request sent.
	//! \param[in]  httpResult		The HTTP result received from server
	//! \param[out] endImportFlag	An indication whether the import process should be ended
	//!								If true, the adaptation should not provide headers and body and terminate gracefully.
	//!								Note: This does NOT mean that the import has failed or that should be cancelled.
	//! \return
	//! - DX_DRM_SUCCESS - The information was imported successfully.
	//! - DX_ERROR_INVALID_STATE - If called not after Finish() returned DX_HTTP_REQUEST_IS_AVAILABLE.
	//! - DX_HTTP_REQUEST_IS_AVAILABLE - In order to process the imported information
	//! a HTTP request must be sent to the server.
	//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
	//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
	virtual EDxDrmStatus HandleHTTPResult(DxUint32 httpResult, DxBool* endImportFlag) = 0;

    //! Provides the IDxCoreImportStream with the user response to the last consent message.
    //! \return
    //! - DX_DRM_SUCCESS - The information was imported successfully.
    //! - DX_ERROR_INVALID_STATE - If called when no message was available.
    //! - DX_HTTP_REQUEST_IS_AVAILABLE - In order to process the imported information
    //! a HTTP request must be sent to the server.
    //! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    virtual EDxDrmStatus HandleUserResponse(EDxUserResponse userResponse) = 0;

	//! Enables the IDxCoreImportStream to handle certain errors.
	//! \return
	//! - DX_DRM_SUCCESS - The error was handled successfully.
	//! - DX_HTTP_REQUEST_IS_AVAILABLE - In order to process the imported information
	//! a HTTP request must be sent to the server.
	//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
	//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
	virtual EDxDrmStatus HandleError(EDxDrmStatus error) = 0;

    //! \return the URL that should be opened in the web browser when HasBrowserUrl() returns true.
    //! The returned pointer points to memory that exists as long as the IDxCoreImportStream object exists.
    //! The caller MUST NOT free this memory.
    virtual const DxChar* GetBrowserUrl() = 0;

	virtual DxBool HasUserNotification() = 0;

	virtual DxBool HasBrowserUrl() = 0;

    virtual DxUint32 GetExpectedSize() = 0;

    virtual DxUint32 GetProgress() = 0;

	//! \return Provides an indication whether the import was successful or not.
	//! Note: If called while import is in process it will return false.
	virtual DxBool IsSuccessful() = 0;

	virtual EDxDrmStatus SetDataItem(const DxChar* itemName, const DxChar* itemValue) = 0;

	virtual const DxChar* GetDataItem(const DxChar* itemName) = 0;
};

#endif

