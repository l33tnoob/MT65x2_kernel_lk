#ifndef DX_DRM_VERSION_H
#define DX_DRM_VERSION_H

/*! \file DxDrmVersion.h
This module enables the DRM to reflect it's build version.

The software version name associated with each version (build) has:
-	<Customer prefix>. This field is used when the software is sent to a spesific customer.
-	Product-code to indicate product-name
-	A letter to indicate version type
-	Four sets of digits to indicate version numbering

As follows: 
CUSTOMER Prefix_NNNN_{"D", "B", "R"} TT.XX.YY.ZZ   (for example: CRYS-B01.01.02.46)

*/
#include "DxDrmDefines.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DX_VERSION_STRING_SIZE	100
#define DX_TIME_STAMP_SIZE	100


DxBool DxDrmVersion_GetVersion(DxChar* buff, DxUint32 buffSize);

DxBool DxDrmVersion_GetTimeStamp(DxChar* buff, DxUint32 buffSize);

//! this function will log the DRM version with the current registered logging functions (can be file/socket/screen/logcat)
void DxDrmVersion_LogVersion();

//! Make sure the logging functions are registered and call DxDrmVersion_LogVersion.
void DxDrmVersion_ForceLogVersion();


#ifdef __cplusplus
}
#endif

#endif

