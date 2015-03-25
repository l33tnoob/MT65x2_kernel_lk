#ifndef IDX_HTTP_REQUEST_H
#define IDX_HTTP_REQUEST_H

#include "DxTypes.h"
#include "DxDrmDefines.h"
/*! This class represents an HTTP request that should be sent to the server.
	An instance of this class is retrieved from IDxCoreImportStream::GetHTTPRequest()
	and contains information that should be sent back to the server in order to
	complete the import of the content or rights.
	The object contains an array of HTTP headers and the request body.
	
	\note The object contains only headers which are DRM related. Other headers
	such as: Host, Content-Length should be added by the HTTP layer implementation.
*/
class IDxHttpRequest
{
public:
    virtual ~IDxHttpRequest()  {};
	
    //! \return The HTTP method of the request (GET/POST)
	virtual EDxHttpMethod GetHttpMethod() const = 0;
  
    //! \return the URL to which the HTTP request should be sent.
    virtual const DxChar* GetHttpUrl() const = 0;
	
    //! \return The number of HTTP headers that the request contains.
    virtual DxUint32 GetNumOfHttpHeaders() const = 0;
	/*! retrieve the name and value of the header which is in the specified
		position in the headers array.
		\param[in] headerIndex The position in the headers array of the header to retrieve.
		\param[out] headerName On exit, The name of the header (for example: "Content-Type").
		\param[out] headerValue On exit, The value of the header.

		\note The pointers which are returned in headerName & headerValue points
		to memory that is valid as long as the IDxHttpRequest object exists.
		You should NOT free this memory.
	*/
    virtual EDxDrmStatus GetHttpHeader(DxUint32 headerIndex, const DxChar*& headerName, const DxChar*& headerValue) const = 0;
	
    virtual EDxDrmStatus RestartHttpBody() const = 0;
	/*! Reads the data of the request body. The data that is read should be sent as the request body.
		\param[out] data                a pointer to an empty buffer that will be filled with retrieved data.
		\param[in]  dataSize            The size of the empty buffer in bytes. Number of bytes to read from file.
		\param[out] dataActuallyRead    On entry, points to an empty DxUint32 variable. MAY be NULL.
		On exit, if not NULL, the number of bytes that were actually read from the file.
		If dataActuallyRead is NULL and not all bytes could be read DX_ERROR_READ_FILE_FAILURE will be returned.
	*/
    virtual EDxDrmStatus ReadHttpBody(void* data, DxUint32 dataSize, DxUint32* dataActuallyRead) const = 0;

	virtual DxUint32 GetHttpBodySize() const = 0;
};

#endif
