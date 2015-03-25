#ifndef IDX_DRM_CORE_STREAM_H
#define IDX_DRM_CORE_STREAM_H

#include "IDxDrmCoreContent.h"

//! IDxDrmCoreStream represents an open DRM stream (such as PDCF). A DRM stream may include several contents (known as tracks).
//! By default the first content is the active content. The active content may be changed
//! using SetActiveContent() and SetActiveContentByTrackId(). Non-protected tracks are ignored by the IDxDrmCoreStream object.
//! The IDxDrmCoreStream provides the following features:
//! - Extraction of content related attributes such as:  Title, Author, Description, etc. 
//! - Query about DRM status of the content (i.e. Is the content encrypted? Do rights exists?)
//! - Decryption of packets.
//! - Consumption of content rights.
//! - Extraction of content's right information.
class IDxDrmCoreStream : public IDxDrmCoreContent
{
public:
    IDxDrmCoreStream();
    virtual ~IDxDrmCoreStream();

	virtual EDxDrmContentType GetDRMContentType()	{return DX_CONTENT_TYPE_DRM_STREAM;	}

	//! The ProcessPacket() function receives trackId and the packet data. It decrypts the packet and updates its new size.
	//! \param[in]	   trackId           Id of the track in the stream that is to be decrypted.
	//! \param[in]	   packetData        Points to the encrypted packet data.
	//! \param[in]	   packetSize        Size of packetData.
	//! \param[in]     decryptedPacket   A pointer to an allocated buffer that will be filled with the decrypted data.
	//!                                  Can be the same address as packetData.
	//! \param[in/out] outputPacketSize  Size of the decryptedPacket buffer. On exit will hold the size of the decrypted data
	//!	                                 (might be smaller than the encrypted data).
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
	virtual EDxDrmStatus ProcessPacket(
		DxUint32 trackId,
		const void* packetData,
		DxUint32 packetSize,
		void* decryptedPacket,
		DxUint32& outputPacketSize) = 0;
	
	virtual EDxDrmStatus ProcessPiffPacket(
		void* sampleData, DxUint32 sampleDataSize, 
		const void* sampleEncryptBoxData,  DxUint32 sampleEcryptBoxDataSize,
		DxUint32 sampleIndex,  DxUint32 trackId) = 0;

	//! The ProcessPayload() function receives the trackId and the payload data. It decrypts the payload in place.
	//! \param[in]	   trackId           Id of the track in the stream that is to be decrypted.
	//!									 This has importance if each track is encrypted by a different key.
	//! \param[in]	   iv		         Initial vector required for decryption.
	//! \param[in]	   ivSize	         Size of the initial vector. For PlayReady it should be 64 bits.
	//! \param[in]	   blockOffset	     Block offset within the payload (16 bytes per block).
	//! \param[in]	   byteOffset        Byte offset within the last payload block.
	//! \param[in]	   payloadData       Points to the encrypted payload data. Decryption is done in-place.
	//! \param[in]	   payloadDataSize   Size of the payloadData.
	//! \param[out]	   decryptedPayloadSize	 On exit will hold the size of the decrypted data.
	//!	                                 (might be smaller than the encrypted payload size in case of padding).
	//! \param[in]	   last15bytes		 Last 15 bytes of the payload required for WMDRM payload decryption.
	//! \param[in]	   last15bytesSize	 Size of the last 15 bytes (which might be less than 15 in small payloads).
	//! \param[in]	   paddingScheme     Padding scheme used for encryption. If null then no padding is assumed.
	//! \return
	//! - DX_SUCCESS - On Success
	//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
	//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
	//! - DX_ERROR_NO_RIGHTS - If there are no rights for the active content.
	//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - If the rights for the active content are time based and the secure clock is not set.
	//! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
	//! - DX_ERROR_BAD_ARGUMENTS - If invalid arguments were passed.
	virtual EDxDrmStatus ProcessPayload(
		DxUint32 trackId,
		const void* iv, DxUint32 ivSize,
		DxUint32 blockOffset,
		DxUint32 byteOffset,
		void* payloadData, DxUint32 payloadDataSize,
		DxUint32* decryptedPayloadSize,
		const void* last15bytes,
		DxUint32 last15bytesSize,
		const DxChar* paddingScheme ) = 0;

	//! The ProcessPayloadEx() function receives the trackId and the payload data. It decrypts the payload in place.
	//! \param[in]	   trackId           Id of the track in the stream that is to be decrypted.
	//!									 This has importance if each track is encrypted by a different key.
	//! \param[in]	   iv		         Initial vector required for decryption.
	//! \param[in]	   ivSize	         Size of the initial vector. For PlayReady it should be 64 bits.
	//! \param[in]	   blockOffset	     Block offset within the payload (16 bytes per block).
	//! \param[in]	   byteOffset        Byte offset within the last payload block.
	//! \param[in]	   payloadData       Points to the encrypted payload data. 
	//! \param[in]	   payloadDataSize   Size of the payloadData.
	//! \param[out]	   decryptedPayloadData  On exit will hold the decrypted data.
	//! \param[out]	   decryptedPayloadSize	 On exit will hold the size of the decrypted data.
	//!	                                 (might be smaller than the encrypted payload size in case of padding).
	//! \param[in]	   last15bytes		 Last 15 bytes of the payload required for WMDRM payload decryption.
	//! \param[in]	   last15bytesSize	 Size of the last 15 bytes (which might be less than 15 in small payloads).
	//! \param[in]	   paddingScheme     Padding scheme used for encryption. If null then no padding is assumed.
	//! \return
	//! - DX_SUCCESS - On Success
	//! - DX_ERROR_SECURE_STORAGE_IS_CORRUPTED - If secure storage is used and found to be corrupted.
	//! - DX_ERROR_IPC_FAILURE - Communication with other DRM agent component could not be established.
	//! - DX_ERROR_NO_RIGHTS - If there are no rights for the active content.
	//! - DX_ERROR_SECURE_CLOCK_IS_NOT_SET - If the rights for the active content are time based and the secure clock is not set.
	//! - DX_ERROR_INTENT_WAS_NOT_SET - If the function is called before the intent was set.
	//! - DX_ERROR_BAD_ARGUMENTS - If invalid arguments were passed.
	virtual EDxDrmStatus ProcessPayloadEx(
		DxUint32 trackId,
		const void* iv, DxUint32 ivSize,
		DxUint32 blockOffset,
		DxUint32 byteOffset,
		const void* payloadData, DxUint32 payloadDataSize,
		void* decryptedPayloadData, DxUint32* decryptedPayloadSize,
		const void* last15bytes,
		DxUint32 last15bytesSize,
		const DxChar* paddingScheme,
		DxBool isSecureBuffer ) = 0;

	virtual EDxDrmStatus ProcessPacketRAR(DxUint32 trackId,
										const void* packetData,
										DxUint32 packetSize,
										void* rarDecryptedPacket,
										DxUint32& outputPacketSize,
										void* codecDecryptedPacket,
										DxUint32& codecOutputPacketSize
#ifdef USE_RAR
										) = 0;
#else
										)
	{   trackId; packetData; packetSize;
            rarDecryptedPacket; outputPacketSize; 
            codecDecryptedPacket; codecOutputPacketSize; 
            return DX_ERROR_NOT_SUPPORTED;
        }
#endif


	//! Change the active content of the stream to the content which matches the given trackId.
	//! \param[in] trackId			A unique ID which represents a track in the stream object.
	//! \return
	//! - DX_SUCCESS - Operation succeeded.
	//! - DX_ERROR_BAD_ARGUMENTS - If contentIndex is not a valid content index, or drmStream is null.
	virtual EDxDrmStatus SetActiveContentByTrackId(DxUint32 trackId) = 0;

};

#endif

