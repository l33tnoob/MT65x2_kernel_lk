#ifndef DX_WM_DRM_DLA_H
#define DX_WM_DRM_DLA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "DxDrmDefines.h"	

//! \file DxWmDrmDLA.h
//! \brief Used to acquire and store rights for WMDRM content. 

//! Used to acquire and store rights for WMDRM content. 

//! Returns string returned by DRM_MGR_GetDeviceProperty('WM_DRM_SECURITYVERSION') into securityVersion
//! \param[out] securityVersion	Indicates the version of the fallback certificate.
//!								If buffer data is DX_NULL or is too small then buffer memory will be allocated by the function
//! \return
//! - DX_SUCCESS - Operation succeeded. WM_DRM_SECURITYVERSION Attribute retrieved successfully.
//! - DX_ERROR_NOT_INITIALIZED - if DRM client was not created or invalid
//! - DX_ERROR_GENERAL_FAILURE - if WMDRMDLA agent was not created or invalid or DRM_MGR_GetDeviceProperty failed
//! - DX_ERROR_BAD_ARGUMENTS - if securityVersion param equals DX_NULL
//! - DX_ERROR_CONTEXT_MISSING - if DRM context was not set by the WMDRM agent
//! - DX_ERROR_MEMORY_ALLOCATION_FAILURE - if memory could not be allocated to buffer
DX_FUNC DRM_C_API EDxDrmStatus DxWmDrmDLA_GetDRMSecurityVersion(DxDrmBuffer* securityVersion);

//! Returns string returned by DRM_MGR_GetDeviceProperty(' WM_DRM_DRMVERSION') into drmVersion
//! \param[out] drmVersion	Indicates the version of the DRM component.
//!							If buffer data is DX_NULL or is too small then buffer memory will be allocated by the function
//! \return
//! - DX_SUCCESS - Operation succeeded. WM_DRM_DRMVERSION Attribute retrieved successfully.
//! - DX_ERROR_NOT_INITIALIZED - if DRM client was not created or invalid
//! - DX_ERROR_GENERAL_FAILURE - if WMDRMDLA agent was not created or invalid or DRM_MGR_GetDeviceProperty failed
//! - DX_ERROR_BAD_ARGUMENTS - if drmVersion param equals DX_NULL
//! - DX_ERROR_MEMORY_ALLOCATION_FAILURE - if memory could not be allocated to buffer
DX_FUNC DRM_C_API EDxDrmStatus DxWmDrmDLA_GetDRMVersion(DxDrmBuffer* drmVersion);

//! Returns string returned by DRM_MGR_GetDeviceProperty('WM_DRM_CLIENTINFO') into systemInfo
//! \param[out] systemInfo	Indicates information about the client and the device certificate.
//!							If buffer data is DX_NULL or is too small then buffer memory will be allocated by the function
//! \return
//! - DX_SUCCESS - Operation succeeded. WM_DRM_CLIENTINFO Attribute retrieved successfully.
//! - DX_ERROR_NOT_INITIALIZED - if DRM client was not created or invalid
//! - DX_ERROR_GENERAL_FAILURE - if WMDRMDLA agent was not created or invalid or DRM_MGR_GetDeviceProperty failed
//! - DX_ERROR_BAD_ARGUMENTS - if systemInfo param equals DX_NULL
//! - DX_ERROR_MEMORY_ALLOCATION_FAILURE - if memory could not be allocated to buffer
DX_FUNC DRM_C_API EDxDrmStatus DxWmDrmDLA_GetSystemInfo(DxDrmBuffer* systemInfo);

//! Receives content v1 or v2 header and returns challenge payload string as returned by 
//! (base64) DRM_MGR_GenerateLicenseChallenge into challenge
//! \param[in] contentHeader		Pointer to an array of bytes containing the content header
//! \param[in] contentHeaderSize	Indicates contentHeader param length
//! \param[out] challenge			Pointer to a buffer that holds the license challenge.
//!									If buffer data is DX_NULL or is too small then buffer memory will be allocated by the function
//! \return
//! - DX_SUCCESS - License challenge retrieved successfully.
//! - DX_ERROR_NOT_INITIALIZED - if DRM client was not created or invalid
//! - DX_ERROR_GENERAL_FAILURE - if WMDRMDLA agent was not created or invalid or DRM_MGR_GenerateLicenseChallenge failed
//! - DX_ERROR_BAD_ARGUMENTS - if contentHeader or challenge params equal DX_NULL or contentHeaderSize param equals zero
//! - DX_ERROR_MEMORY_ALLOCATION_FAILURE - if memory could not be allocated to buffer
DX_FUNC DRM_C_API EDxDrmStatus DxWmDrmDLA_GetLicenseChallenge(const void* contentHeader, DxUint32 contentHeaderSize, DxDrmBuffer* challenge);

//! Receives license string (encoded in base64), passes it to the DRM_MGR_ProcessLicenseResponse()  
//! and returns the result.
//! \param[in] licenseData	Pointer to an array of bytes containing the license response
//! \param[in] licenceSize	Indicates licenseData param length
//! \return
//! - DX_SUCCESS - DRM_MGR_ProcessLicenseResponse succeeded, License was stored successfully.
//! - DX_ERROR_NOT_INITIALIZED - if DRM client was not created or invalid
//! - DX_ERROR_GENERAL_FAILURE - if WMDRMDLA agent was not created or invalid or DRM_MGR_ProcessLicenseResponse failed
//! - DX_ERROR_BAD_ARGUMENTS - if licenseData param equals DX_NULL or licenceSize equals zero
DX_FUNC DRM_C_API EDxDrmStatus DxWmDrmDLA_StoreLicense(const void* licenseData, DxUint32 licenceSize);

//! Receives metering certificate and returns challenge payload string as returned by
//! (base64) DRM_MGR_GenerateMeterChallenge into challenge
//! param[in] meterCert		certificate (XML string) that identifies the metering ID to collect data for. 
//!							The metering certificate is provided by the proxy application on the computer or by the media player 
//!							on the device when initiating this process
//! param[in] meterCertSize	Indicates meterCert param length
//! param[out] challenge	Pointer to a buffer that holds the metering challenge
//!							if buffer data is DX_NULL or is too small then buffer memory will be allocated by the function
//! \return
//! - DX_SUCCESS - DRM_MGR_GenerateMeterChallenge succeeded. Metering challenge generated successfully
//! - DX_ERROR_NOT_INITIALIZED - if DRM client was not created or invalid
//! - DX_ERROR_GENERAL_FAILURE - if WMDRMDLA agent was not created or invalid or DRM_MGR_GenerateMeterChallenge failed
//! - DX_ERROR_BAD_ARGUMENTS - if meterCert or challenge param equal DX_NULL or meterCertSize equals zero
//! - DX_ERROR_MEMORY_ALLOCATION_FAILURE - if memory could not be allocated to buffer
DX_FUNC DRM_C_API EDxDrmStatus DxWmDrmDLA_GetMeteringChallenge(const void* meterCert, DxUint32 meterCertSize, DxDrmBuffer* challenge);

//! Receives metering response, passes it to the DRM_MGR_ProcessMeterResponse()  
//! and returns the result.
//! param[in] responseData		Pointer to the input buffer containing the metering response
//! param[in] responseSize		Indicates responseData param length
//! param[out] flags    		If not NULL will return the flags for the metering response (Either 0 or DRM_METER_RESPONSE_PARTIAL)
//! \return
//! - DX_SUCCESS - DRM_MGR_ProcessMeterResponse succeeded. Device metering data cleared and metering store 
//!				   was reset for the metering IDs specified in the metering response.
//! - DX_ERROR_NOT_INITIALIZED - if DRM client was not created or invalid or DRM_MGR_ProcessMeterResponse failed
//! - DX_ERROR_GENERAL_FAILURE - if WMDRMDLA agent was not created or invalid
//! - DX_ERROR_BAD_ARGUMENTS - if responseData param equals DX_NULL or responseSize param equals zero
DX_FUNC DRM_C_API EDxDrmStatus DxWmDrmDLA_StoreMeteringResponse(const void* responseData, DxUint32 responseSize, DxUint32* flags);

#ifdef __cplusplus
}
#endif

#endif
