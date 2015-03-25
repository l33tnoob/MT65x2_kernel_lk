#ifndef __DP_MULTI_STREAM_H__
#define __DP_MULTI_STREAM_H__

#include "DpDataType.h"
#include <vector>
using namespace std;

class DpStream;
class DpChannel;
class DpBasicBufferPool;

class DpMultiStream
{
public:
    DpMultiStream();

    ~DpMultiStream();

    // VA address only interface, MVA address will be allocated by Dp
    DP_STATUS_ENUM setSrcBuffer(void**   pVAddrList, 
                                uint32_t *pSizeList, 
                                uint32_t planeNumber);

    // VA + MVA address interface
    DP_STATUS_ENUM setSrcBuffer(void**   pVAddrList, 
                                void**   pMVAddrList, 
                                uint32_t *pSizeList, 
                                uint32_t planeNumber);

    // ION fd interface
    DP_STATUS_ENUM setSrcBuffer(int32_t   fileDesc, 
                                uint32_t *sizeList, 
                                uint32_t planeNumber);

    /**
     * Set source image information
     * Parameters
     * width    : source image width
     * height   : source image height
     * format   : source image format
     * ROI       : source image ROI
     * pitch     : source image pitch
     */
    DP_STATUS_ENUM setSrcConfig(int32_t         width, 
                                int32_t         height, 
                                int32_t         yPitch,
                                int32_t         uvPitch,
                                DpColorFormat   format, 
                                DP_PROFILE_ENUM profile = DP_PROFILE_BT601, 
                                DpRect*         ROI = 0);

    // VA address only interface, MVA address will be allocated by Dp
    DP_STATUS_ENUM setDstBuffer(void**   pVAddrList, 
                                uint32_t *pSizeList, 
                                uint32_t planeNumber, 
                                uint32_t port = 0);

    // VA + MVA address interface
    DP_STATUS_ENUM setDstBuffer(void**   pVAddrList, 
                                void**   pMVAddrList,
                                uint32_t *pSizeList, 
                                uint32_t planeNumber, 
                                uint32_t port = 0);

    // ION fd interface
    DP_STATUS_ENUM setDstBuffer(int32_t fileDesc, 
                                uint32_t *sizeList, 
                                uint32_t planeNumber, 
                                uint32_t port = 0);

    /**
     * Set source image information
     * Parameters
     * width    : destination image width
     * height   : destination image height
     * format   : destination image format
     * port      : destination port to configure
     * ROI       : destination image ROI
     * pitch     : destination image pitch
     */
    DP_STATUS_ENUM setDstConfig(int32_t         width, 
                                int32_t         height, 
                                int32_t         yPitch,
                                int32_t         uvPitch,
                                DpColorFormat   format, 
                                DP_PROFILE_ENUM profile = DP_PROFILE_BT601,
                                DpRect*         ROI = 0,
                                int32_t         port = 0); 

    DP_STATUS_ENUM setRotate(int32_t rot, 
                             uint32_t port = 0);

    DP_STATUS_ENUM setFlip(bool flip, 
                           uint32_t port = 0);

    DP_STATUS_ENUM setTDSHP(bool enTDSHP, 
                            uint32_t port = 0);

    DP_STATUS_ENUM setDither(bool enDither, 
                           uint32_t port = 0);

    /**
      * Start MultiStream thread if need and trigger frame process
      */
    DP_STATUS_ENUM invalidate();

    /**
      * Stop MultiStream thread, call this API when user wants to terminate the process
      */
    DP_STATUS_ENUM stop();

private:

    DpStream                        *m_pStream;
    DpChannel                       *m_pChannel;
    int32_t                         m_channelID;
    bool                            m_frameChange;
    bool                            m_cropChange;
    bool                            m_streamStart;
    DpBasicBufferPool               *m_pSrcPool;
    DpColorFormat                   m_srcFormat;
    DP_PROFILE_ENUM                 m_srcProfile;
    DpRect                          m_srcROI;
    int32_t                         m_srcWidth;
    int32_t                         m_srcHeight;
    int32_t                         m_srcYPitch;
    int32_t                         m_srcUVPitch;
    int32_t                         m_srcBuffer;
    int32_t                         m_dstCurPortEn;
    int32_t                         m_dstPortEn;
    // use vector to support dynamic output port
    std::vector<DpBasicBufferPool*> m_pDstPool;
    std::vector<DpColorFormat>      m_dstFormat;
    std::vector<DP_PROFILE_ENUM>    m_dstProfile;
    std::vector<DpRect>             m_dstROI;
    std::vector<int32_t>            m_dstWidth;
    std::vector<int32_t>            m_dstHeight;
    std::vector<int32_t>            m_dstYPitch;
    std::vector<int32_t>            m_dstUVPitch;
    std::vector<int32_t>            m_rotation;
    std::vector<bool>               m_flipStatus;
    std::vector<bool>               m_TDSHPStatus;
    std::vector<bool>               m_ditherStatus;
    std::vector<int32_t>            m_dstBuffer;

};

#endif  // __DP_BLIT_STREAM_H__
