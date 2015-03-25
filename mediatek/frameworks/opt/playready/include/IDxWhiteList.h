#ifndef IDX_WHITE_LIST_H
#define IDX_WHITE_LIST_H

#include "DxTypes.h"
#include "DxDrmDefines.h"

//! \file IDxWhiteList.h
//! \brief Used to manage OMA V2 RI FQDNs white list. 

//! Used to manage OMA V2 RI FQDNs white list. 
class IDxWhiteList
{
public:
    IDxWhiteList();
    virtual ~IDxWhiteList();
	//! Adds the specified FQDN to the whitelist.
    virtual EDxDrmStatus Add(const DxChar* riUrl) = 0;
	//! Removes the specified GQDN from the whitelist.
    virtual EDxDrmStatus Delete(const DxChar* riUrl) = 0;
	//! Returns the number of FQDNs that are in the whitelist.
    virtual DxUint32 GetCount() = 0;
	//! Returns the FQDN in the specified position in the whitelist.
    virtual const DxChar* GetAt(DxUint32 index) = 0;
	//! Checks is the specified FQDN is in the whitelist.
    virtual DxBool Contains(const DxChar* riUrl) = 0;
};

#endif
