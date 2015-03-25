#ifndef IDX_DRM_CORE_CLIENT_H
#define IDX_DRM_CORE_CLIENT_H

#include "IDxCoreImportStream.h"
#include "IDxDrmCoreFile.h"
#include "IDxDrmCoreStream.h"
#include "IDxWhiteList.h"
#include "IDxWmDrmDLA.h"
#include "DxDrmDefines.h"

//! IDxDrmCoreClient is the top-level class of the Discretix Multi-Scheme DRM.
//! This class provide API that allow the calling application to be scheme agnostic
//! (i.e. it can treat all DRM schemes the same way).
//! an object of this class may be used to create other objects that provide additional
//! functionality.
//! IDxDrmCoreClient is an abstract class. An instance of this class can be obtained by calling
//! DxCreateDrmCoreClient(). This instance MUST be deleted (using the delete operator) at the end of use.
//! Most of the methods of IDxDrmCoreClient and of the objects that are created by it return 
//! EDxDrmStatus as error code. This error code can be one of these generic codes:
//! - DX_SUCCESS - On Success
//! - DX_ERROR_BAD_ARGUMENTS - If at least one of the provided argument is invalid.
//! - DX_ERROR_MEMORY_ALLOCATION_FAILURE - If a memory allocation failed.
//! - DX_ERROR_GENERAL_FAILURE - Some internal error occurred.
//! Any other specific error codes that are returns by the function will be listed in the function
//! documentation.
//! When a function received parameter of type "const DxChar*" it expected NULL terminated string. 
//! All the strings that are accepted or returned by the Discretix client are UTF8 encoded.

class IDxDrmCoreClient
{
public:
    IDxDrmCoreClient();
    virtual ~IDxDrmCoreClient();
    //! Identifies the DRM Scheme of the content in the file which is identified by fileName.
    //! \param[in]  fileName            name of the content file 
    //! \param[out] drmScheme           holds the value of the identified DRM Scheme on exit.
    //! \param[out] fileMimeType        empty buffer that on exit will hold the file mime type. 
    //!                                 This value may be NULL if file mime type is not required.
    //! \param[in]  fileMimeTypeSize    the size in bytes of fileMimeType buffer
    //! \param[out] contentMimeType     empty buffer that on exit will hold the content mime type. 
    //!                                 This value may be NULL if content mime type is not required.
    //! \param[in]  contentMimeTypeSize the size in bytes of contentMimeType buffer
	//! \param[out] rightsStatus	    On exit will indicate if rights are available for this content. 
	//! \param[out] drmContentType      On exit will indicate whether content is drmFile, drmStream or neither. 
	//! \note If fileMimeType or contentMimeType are too small to contain
    //!     the entire mime type they will contain a truncated version of the mime type.
    //!     buffers with size DX_MAX_MIME_TYPE_SIZE should be enough to contain the entire mime type.
    //! \return
    //! - DX_SUCCESS - if operation succeeded. Inspect the value of drmScheme in order to know if
    //!     content is a DRM content or not.
    //! - DX_ERROR_FILE_ACCESS_ERROR - If file not exist or file could not be opened or read.
    virtual EDxDrmStatus IsDrmContentByFileName(const DxChar* fileName, EDxDrmScheme& drmScheme, 
        DxChar* fileMimeType, DxUint32 fileMimeTypeSize, 
        DxChar* contentMimeType, DxUint32 contentMimeTypeSize,
		EDxRightsStatus* rightsStatus, EDxDrmContentType* drmContentType) = 0;

    //! Identifies the DRM Scheme of the content in the file which is identified by osFileHandle.
    //! \param[in]  osFileHandle    Already opened OS file handle.
    //! \param[out] drmScheme       holds the value of the identified DRM Scheme on exit.
    //! \param[out] fileMimeType        empty buffer that on exit will hold the file mime type. 
    //!                                 This value may be NULL if file mime type is not required.
    //! \param[in]  fileMimeTypeSize    the size in bytes of fileMimeType buffer
    //! \param[out] contentMimeType     empty buffer that on exit will hold the content mime type. 
    //!                                 This value may be NULL if content mime type is not required.
    //! \param[in]  contentMimeTypeSize the size in bytes of contentMimeType buffer
	//! \param[out] drmContentType      On exit will indicate whether content is drmFile, drmStream or neither. 
	//! \return
    //! - DX_SUCCESS - if operation succeeded. Inspect the value of drmScheme in order to know if
    //!     content is a DRM content or not.
    //! - DX_ERROR_FILE_ACCESS_ERROR - If file not exist or file could not be opened or read.
    virtual EDxDrmStatus IsDrmContentByFileHandle(DxOSFileHandle osFileHandle, EDxDrmScheme& drmScheme,
        DxChar* fileMimeType, DxUint32 fileMimeTypeSize, 
		DxChar* contentMimeType, DxUint32 contentMimeTypeSize,
		EDxDrmContentType* drmContentType) = 0;

    //! Identifies the DRM Scheme of the content that is in the contentData buffer.
    //! \param[in]  contentData     buffer that holds content data (which is usually read from the beginning of a content file)
    //! \param[in]  dataSize        size in bytes of the contentData buffer.
    //! \param[out] drmScheme       holds the value of the identified DRM Scheme on exit.
    //! \param[out] fileMimeType        empty buffer that on exit will hold the file mime type. 
    //!                                 This value may be NULL if file mime type is not required.
    //! \param[in]  fileMimeTypeSize    the size in bytes of fileMimeType buffer
    //! \param[out] contentMimeType     empty buffer that on exit will hold the content mime type. 
    //!                                 This value may be NULL if content mime type is not required.
    //! \param[in]  contentMimeTypeSize the size in bytes of contentMimeType buffer
	//! \param[out] drmContentType      On exit will indicate whether content is drmFile, drmStream or neither. 
	//! \return
    //! - DX_SUCCESS - if operation succeeded. Inspect the value of drmScheme in order to know if
    //!     content is a DRM content or not.
    //! - DX_ERROR_BUFFER_IS_TOO_SMALL -  The data in the contentData buffer is not enough to complete the identification.
    //!     A 264 bytes should be enough to assure identification.
    virtual EDxDrmStatus IsDrmContentByData(const void* contentData, DxUint32 dataSize, EDxDrmScheme& drmScheme,
        DxChar* fileMimeType, DxUint32 fileMimeTypeSize, 
		DxChar* contentMimeType, DxUint32 contentMimeTypeSize,
		EDxDrmContentType* drmContentType) = 0;

	//! Returns the rights status of the given DRM file.
	//! \param[in]  fileName	    The name of the DRM file.
	//! \return
	//! - DX_RIGHTS_STATUS_NOT_PROTECTED - If not a DRM content.
	//! - DX_RIGHTS_STATUS_NOT_VALID - If the content has no valid rights.
	//! - DX_RIGHTS_STATUS_VALID - If the content has valid rights.
	//! - DX_RIGHTS_STATUS_FORWARD_LOCK - If the content has valid rights and is forward locked.
	virtual EDxRightsStatus GetRightsStatus(const DxChar* fileName) = 0;

	//! Retrieve set of flags that are available to the specified content.
	//! \param[in]	fileName		Specifies DRM content fileName from which to retrieve the set of flags. 
	//! \param[in]  flagsToCheck    OR-ed combination of flags to be retrieved. 
	//!                             The OR-ed values should be taken from the EDxContentFlag enumeration.
	//! \param[out] activeFlags     On exit, Only the flags that were specified in flagsToCheck and are active will be on.
	//! \param[in]  permissions     OR-ed combination of permissions taken from the EDxPermission enumeration.
	//!                             The flags that will be retrieved will be relevant for the specified permissions.
	//! \return
	//! - DX_SUCCESS - On Success
	//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - If the flags that were asked cannot be computed correctly because the secure clock is not set.
	//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
	//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
	virtual EDxDrmStatus GetContentFlags(const DxChar* fileName, DxUint32 flagsToCheck, DxUint32& activeFlags, DxUint32 permissions) = 0;
	

	//! Determines whether the given mimeType is recognized by the DRM client.
	//! If so, the content might be imported using CreateImportStream().
	//! \param[in]  mimeType	    The mimeType to be checked.
	//! \param[out] drmScheme	    If not Null it will hold the value of the DrmScheme to which the mimeType belongs (OmaV1, OmaV2...).
	//! \return
	//! - DX_TRUE - If mimeType was recognized by the DRM client.
	//! - DX_FALSE - If mimeType was NOT recognized by the DRM client.
	virtual DxBool IsDrmMimeType(const DxChar* mimeType, EDxDrmScheme* drmScheme) = 0;

	virtual EDxDrmStatus DetectMimeType(const void* data, DxUint32 dataSize, DxChar* mimeType, DxUint32 mimeTypeSize, DxUint32& startOffset) = 0;

    //! Opens a DRM content file that is specified by fileName for usage.
    //! \param[out] drmFile     On entry, SHOULD be NULL. 
    //!                         On exit, points to a newly allocated DRM file object. 
    //!                         This object MUST be deleted at the end of use using the delete operator.
    //! \param[in]  fileName    The name of the content file. MUST NOT be NULL.
    //! \param[in]  openMode	Specifies if the file should be opened for read, write or both.
    //! \return
    //! - DX_SUCCESS - if DRM Scheme could be identified.
    //! - DX_ERROR_CONTENT_NOT_RECOGNIZED - if DRM Scheme could not be identified.
    //! - DX_ERROR_FILE_ACCESS_ERROR - If file not exist or file could not be opened or read.
    //! - DX_ERROR_FILE_IS_BEING_DOWNLOADED - If the file is being downloaded and there is not enough
    //!     data to begin using it.
    virtual EDxDrmStatus OpenDrmFileByName(IDxDrmCoreFile*& drmFile, const DxChar* fileName, EDxFileOpenMode openMode = DX_OPEN_FOR_READ) = 0;

	//! Opens a DRM content file that is specified by osFileHandle for usage.
	//! \param[out] drmFile     On entry, SHOULD be NULL. 
	//!                         On exit, points to a newly allocated DRM file object. 
	//!                         This object MUST be deleted at the end of use using the delete operator.
	//! \param[in] osFileHandle Opened file handle for the file. This file handle will
	//!                         be closed when the drmFile object will be deleted. MUST NOT be NULL.
	//! \param[in] fileName     The name of the content file. MAY be NULL.
	//! \return
	//! - DX_SUCCESS - if DRM Scheme could be identified.
	//! - DX_ERROR_CONTENT_NOT_RECOGNIZED - if DRM Scheme could not be identified.
	//! - DX_ERROR_FILE_ACCESS_ERROR - If file could not be read.
	//! - DX_ERROR_FILE_IS_BEING_DOWNLOADED - If the file is being downloaded and there is not enough
	//!     data to begin using it.
	virtual EDxDrmStatus OpenDrmFileByHandle(IDxDrmCoreFile*& drmFile, DxOSFileHandle osFileHandle, const DxChar* fileName = DX_NULL) = 0;

	//! Opens a DRM content stream (PDCF) that is specified by fileName for usage.
	//! \param[out] drmStream   On entry, SHOULD be NULL. 
	//!                         On exit, points to a newly allocated DRM stream object. 
	//!                         This object MUST be deleted at the end of use using the delete operator.
	//! \param[in]  fileName    The name of the content stream file. MUST NOT be NULL.
	//! \param[in]  openMode	Specifies if the file should be opened for read, write or both.
	//! \return
	//! - DX_SUCCESS - if DRM Scheme could be identified.
	//! - DX_ERROR_CONTENT_NOT_RECOGNIZED - if DRM Scheme could not be identified.
	//! - DX_ERROR_FILE_ACCESS_ERROR - If file not exist or file could not be opened or read.
	//! - DX_ERROR_FILE_IS_BEING_DOWNLOADED - If the file is being downloaded and there is not enough
	//!     data to begin using it.
	virtual EDxDrmStatus OpenDrmStreamByName(IDxDrmCoreStream*& drmStream, const DxChar* filename, DxUint32 openMode = DX_OPEN_FOR_READ) = 0;

    virtual EDxDrmStatus OpenDrmStreamFromData(IDxDrmCoreStream*& drmStream, const void* data, DxUint32 dataSize) = 0;

	//! This function should be used to open a drm content (file or stream)
	//! \note The IDxDrmCoreContent object should be deleted!.
	//! \return
	//! - DX_DRM_SUCCESS - Operation succeeded. drmCoreContent points to a valid DRM content.
	//! - DX_ERROR_GENERAL_FAILURE - Operation failed.
	//! - DX_ERROR_CONTENT_NOT_RECOGNIZED - The fileName points to a content not recognized by the client
	virtual EDxDrmStatus OpenDrmContentByName(IDxDrmCoreContent*& drmCoreContent, const DxChar* fileName, EDxFileOpenMode openMode = DX_OPEN_FOR_READ) = 0;

    //! Notifies the DRM client that a file is about to be deleted - 
    //! The client will delete the file name from the content cache 
    //! \param[in]  fileName            file name to delete from content cache
    //! \note The function should be called before the file was actually deleted	
    //!		fileName MUST NOT be null
    //! \return
    //! - DX_DRM_SUCCESS - if operation succeeded. 	
    //! - DX_ERROR_BAD_ARGUMENTS - fileName parameter passed is DX_NULL
    //! - DX_ERROR_FILE_ACCESS_ERROR - If file (fileName) not exist or file could not be opened or read.
    virtual EDxDrmStatus OnFileDelete(const DxChar* fileName) = 0; 

    //! Notifies the DRM client that a new file was created/moved - 
    //! The client will add the fileName to the content cache 
    //! \param[in]  fileName            file name to add to the content cache
    //! \param[in]  oldFileName         when the new content was created by a move operation
    //!					then oldFileName should indicate the content old name - can be null
    //! \note The function should be called after the file was created/moved	
    //! \return
    //! - DX_DRM_SUCCESS - if operation succeeded.	
    //! - DX_ERROR_BAD_ARGUMENTS - fileName parameter passed is DX_NULL
    //! - DX_ERROR_FILE_ACCESS_ERROR - If file (fileName) not exist or file could not be opened or read.
    virtual EDxDrmStatus OnNewFile(const DxChar* fileName, const DxChar* oldFileName = DX_NULL) = 0; 

	virtual EDxDrmStatus AddHttpAuthenticationInfo(const DxChar* host, const DxChar* realm, const DxChar* username, const DxChar* password) = 0;
	
	virtual EDxDrmStatus GetHttpAuthenticationInfo(const DxChar* host, const DxChar* realm,
		DxChar* username, DxUint32 usernameSize,
		DxChar* password, DxUint32 passwordSize) = 0;


	virtual EDxDrmStatus GetJoinDomainChallenge( void* service_Id , DxUint32 serviceLen, void* account_Id, DxUint32 accountLen, DxUint32* domainRevision, void* friendlyName, DxUint32 friendlyNameLen, void* customData, DxUint32 customDataLen, void* challenge, DxUint32* size) = 0 ;

	virtual EDxDrmStatus GetLeaveDomainChallenge( void* service_Id , DxUint32 serviceLen, void* account_Id, DxUint32 accountLen, DxUint32* domainRevision, void* customData, DxUint32 customDataLen, void* challenge, DxUint32* size) = 0;

	virtual EDxDrmStatus GetLicenseAcq_GenerateAck( void** licenseResponse , void* challenge, DxUint32* size) = 0 ;

	virtual EDxDrmStatus GetMeterCertChallenge( void* metering_Id, DxUint32 meteringLen, void* customData,  DxUint32 customDataLen, void* challenge, DxUint32* size) = 0;

	virtual EDxDrmStatus GetMeteringChallenge( void* metering_Id, DxUint32 meteringLen,  void* customData, DxUint32 customDataLen, void* meterServerUrl, DxUint32* meterServerUrlLen, void* challenge, DxUint32* size) = 0;

	virtual EDxDrmStatus ProcessLicenseAcqResponse( const void* serverResponse, DxUint32 serverResponseSize, void** serverResponseResult, DxBool* isAckRequired) = 0;

	virtual EDxDrmStatus ProcessLicenseAcq_AckResponse( const void* serverResponse, DxUint32 serverResponseSize) = 0;

	virtual EDxDrmStatus ProcessJoinDomainResponse( const void* serverResponse, DxUint32 serverResponseSize) = 0;

	virtual EDxDrmStatus ProcessLeaveDomainResponse( const void* serverResponse, DxUint32 serverResponseSize) = 0;

	virtual EDxDrmStatus ProcessMeterCertResponse( const void* serverResponse, DxUint32 serverResponseSize) = 0;

	virtual EDxDrmStatus ProcessMeteringResponse( const void* serverResponse, DxUint32 serverResponseSize) = 0;
#ifndef DX_ONLY_CONSUMPTION_SUPPORT

	//! Retrieves the value of the specified global text attribute.
	//! The value of the text attribute is set in the preallocated buffer - attrValue.
	//! If the attribute could not be found or invalid contentIndex was specified, an error code will be returned.
	//! \param[in]  textAttr		Specifies the attribute to retrieve.
	//! \param[out] attrValue		attribute value buffer. On exit will hold the value of the text attribute.
	//! \param[in] attrValueSize   Size of the buffer.
	//! - return
	//DX_SUCCESS - on success
	//DX_BUFFER_NOT_BIG_ENOUGH - if the given buffer is too small, for the given text attribute
	virtual EDxDrmStatus GetTextAttribute(EDxTextAttribute textAttr, DxChar* attrValue, DxUint32 attrValueSize) = 0;

    //! Creates an import stream that will be used to import new content or rights to the DRM system.
    //! Import streams are usually useful for importing content that is being downloaded to the device.
    //! possible data can be:
    //! - OMA V1 Content (Forward Lock, Combined Delivery, OMA V1 DCF)
    //! - OMA V1 Rights Objects (Separate Delivery)
    //! - OMA V2 ROAP Triggers & Messages
    //! - OMA V2 Rights Objects (also Pre-Loaded)
    //! - OMA V2 Content (OMA V2 DCF)
    //! - WMDRM Content
    //! - HTMLs including WMDRM License response.
    //! \param[out] importStream    On entry, SHOULD be NULL. 
    //!                             On exit, points to a newly allocated Import Stream object. 
    //!                             This object MUST be deleted at the end of use using the delete operator.
    //! \param[in]  mimeType        MIME type of the content or rights to import.
    //! \return
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_MIME_TYPE_NOT_RECOGNIZED - If MIME type is not recognized as DRM content MIME type.
    virtual EDxDrmStatus CreateImportStream(IDxCoreImportStream*& importStream, const DxChar* mimeType) = 0;

    //! Backups the secure storage. This backup can be used later by RestoreStorage() to restore factory
    //! settings. If a previous backup is already exists it will be overwritten.
    //! \param[in] backupName	Optional name of the backup. If NULL is provided a default backup will be created.
    //! \return
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is found to be corrupted.
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    //virtual EDxDrmStatus BackupStorage(const DxChar* backupName = DX_NULL) = 0;

    //! Restores the secure storage from backup that was previously created using BackupStorage().
    //! \param[in] backupName	Optional name of the backup to restore. 
    //!                         If NULL is provided the default backup will be restored.
    //! \return
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_NOT_AVAILABLE - If backup was not previously created by BackupStorage().
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    //virtual EDxDrmStatus RestoreStorage(const DxChar* backupName = DX_NULL) = 0;

    //! Deletes the assets specified by assetBitMask from the secure storage.
    //! \param[in]  assetBitMask    an OR-ed combination of assets to delete. 
    //!                             Asset values should be taken from EDxAsset enumeration.
    //! \return
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is found to be corrupted.
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    virtual EDxDrmStatus DeleteAssets(DxUint32 assetBitMask) = 0;

    //! Deletes expired Rights objects & Licenses from the secure storage.
    //! Expired Rights objects are consumable objects (i.e. count, timed count, interval, accumulated)
    //! that were fully consumed or time based objects whose validity period is over.
    //! \return
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is found to be corrupted.
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    virtual EDxDrmStatus DeleteObsoleteRights() = 0;
	
    //! Creates an object that allows handling of the OMA V2 WhiteList.    
    //! On exit, points to a newly allocated WhiteList object. 
    //! This object MUST be deleted at the end of use using the delete operator.    
    virtual IDxWhiteList* GetWhiteList() = 0;

	//! Creates an object that allows handling of the WM Direct License Acquisition process.	
	//! On exit, points to a newly allocated DLA object. 
	//! This object MUST be deleted at the end of use using the delete operator.	
	virtual IDxWmDrmDLA* GetWmDrmDLA() = 0;

    //! Stores an asset in the secure storage as part of the provisioning process. The format of the
    //! data buffer should match the asset type described in assetType.
    //! The following table list the asset types that can be specified and the matching data format.
    //! - DX_STORE_OMA_V2_CERTIFICATE             DER encoded Certificate
	//! - DX_STORE_OMA_V2_PROVISION_PACKAGE       Discretix generated provisioning package.
	//! - DX_STORE_OMA_V2_PRIVATE_KEY             TBD   
    //! - DX_STORE_WM_CERTIFICATE_TEMPLATE        UTF16 encoded XML with MS Device Certificate Template
    //! - DX_STORE_WM_CERTIFICATE,                UTF16 encoded XML with MS Device Certificate
    //! - DX_STORE_WM_GROUP_PRIVATE_KEY           20 binary bytes that hold the WMDRM Group Private Key.
    //! - DX_STORE_WM_FALLBACK_KEYS               20 binary bytes that hold the WMDRM fall-back keys.
    //! \return
    //! - DX_SUCCESS - On Success
    //! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and is found to be corrupted.
    //! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
    //! - DX_ERROR_INVALID_FORMAT - If the format of the asset to be stored does not match the asset type.
    //! - DX_ERROR_TAG_ALREADY_EXIST -  The asset cannot be stored because similar asset is already provisioned
    //!     on the device. Usually a call to ProvisioningDelete() with the specified tag will remove the asset
    //!     from the secure storage.
    //! - DX_ERROR_ASSET_ALREADY_EXIST - An identical asset (i.e. with the same id) already exists in the
    //!     secure storage (even if it was stored with different tag). A call to DeleteAssets() with the 
    //!     appropriate value will remove the asset from the secure storage. 
    virtual EDxDrmStatus ProvisioningStore(EDxStoreAsset assetType, const void* assetTag, DxUint32 tagSize, const void* data, DxUint32 dataSize) = 0;

    virtual EDxDrmStatus ProvisioningDeviceCredentials(const void* assetTag, DxUint32 tagSize, const void* deviceCert, DxUint32 deviceCertSize,
        const void* privateKeyModulus, DxUint32 privateKeyModulusSize, const void* privateKeyExponent, DxUint32 privateKeyExponentSize) = 0;

    virtual EDxDrmStatus ProvisioningDeviceCredentials(const void* assetTag, DxUint32 tagSize, const void* deviceCert, DxUint32 deviceCertSize,
        const void* privateKeyP, DxUint32 privateKeyPSize, const void* privateKeyQ, DxUint32 privateKeyQSize, const void* privateKeyExponent, DxUint32 privateKeyExponentSize) = 0;

    //! Verifies that the provisioning process of the assets that were stored with the specified tag 
    //! was completed successfully. drmScheme specified the DRM scheme of the provisioned assets that need
    //! to be verified.
    //! The following table lists the verification that is being done for every scheme.
    //! - DX_SCHEME_OMA_V2      The data buffer should contain ID(TBD) of trusted authority.
    //!                         The function will verify that a device certificate chain that lead to
    //!                         the trusted authority root certificate was stored on the device.
    //!                         It will also verify the existence of a private key that match the 
    //!                         public key that is in the device certificate that is included in this chain.
    //! - DX_SCHEME_WMDRM       The data buffer is ignored. The function will verify that a device certificate
    //!                         exists. If device certificate does not exist the function will try to create
    //!                         it from the certificate template and group private key (if they exist).
    //!                         The function will verify that the group private key can be used to decrypt
    //!                         the device private key which appears in the device certificate.
    virtual EDxDrmStatus ProvisioningVerify(EDxDrmScheme drmScheme, const void* assetTag, DxUint32 tagSize) = 0;

    //! Deletes from secure storage all the assets of the specified DRM schemed that were stored 
    //! with the specified tag.  
    virtual EDxDrmStatus ProvisioningDelete(EDxDrmScheme drmScheme, const void* assetTag, DxUint32 tagSize) = 0;	

	//! @TODO add explanation
	//virtual EDxDrmStatus ProvisionPackage(const void* data, DxUint32 dataSize) = 0;

	//! @TODO add explanation
    virtual EDxDrmStatus MasterClockHasChanged() = 0;

    // try to set the secure clock for the given DRM scheme.
    virtual EDxDrmStatus SynchronizeDrmTime(IDxCoreImportStream*& importStream, EDxDrmScheme drmScheme) = 0;

    //virtual EDxDrmStatus GetSecureClock(const DxChar* clockId, DxTimeStruct& timeStruct) = 0;

    virtual EDxDrmStatus SetDrmSettingsValue(EDxDrmSettingsItem settingsItem, DxUint32 settingsItemValue) = 0;

    virtual EDxDrmStatus GetDrmSettingsValue(EDxDrmSettingsItem settingsItem, DxUint32& settingsItemValue) = 0;

	virtual EDxDrmStatus ProcessMtpCommand(const void* inData, DxUint32 inSize, DxDrmBuffer& outputData) = 0;

#endif
};

//! This function should be used to initialize the DRM core client.
//! \note DxDrmCoreClient_Terminate() should be called at the end of use (after all DRM objects are deleted).
//! \return
//! - DX_DRM_SUCCESS - Operation succeeded. New DRM client object was created.
//! - DX_ERROR_MEMORY_ALLOCATION_FAILURE - if memory allocation failed.
EDxDrmStatus DxDrmCoreClient_Init();

//! This function is used to terminate the DRM client.
//! It should not be called if there are active DRM objects (such as IDxDrmCoreFile, IDxDrmCoreImportStream etc.)
void DxDrmCoreClient_Terminate();

//! This function should be used to get an instance of the DRM core client.
//! \note The IDxDrmCoreClient object should not be deleted!.
//! \return
//! - DX_DRM_SUCCESS - Operation succeeded. drmClient point to a valid DRM client
//! - DX_ERROR_NOT_INITIALIZED - If the DRM client was not initialized, or has been terminated.
EDxDrmStatus DxDrmCoreClient_Get(IDxDrmCoreClient*& drmClient);
//EDxDrmStatus DxDrmCoreClient_DecryptFile(const DxChar* aDrmFileName, const DxChar* aDestFileName);

EDxDrmStatus DxDrmCoreClient_SetDeviceDetails(const DxChar* imsiStr, const DxChar* manufacturer,const DxChar* model, const DxChar* revision);

EDxDrmStatus DxDrm_QuickReset();

#endif

