#ifndef IDX_USER_MESSAGE_H
#define IDX_USER_MESSAGE_H

#include "DxTypes.h"
#include "DxDrmDefines.h"

/*! IDxUserMessage represents a message that should be displayed to the user in order to get its consent.
	An instance of this class is retrieved from IDxCoreImportStream::GetUserMessage().
*/
class IDxUserMessage
{
public:
	virtual ~IDxUserMessage() {}
	/*! \return The type of the  message. This value can be used to decide which buttons should
		be displayed in the dialog box that is displayed to the user. */
    virtual EDxMessageType GetMessageType() const  = 0;
	
	/*! \return The severity of the message. This value can be used to decide which icon or
		title should be displayed in the dialog box that is displayed to the user. */
    virtual EDxMessageSeverity GetMessageSeverity() const = 0;

	/*! \return The code of the message. This value can be used to decide which message 
		should be displayed in the dialog box that is displayed to the user. This is useful
		if the specific message that is displayed to the user is language dependent. */
    virtual EDxMessageCode GetMessageCode() const = 0;

	/*! Retrieves the value of the value of specified parameter. This value can be embedded in the
		message text (in the appropriate place). */
    virtual const DxChar* GetMessageParam(EDxMessageParamCode paramCode) const = 0;
	
	//! Initialize the object with a given IDxUserMessage*. Copy constructor.
	//! \return
	//! - DX_SUCCESS -The initialization was successful.
	//! - DX_BAD_ARGUMENTS - The given IDxUserMessage* in null or is incompatible or initialization. 
    virtual DxStatus InitWithMessage(const IDxUserMessage* other) = 0;

};

#endif
