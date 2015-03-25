#ifndef __DP_FRAG_STREAM_H__
#define __DP_FRAG_STREAM_H__

#include "DpDataType.h"

class DpStream;
class DpChannel;
class DpRingBufferPool;
class DpBasicBufferPool;

class DpFragStream
{
public:
    DpFragStream();

    ~DpFragStream();

    /**
     * Set source image information
     * Parameters
     * format   : source image format
     * width    : source image width
     * height   : source image height
     * pitch    : source image pitch
     */
    DP_STATUS_ENUM setSrcConfig(DpColorFormat format,
                                int32_t       width,
                                int32_t       height,
                                int32_t 	  MCUXSize,
                                int32_t 	  MCUYSize,                                   
                                int32_t       pitch,
								DpRect       *pROI = 0);


    DP_STATUS_ENUM setDstBuffer(void     **pAddrList,
                                uint32_t *pSizeList,
                                uint32_t planeNumber);

    /**
     * Set target image information
     * Parameters
     * format   : target image format
     * width    : target image width
     * height   : target image height
     * pitch    : target image pitch
     */
    DP_STATUS_ENUM setDstConfig(DpColorFormat format,
                                int32_t       width,
                                int32_t       height,
                                int32_t       pitch);

    /**
     * Start fragment processing
     */
    DP_STATUS_ENUM startFrag(uint32_t* pMCUYCount,bool bShrpEnabled);

    /**
     * Query fragment information
     * Parameters:
     * pBufID   : The buffer identifier of the fragment
     * pFormat  : The required fragment buffer format
     * **pBase  : Base address depends on the plane count
     * pXStart  : The required X source start for decoder
     * pYStart  : The required Y source start for decoder
     * pWidth   : The required source width for decoder
     * pHeight  : The required source height for decoder
     * pPitch   : The required source pitch for decoder
     * waitBuf  : Wait for the fragment information ready
     */
    DP_STATUS_ENUM dequeueFrag(int32_t       *pBufID,
                               DpColorFormat *pFormat,
                               void          **pBase,
                               int32_t       *pMCUXStart,
                               int32_t       *pMCUYStart,
                               int32_t       *pWidth,
                               int32_t       *pHeight,
                               int32_t       *pPitch,
                               bool          waitBuf = true);

    /**
     * Set fragment information
     * Parameters:
     * The specified fragment identifier
     */
    DP_STATUS_ENUM queueFrag(int32_t bufID);

    /**
     * Stop the fragment processing
     */
    DP_STATUS_ENUM stopFrag();

    DP_STATUS_ENUM setDither(bool enDither) 
    {
        m_ditherStatus = enDither;

        return DP_STATUS_RETURN_SUCCESS;
    }

    DP_STATUS_ENUM setMcuCol(int32_t mcu_col) 
    {
        m_MCU_col = mcu_col;

        return DP_STATUS_RETURN_SUCCESS;
    }
	
    DP_STATUS_ENUM setEmbeddJPEG(bool enEmbeddedJPEG = false) 
    {
        m_embeddedJPEG = enEmbeddedJPEG;

        return DP_STATUS_RETURN_SUCCESS;
    }


private:
    DpStream          *m_pStream;
    DpChannel         *m_pChannel;
    int32_t           m_channelID;
    DpRingBufferPool  *m_pSrcPool;
    DpBasicBufferPool *m_pDstPool;
    DpColorFormat     m_srcFormat;
    int32_t           m_MCUYSize;
	int32_t           m_MCUXSize;
    int32_t           m_srcWidth;
    int32_t           m_srcHeight;
    int32_t           m_srcYPitch;
    int32_t           m_srcUVPitch;
    int32_t           m_dstBufID;
    DpColorFormat     m_dstFormat;
    int32_t           m_dstWidth;
    int32_t           m_dstHeight;
    int32_t           m_dstYPitch;
    int32_t           m_dstUVPitch;
    int32_t           m_MCU_col;
	DpRect            m_cropInfo;
	bool              m_ditherStatus;
    bool              m_streamStart;
	bool              m_embeddedJPEG;
};

#endif  // __DP_FRAG_STREAM_H__
