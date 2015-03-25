#ifndef I_ZSDCC_CAP_BUF_MGR
#define I_ZSDCC_CAP_BUF_MGR
//
#include <utils/String8.h>
#include <vector>
#include <list>
using namespace std;

#include <mtkcam/drv/imem_drv.h>
#include <inc/ICaptureBufHandler.h>

//
//
namespace android {
namespace NSMtkZsdCcCamAdapter {
//
class CapBuffer : public IImgBuf
{

/******************************************************************************
*   Inheritance from IMemBuf.
*******************************************************************************/
public:
    virtual int64_t             getTimestamp()              const       { return mTimestamp; }
    virtual void                setTimestamp(int64_t const timestamp)   { mTimestamp = timestamp; }
    virtual size_t              getBufSize()                const       { return mbufInfo.size; }
    virtual void*               getVirAddr()                const       { return (void*)mbufInfo.virtAddr; }
    virtual void*               getPhyAddr()                const       { return (void*)mbufInfo.phyAddr; }
    virtual const char*         getBufName()                const       { return mName; }
    virtual int                 getIonFd()                  const       { return mbufInfo.memID; }


/******************************************************************************
*   Inheritance from IImgBuf.
*******************************************************************************/
public:

   //pass from construction
    virtual uint32_t            getImgWidth()               const       { return mWidth;  }
    virtual uint32_t            getImgHeight()              const       { return mHeight; }
    virtual String8 const&      getImgFormat()              const       { return ms8format; }

    //query by camera format
    virtual uint32_t            getImgWidthStride(uint_t const uPlaneIndex = 0) const
    {
        return FmtUtils::queryImgWidthStride(ms8format.string(), getImgWidth(), uPlaneIndex);
    }
    virtual uint32_t            getBitsPerPixel()           const       { return mBpp; }

/******************************************************************************
*   Initialization.
*******************************************************************************/
public:
    CapBuffer(      uint32_t _w = 0,
                    uint32_t _h = 0,
                    const char* _format = "",
                    const char* _name = "")
        : IImgBuf()
        , mWidth(_w)
        , mHeight(_h)
        , ms8format(String8(_format))
        , mBpp(FmtUtils::queryBitsPerPixel(_format))
        , mbufSize(FmtUtils::queryImgBufferSize(_format, mWidth, mHeight))
        , mTimestamp(0)
        , mName(_name)
        , mpIMemDrv(NULL)
        , mRotation(0)
    {
        createBuffer();
    }

    virtual                     ~CapBuffer()                             {  destroyBuffer(); }
    bool                        operator!()                 const       { return mbufInfo.virtAddr != 0; }
    void                update(uint32_t const u4Width,uint32_t const u4Height, uint32_t const u4Rotate);

/******************************************************************************
*   Initialization.
*******************************************************************************/
private:
    virtual void                createBuffer();
    virtual void                destroyBuffer();

private:
    uint32_t mWidth;
    uint32_t mHeight;
    String8  ms8format;
    uint32_t mBpp;
    uint32_t mbufSize;
    uint32_t mRotation;
    int64_t  mTimestamp;
    const char* mName;
    IMemDrv *mpIMemDrv;
    IMEM_BUF_INFO mbufInfo;
};

/******************************************************************************
 *
 ******************************************************************************/
class ICaptureBufMgr : public ICaptureBufMgrHandler
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPreviewCmdQueThreadHandler Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    virtual bool            dequeProvider(CapBufQueNode& rNode) = 0;
    virtual bool            dequeProvider(list<CapBufQueNode>* pvNode) = 0;
    virtual bool            enqueProvider(CapBufQueNode& rNode, bool bIsFilled) = 0;
    virtual bool            enqueProvider(unsigned int va, bool bIsFilled) = 0;
    virtual bool            dequeProcessor(CapBufQueNode& rNode, int index) = 0;
    virtual bool            enqueProcessor(CapBufQueNode& rNode) = 0;



    virtual int32_t         getStoredBufferCnt() = 0;
    virtual void            setStoredBufferCnt(int32_t const cnt) = 0;

public:
    virtual void            allocBuffer(int w, int h, const char* format, int rotation,
                                        int _w, int _h, const char* _format, int cnt) = 0;
    virtual void            reallocBuffer(int w, int h, const char* format,
                                           int _w, int _h, const char* _format, int cnt) = 0;

    virtual void            freeBuffer() = 0;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    static ICaptureBufMgr*  createInstance();
    virtual void            destroyInstance() = 0;
    virtual                 ~ICaptureBufMgr(){};
};

};  // namespace NSMtkZsdCcCamAdapter
};  // namespace android

#endif
