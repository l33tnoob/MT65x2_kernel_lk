#ifndef DX_DRM_CLIENT_H
#define DX_DRM_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif
#include "DxDrmCallbacks.h"

extern DxGetDestinationFileNameFunc g_DxGetDestinationFileName;
extern DxMessageBoxFunc g_DxMessageBox;
extern DxOpenWebBrowserFunc g_DxOpenWebBrowser;
extern DxLaunchFileFunc g_DxLaunchFile;
extern DxDownloadProgressFunc g_DxDownloadProgress[DX_NUM_OF_DOWNLOAD_CALLBACKS];
extern DxHttpConnectionFunc g_DxHttpConnection[DX_NUM_OF_HTTP_CALLBACKS];

void RegisterCallbacks();
#ifdef __cplusplus
}
#endif
#endif