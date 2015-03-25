#ifndef DX_WHITE_LIST_H
#define DX_WHITE_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "DxDrmDefines.h"

/*WhiteList operations*/

//! Adds the specified FQDN to the whitelist.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
DX_FUNC DRM_C_API EDxDrmStatus DxWhiteList_Add(const DxChar* riUrl);

//! Removes the specified GQDN from the whitelist.
//! \return
//! - DX_SUCCESS - Operation completed successfully.
//! - DX_ERROR_NOT_INITIALIZED - The DRM Client has not been initialized.
DX_FUNC DRM_C_API EDxDrmStatus DxWhiteList_Delete(const DxChar* riUrl);

//! Returns the number of FQDNs that are in the whitelist.
DX_FUNC DRM_C_API DxUint32 DxWhiteList_GetCount();

//! Returns the FQDN in the specified position in the whitelist.
DX_FUNC DRM_C_API const DxChar* DxWhiteList_GetAt(DxUint32 index);

//! Checks is the specified FQDN is in the whitelist.
DX_FUNC DRM_C_API DxBool DxWhiteList_Contains(const DxChar* riUrl);

#ifdef __cplusplus
}
#endif

#endif
