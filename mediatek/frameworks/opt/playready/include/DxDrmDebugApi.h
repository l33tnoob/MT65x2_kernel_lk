#ifndef DX_DRM_DEBUG_API_H
#define DX_DRM_DEBUG_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "DxDrmDefines.h"

DX_FUNC DRM_C_API DxStatus DxLoadConfigFile(const DxChar *fileName);
DX_FUNC DRM_C_API void DxTerminateConfigFile();
DX_FUNC DRM_C_API const DxChar* DxGetConfigString(const DxChar *propertyName, const DxChar *defaultValue);
DX_FUNC DRM_C_API DxUint32 DxGetConfigNumber(const DxChar *propertyName, DxUint32 defaultValue);
DX_FUNC DRM_C_API DxBool DxGetConfigBoolean(const DxChar *propertyName, DxBool defaultValue);
DX_FUNC DRM_C_API DxStatus DxSetConfigString(const DxChar* propertyName, const DxChar* propertyValue);
DX_FUNC DRM_C_API DxStatus DxSetConfigNumber(const DxChar* propertyName, DxUint32 propertyValue);
DX_FUNC DRM_C_API DxStatus DxSetConfigBoolean(const DxChar* propertyName, DxBool propertyValue);

DX_FUNC DRM_C_API DxUint32 DxEnableModuleLogPrinting(DxUint32 ModuleCode);
DX_FUNC DRM_C_API DxUint32 DxDisableModuleLogPrinting(DxUint32 ModuleCode);
DX_FUNC DRM_C_API const DxChar* DxOpenLogFile(const DxChar* fileName);
DX_FUNC DRM_C_API void DxCloseLogFile();
DX_FUNC DRM_C_API void DxFlushLogFile();

DX_FUNC DRM_C_API DxStatus DxSetSecureTime(const DxChar* clockId, DxInt32 secs);
DX_FUNC DRM_C_API DxStatus DxGetSecureTime(const DxChar* clockId, DxInt32* outVal);
DX_FUNC DRM_C_API DxStatus DxUnsetSecureTime();
DX_FUNC DRM_C_API DxStatus DxSyncronizeOmaV1Time();

DX_FUNC DRM_C_API DxStatus DxUnsetMeteringConsent();

DX_FUNC DRM_C_API DxStatus DxRestartServer();
DX_FUNC DRM_C_API DxBool DxIsServerActive();

DX_FUNC void DX_DBG_Print(const DxChar* str);

#ifdef __cplusplus
}
#endif

#endif
