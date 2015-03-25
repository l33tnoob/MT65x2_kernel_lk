#include "DxDrmFile.h"
#include "DxImportStream.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "DxDrmDefines.h"

//DX_FUNC DxStatus DxDrmFile_ShowLicenseInfo(HDxDrmFile& drmFile);

DX_FUNC DRM_C_API DxStatus DxRightsObject_ToString(HDxRightsObject rightsObject, DxChar* roStr, DxUint32 roStrSize);

DX_FUNC DRM_C_API const DxChar* DxImportStream_GetTextAttributeByName(HDxImportStream importStream, const DxChar* attrName);

DX_FUNC DRM_C_API EDxDrmStatus DxDrmClient_LoadConfigFile(const DxChar* fileName);


#ifdef __cplusplus
}
#endif

