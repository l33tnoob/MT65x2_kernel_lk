#ifndef IDX_WMDRM_DLA_H
#define IDX_WMDRM_DLA_H

#include "DxDrmDefines.h"

//! \file IDxWmDrmDLA.h
//! \brief Used to acquire and store rights for WMDRM content. 

//! Used to acquire and store rights for WMDRM content. 
class IDxWmDrmDLA
{
public:
	IDxWmDrmDLA();
	virtual ~IDxWmDrmDLA();
	//! Returns string returned by DRM_MGR_GetDeviceProperty('WM_DRM_SECURITYVERSION') into securityVersion
	virtual EDxDrmStatus GetDRMSecurityVersion(DxDrmBuffer* securityVersion) = 0;
	//! Returns string returned by DRM_MGR_GetDeviceProperty(' WM_DRM_DRMVERSION') into drmVersion
	virtual EDxDrmStatus GetDRMVersion(DxDrmBuffer* drmVersion) = 0;
	//! Returns string returned by DRM_MGR_GetDeviceProperty('WM_DRM_CLIENTINFO') into systemInfo
	virtual EDxDrmStatus GetSystemInfo(DxDrmBuffer* systemInfo) = 0;	
	//! Receives content v1 or v2 header and returns challenge payload string as returned by 
	//! (base64) DRM_MGR_GenerateLicenseChallenge into challenge
	virtual EDxDrmStatus GetLicenseChallenge(const void* contentHeader, DxUint32 contentHeaderSize, DxDrmBuffer* challenge) = 0;
	//! Receives license string (encoded in base64), passes it to the DRM_MGR_ProcessLicenseResponse()  
	//! and returns the result.
	virtual EDxDrmStatus StoreLicense(const void* licenseData, DxUint32 licenceSize) = 0;	
	//! Receives metering certificate and returns challenge payload string as returned by
	//! (base64) DRM_MGR_GenerateMeterChallenge into challenge
	virtual EDxDrmStatus GetMeteringChallenge(const void* certificate, DxUint32 certLength, DxDrmBuffer* challenge) = 0;	
	//! Receives metering response, passes it to the DRM_MGR_ProcessMeterResponse()  
	//! and returns the result.the flags parameter if not null wil lreturn the flags result of the function (Either 0 or DRM_METER_RESPONSE_PARTIAL)
	virtual EDxDrmStatus StoreMeteringResponse(const void* responseData, DxUint32 responseSize, DxUint32* flags) = 0;	
};

#endif
