#ifndef DX_IMPORT_STREAM_H
#define DX_IMPORT_STREAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "DxDrmDefines.h"


//! \file DxImportStream.h
//! \brief DxImportStream is used to introduce new DRM objects to the DRM system.
//!
//! The DxImportStream interface is most useful when the information is downloaded
//! from the network (usually using HTTP). In order to create an instance of DxImportStream
//! DxDrmClient_CreateImportStream() should be called with the DRM object's MIME
//! type (which can be retrieved from the Content-Type HTTP header).
//! After the import stream object was created all the HTTP headers can be provided 
//! to the DRM system by calling DxImportStream_AddHTTPHeader() (the Content-Type
//! header can be provided too). Providing of HTTP headers is optional but is recommended.   
//! The information itself (HTTP body in case of HTTP) should be provided to the DRM system 
//! by calling DxImportStream_AddBuffer(). The information can be provided in chunks by sequence of calls
//! to DxImportStream_AddBuffer() with consecutive chunks of information. 
//! When the information is finished, DxImportStream_Finish() should be called. If the information stream is 
//! interrupted, DxImportStream_Cancel() should be called. 
//! When the import is over the object should be destroyed using DxImportStream_Destroy().


//! HDxImportStream is a handle used to introduce new DRM objects to the DRM system. It is created by calling DxDrmClient_CreateImportStream().
typedef void* HDxImportStream;

//! Sets the destination file name for the imported content.
//! \param[in] importStream		A handle previously created by DxDrmClient_CreateImportStream()
//! \param[in] destFileName		Destination file name represented by a null terminated string.
//! \return
//! - DX_SUCCESS - Operation succeeded. Destination filename has been set.
//! - DX_ERROR_BAD_ARGUMENTS - importStream parameter passed is null.
DX_FUNC DRM_C_API EDxDrmStatus DxImportStream_SetDestinationFile(HDxImportStream importStream, const DxChar* destFileName);

//! Introduces a single HTTP header.
//! \param[in] importStream		A handle previously created by DxDrmClient_CreateImportStream()
//! \param[in] headerName		The header name (i.e. "Content-Type")
//! \param[in] headerValue		The header value (i.e. "image/jpeg")
//! \return
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_BAD_ARGUMENTS - importStream parameter passed is null.
DX_FUNC DRM_C_API EDxDrmStatus DxImportStream_AddHTTPHeader(HDxImportStream importStream, const DxChar* headerName, const DxChar* headerValue);

//! Introduces HTTP headers that arrived with the information that is being imported.
//! \param[in] importStream	A handle previously created by DxDrmClient_CreateImportStream()
//! \param[in] httpHeaders	A string that includes lines of HTTP headers.
//!                         every line has the following syntax: "Header-Name: HeaderValue"
//!                         (This is the transmission format of the HTTP headers).
DX_FUNC DRM_C_API EDxDrmStatus DxImportStream_AddHTTPHeaders(HDxImportStream importStream, const DxChar* httpHeaders);

//! Add new chunk of information. If destination file is required the function will pop
//! UI to the user requesting destination file name.
//! \param[in] importStream		A handle previously created by DxDrmClient_CreateImportStream()
//! \param[in] data				Pointer to input data
//! \param[in] dataSize			Size of input data
//! \return
//! - DX_SUCCESS - Operation succeeded.
//! - DX_ERROR_INVALID_STATE - If the function was called in invalid state (i.e. after DxImportStream_Finish() 
//! terminated successfully or after a call to DxImportStream_Cancel())
//! - DX_ERROR_INVALID_FORMAT - If an error was found in the format of the imported information.
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - If communication with DRM server could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - importStream parameter passed is null.
DX_FUNC DRM_C_API EDxDrmStatus DxImportStream_AddBuffer(HDxImportStream importStream, const void* data, DxUint32 dataSize);

//! Cancels the operation. All operations that were done during the import should be reversed.
//! The DxImportStream_Cancel() function may be called also after DxImportStream_Finish() was called.
//! \param[in] importStream		A handle previously created by DxDrmClient_CreateImportStream()
//! \return
//! - DX_SUCCESS - Operation terminated successfully.
//! - DX_ERROR_BAD_ARGUMENTS - importStream parameter passed is null.
DX_FUNC DRM_C_API EDxDrmStatus DxImportStream_Cancel(HDxImportStream importStream);

//! Signals the import stream that all the imported information was supplied and the information
//! processing should be finalized. If destination file is required the function will pop
//! UI to the user requesting destination file name.
//! If HTTP request is available the function will send the request to the server and will supply the
//! server's response to the import stream.
//! If User consent is required the function will display the consent message to the user and will
//! supply the user response to the import stream.
//! The function will continue the process until the information is successfully imported or until
//! error encountered.
//! \return
//! - DX_DRM_SUCCESS - The information was imported successfully.
//! - DX_ERROR_INVALID_FORMAT - If an error was found in the format of the imported information.
//! - DX_ERROR_INVALID_STATE - If the function was called in invalid state (i.e. after Finish() 
//! terminated successfully or after a call to Cancel())
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - If communication with DRM server could not be established.
//! - DX_ERROR_BAD_ARGUMENTS - importStream parameter passed is null.
DX_FUNC DRM_C_API EDxDrmStatus DxImportStream_Finish(HDxImportStream importStream);

//! This function is used in order to pass parameters into the importStream. It is mostly for internal use.
//! \param[in] importStream		A handle previously created by DxDrmClient_CreateImportStream()
//! \param[in] itemName			Name of the parameter to be set. A list of available parameters is defined in "DxDrmDefines.h"
//! \param[in] itemValue		Value of the parameter to be set.
//! \return
//! - DX_DRM_SUCCESS - The information was imported successfully.
//! - DX_ERROR_BAD_ARGUMENTS - importStream parameter passed is null.
DX_FUNC DRM_C_API EDxDrmStatus DxImportStream_SetDataItem(HDxImportStream importStream, const DxChar* itemName, const DxChar* itemValue);

//! This function is used in order to get parameters previously set to the importStream. It is mostly for internal use.
//! \param[in] importStream		A handle previously created by DxDrmClient_CreateImportStream()
//! \param[in] itemName			Name of the parameter to get. A list of available parameters is defined in "DxDrmDefines.h"
//! \return
//!  The value of the parameter. NULL in case it was not found.
DX_FUNC DRM_C_API const DxChar* DxImportStream_GetDataItem(HDxImportStream importStream, const DxChar* itemName);

//! Use this function to set a UI context which might be needed by the MessageBox implementation
//! (e.g. A handle to the owner window of the message box in winCE). If UI context is set, it will be passed to all DRM Objects
//! created by this module.
//! Note: Some platforms do not require this parameter to be set.
//! \param[in] importStream		A handle previously created by DxDrmClient_CreateImportStream()
//! \param[in] uiContext	    A pointer to the UI context to be set.
DX_FUNC DRM_C_API void DxImportStream_SetUiContext(HDxImportStream importStream, void* uiContext);

//! Retrieves the UI Context previously set.
DX_FUNC DRM_C_API void* DxImportStream_GetUiContext(HDxImportStream importStream);

//! Terminates the import-stream handle.
DX_FUNC DRM_C_API void DxImportStream_Destroy(HDxImportStream* importStream);


#ifdef __cplusplus
}
#endif

#endif
