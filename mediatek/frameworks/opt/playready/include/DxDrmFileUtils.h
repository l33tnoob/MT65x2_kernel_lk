#ifndef DX_DRM_FILE_UTILS_API_H
#define DX_DRM_FILE_UTILS_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "DxDrmDefines.h"
#include "VOS_API/DX_VOS_BaseTypes.h"
#include "DxInfra/CUtils/DxBuffer.h"

/*File operations*/

DX_FUNC DRM_C_API DxStatus DxDrmFileUtils_ReadFile(DxBuffer* data, const DxChar* path);

DX_FUNC DRM_C_API DxStatus DxDrmFileUtils_WriteFile(const void* data, DxUint32 dataSize, const DxChar* path);

DX_FUNC DRM_C_API DxStatus DxDrmFileUtils_DeleteFile(const DxChar* path);

DX_FUNC DRM_C_API DxStatus DxDrmFileUtils_MoveFile(const DxChar* oldPath, const DxChar* newPath);

DX_FUNC DRM_C_API DxBool DxDrmFileUtils_IsFileExist(const DxChar* path);

DX_FUNC DRM_C_API DxStatus DxDrmFileUtils_ListDirectory(DxBuffer* fileNames, const DxChar* path);	

DX_FUNC DRM_C_API DxStatus DxDrmFileUtils_CreateDirectory(const DxChar* path);		


#ifdef __cplusplus
}
#endif

#endif
