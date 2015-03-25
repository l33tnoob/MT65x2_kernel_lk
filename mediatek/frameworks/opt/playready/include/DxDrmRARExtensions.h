#ifndef DX_DRM_RAR_EXTENSIONS_H
#define DX_DRM_RAR_EXTENSIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "DxDrmStream.h"
#include "DxDrmFile.h"

//! \file DxDrmRARExtensions.h
//! \brief This file is an extension for DxDrmFile and DxDrmStream APIs.
//! It allows decrypting content into the RAR protected area.
    
//! The DxDrmStream_ProcessPacketRAR() function receives trackId and the packet data. It decrypts the packet and updates its new size.
//! \param[in]     drmStream	     A handle previously created by DxDrmClient_OpenDrmStream()
//! \param[in]	   trackId           Id of the track in the stream that is to be decrypted.
//! \param[in]	   packetData        Points to the encrypted packet data.
//! \param[in]	   packetSize        Size of packetData.
//! \param[in]     rarHandle         A RAR handle pointing to the RAR buffer allocated to hold the decrypted data.
//! \param[in/out] rarBufferSize     Size of the RAR buffer given in rarHandle. On exit will hold the size of the data decrypted to the RAR buffer.
//!	                                 (might be smaller than the RAR buffer size).
//! \param[in]     packetHeader      A buffer to hold decrypted data from the beginning of the packet. This data is returned only for video packets.
//!									 The amount of data returned is determined by the video codec. If the application does not need this information 
//!									 it may pass NULL in this parameter.
//! \param[in/out] packetHeaderSize  Size of the data buffer given in packetHeader. On exit will hold the size of the size of the actual data that was returned.
//!	                                 (might be smaller than the buffer size provided).

//! \return
//! - DX_SUCCESS - On Success
//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
//! - DX_ERROR_NO_RIGHTS - If there are no rights for the active content.
//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - If the rights for the active content are time based and the secure clock is not set.
//! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
//! - DX_ERROR_BAD_ARGUMENTS - If invalid arguments were passed.
//! - DX_NOT_AVAILABLE - Track or AUFormatBox was not found.
//! - DX_ERROR_INVALID_FORMAT - Packet data has invalid format.
DX_FUNC DRM_C_API EDxDrmStatus DxDrmStream_ProcessPacketRAR(HDxDrmStream drmStream, DxUint32 trackId, const void* packetData, DxUint32 packetSize, void* rarHandle, DxUint32* rarBufferSize, void* packetHeader, DxUint32* packetHeaderSize);


#ifdef __cplusplus
}
#endif

#endif

