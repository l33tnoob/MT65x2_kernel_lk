#ifndef DX_DRM_CORE_DEFINES_H
#define DX_DRM_CORE_DEFINES_H

#include "DxTypes.h"

// We limit the RO size in order to limit the size of malloc. This is a magic number.
#define DX_MAX_RO_SIZE          (20*1024)
#define DX_MAX_MIME_TYPE_SIZE   ((256+1)*2)// for prevent utf8 to utf16 conversion problem.

// For progressive-dl, when reading a file that is being downloaded this is the max time the read() function will block.
#define DX_MAX_READ_SECS_TIMEOUT 10


// OMA V1 mime-types
#define DX_OMA_V1_CONTENT_MIME_TYPE				"application/vnd.oma.drm.content"
#define DX_OMA_V1_DRM_MESSAGE_MIME_TYPE			"application/vnd.oma.drm.message"
#define DX_OMA_V1_RO_XML_MIME_TYPE				"application/vnd.oma.drm.rights+xml"
#define DX_OMA_V1_RO_WBXML_MIME_TYPE			"application/vnd.oma.drm.rights+wbxml"
#define DX_DD_XML_MIME_TYPE						"application/vnd.oma.dd+xml"
#define DX_DD_V2_XML_MIME_TYPE					"application/vnd.oma.dd2+xml"
#define DX_MULTIPART_RELATED_MIME_TYPE			"multipart/related"

// OMA V2 mime-types
#define DX_OMA_V2_CONTENT_MIME_TYPE				"application/vnd.oma.drm.dcf"
#define DX_OMA_V2_RO_XML_MIME_TYPE				"application/vnd.oma.drm.ro+xml"
#define DX_OMA_V2_TRIGGER_MIME_TYPE				"application/vnd.oma.drm.roap-trigger+xml"
#define DX_OMA_V2_TRIGGER_WBXML_MIME_TYPE		"application/vnd.oma.drm.roap-trigger+wbxml"
#define DX_OMA_V2_ROAP_PDU_MIME_TYPE			"application/vnd.oma.drm.roap-pdu+xml"

// WMDRM mime-types
#define DX_WMDRM_LICENSE_MIME_TYPE				"application/vnd.ms-wmdrm.lic-resp"
#define DX_WMDRM_METERING_REQ_MIME_TYPE			"application/vnd.ms-wmdrm.meter-chlg-req"
#define DX_WMDRM_WMA_MIME_TYPE					"audio/x-ms-wma"
#define DX_WMDRM_WMV_MIME_TYPE					"video/x-ms-wmv"
#define DX_WMDRM_ASF_MIME_TYPE					"video/x-ms-asf"

// PlayReady mime-types
#define DX_PLAYREADY_INITIATOR_MIME_TYPE		"application/vnd.ms-playready.initiator+xml"
#define DX_PLAYREADY_AUDIO_MIME_TYPE			"audio/vnd.ms-playready.media.pya"
#define DX_PLAYREADY_VIDEO_MIME_TYPE			"video/vnd.ms-playready.media.pyv"
#define DX_PLAYREADY_ENVELOPE_MIME_TYPE			"application/vnd.ms-playready.envelope"	// No mime-type is defined for envelope files so we invented one
#define DX_PLAYREADY_LICENSE_RESPONSE_MIME_TYPE	"application/vnd.ms-playready.lic-resp"	// No mime-type is defined for playReady license responses so we invented one

#define DX_ISMA_MIME_TYPE                       "audio/isma"
#define DX_ISMV_MIME_TYPE                       "video/ismv"

#define DX_PIFF_MIME_TYPE						DX_ISMV_MIME_TYPE
#define DX_PIFF_MANIFEST_MIME_TYPE				"video/vnd.ms.smoothstreaming"	// No mime-type is defined for smooth streaming files so we invented one

#define DX_HLS_M3U8_PLAYLIST_MIME_TYPE			"application/x-mpegURL"
#define DX_HLS_M3U8_APPLE_PLAYLIST_MIME_TYPE	"application/vnd.apple.mpegURL"
#define DX_HLS_VIDEOTS_MIME_TYPE				"video/MP2T"


#define FILE_EXTENSION_DCF						".dcf"
#define FILE_EXTENSION_ODF						".odf"
#define FILE_EXTENSION_DRM						".drm"

#define DX_DEFAULT_SECS_TO_RIGHTS_ARRIVAL 25

//! Provides a file name (without extension) that may be suggested for the user when
//! she is asked to provide the destination file name.
#define DX_DATA_ITEM_SUGGESTED_FILE_NAME		"SuggestedFileName"
#define DX_DATA_ITEM_IMPORT_SOURCE_FILE_NAME	"SourceFileName"
#define DX_DATA_ITEM_IMPORT_SOURCE_URL			"SourceUrl"
#define DX_DATA_ITEM_OPERATION_NAME				"OperationName"
#define DX_DATA_ITEM_OPERATION_FAILURE_TEXT		"OperationFailureText"
#define DX_DATA_ITEM_DEST_FILE_NAME				"DestFileName"
#define DX_DATA_ITEM_SET_INTENT_IN_PROGRESS		"SetIntentInProgress"
#define DX_DATA_ITEM_OPEN_FILE_AFTER_DOWNLOAD	"ShouldOpenFile"
#define DX_DATA_ITEM_INTERNET_CONNECTION_OPEN	"InternetConnectionOpen"
#define DX_DATA_ITEM_TRANSPORT_METHOD			"Transport-Method"


typedef enum EDxDrmInternalSettingsItem
{
	DX_INTERNAL_SETTINGS_FIRST_ITEM = 100,
	DX_INTERNAL_SETTINGS_PLAYREADY_RESET_STATE = DX_INTERNAL_SETTINGS_FIRST_ITEM,
	DX_NUM_OF_INTERNAL_SETTINGS_ITEMS
}EDxDrmInternalSettingsItem;


// These are the different configurable keys in the DRM system.
// configuring the values may be done by using the DxConfigFile mechanism.

// RenewalTableXml - The path of the renewal table XML file.
#define DX_CONFIG_KEY_RENEWAL_TABLE_XML_PATH			"RenewalTableXml"

// CodeHash - For debug. Binary string (Hex representation) of the running library's code hash
// e.g. CodeHash = 00110022003300440055006600770088009900aa
#define DX_CONFIG_KEY_CODE_HASH							"CodeHash"

// In debug mode the allows setting a arbitrary master key for the secureAsset mechanism
// e.g. MasterKey = F355EA4C4EDF5156E359C16F81710AEE
#define DX_CONFIG_KEY_MASTER_KEY						"MasterKey"

// CURL opertaion timeout in seconds
#define DX_CONFIG_KEY_CURL_TRANSFER_OP_TIMEOUT          "CurlTransferOpTimeout"


// These definitions correspond with the EDxTextAttribute defined in DxDrmDefines.h
#define DX_ATTR_NAME_CONTENT_ID							"Content-ID"
#define DX_ATTR_NAME_CONTENT_AUTHOR						"Content-Author"
#define DX_ATTR_NAME_CONTENT_RATING						"Content-Rating"
#define DX_ATTR_NAME_CONTENT_COPYRIGHT					"Content-Copyright"
#define DX_ATTR_NAME_CONTENT_DESCRIPTION				"Content-Description"
#define DX_ATTR_NAME_CONTENT_NAME						"Content-Name"
#define DX_ATTR_NAME_CONTENT_MIME_TYPE					"ContentMimeType"
#define DX_ATTR_NAME_RI_URL								"RI_URL"
#define DX_ATTR_NAME_SILENT_URL							"SILENT_URL"
#define DX_ATTR_NAME_CUSTOM_DATA						"CustomData"
#define DX_ATTR_NAME_USE_WMDRM_FILE_AS_PLAYREADY		"UseWMDRMFileAsPlayReady"
#define DX_ATTR_NAME_ENCR_CONTENT_START_OFFSET			"EncrStartOffset"
#define DX_ATTR_NAME_ORIGINAL_FILE_NAME			        "OriginalFileName"
#define DX_ATTR_NAME_DRM_HEADER							"DrmHeader"
#define DX_ATTR_NAME_ADDITIONAL_HEADER_1				"AdditionalHeader_1"
#define DX_ATTR_NAME_ADDITIONAL_HEADER_VALUE_1			"AdditionalHeader-Value_1"
#define DX_ATTR_NAME_ADDITIONAL_HEADER_2				"AdditionalHeader_2"
#define DX_ATTR_NAME_ADDITIONAL_HEADER_VALUE_2			"AdditionalHeader-Value_2"
#define DX_ATTR_NAME_ADDITIONAL_HEADER_3				"AdditionalHeader_3"
#define DX_ATTR_NAME_ADDITIONAL_HEADER_VALUE_3			"AdditionalHeader-Value_3"
#define DX_ATTR_NAME_ADDITIONAL_HEADER_4				"AdditionalHeader_4"
#define DX_ATTR_NAME_ADDITIONAL_HEADER_VALUE_4			"AdditionalHeader-Value_4"

#endif

