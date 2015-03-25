#ifndef PREVIEWFEATURE_BUFF_MGR_H
#define PREVIEWFEATURE_BUFF_MGR_H

/******************************************************************************
 *
 ******************************************************************************/
#include <utils/threads.h>
#include <utils/RefBase.h>
#include <mtkcam/common.h>
#include <mtkcam/drv/imem_drv.h>
//
#include <vector>
using namespace std;
//
/******************************************************************************
 *
 ******************************************************************************/
class PREVIEWFEATUREBuffer : public IImgBuf
{

/******************************************************************************
*   Inheritance from IMemBuf.
*******************************************************************************/
public:
    virtual int64_t             getTimestamp() const                    { return mTimestamp; }
    virtual void                setTimestamp(int64_t const timestamp)   { mTimestamp = timestamp; }
    virtual size_t              getBufSize() const                      { return mbufInfo.size; }
    virtual void*               getVirAddr() const                      { return (void*)mbufInfo.virtAddr; }
    virtual void*               getPhyAddr() const                      { return (void*)mbufInfo.phyAddr; }
    virtual const char*         getBufName() const                      { return mName; }              
    virtual int                 getIonFd() const                        { return mbufInfo.memID; }

    
/******************************************************************************
*   Inheritance from IImgBuf.
*******************************************************************************/
public:     
    virtual String8 const&      getImgFormat()      const               { return mformat; }
    virtual uint32_t            getImgWidthStride(
                                    uint_t const uPlaneIndex = 0
                                )   const
                                {
                                    return  FmtUtils::queryImgWidthStride(getImgFormat(), getImgWidth(), uPlaneIndex);
                                }

    virtual uint32_t            getImgWidth()       const               { return mWidth;  }
    virtual uint32_t            getImgHeight()      const               { return mHeight; }
    virtual uint32_t            getBitsPerPixel()   const               { return mBpp;    }


/******************************************************************************
*   Initialization.
*******************************************************************************/
public:
    PREVIEWFEATUREBuffer( uint32_t _w = 0, 
              uint32_t _h = 0, 
              uint32_t _bpp = 0, 
              uint32_t _bufsize = 0, 
              String8  _format = String8(""),
              const char* _name = "")
        : IImgBuf()
        , mWidth(_w)
        , mHeight(_h)
        , mBpp(_bpp)
        , mbufSize(_bufsize)
        , mTimestamp(0)
        , mformat(_format)
        , mName(_name)
        , mpIMemDrv(NULL)
    {
        createBuffer();
    }

    virtual                     ~PREVIEWFEATUREBuffer() {destroyBuffer();};
    bool                        operator!() const   { return mbufInfo.virtAddr != 0; }

/******************************************************************************
*   Initialization.
*******************************************************************************/    
private:    
    virtual void                createBuffer();   
    virtual void                destroyBuffer();    
    
private:
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mBpp;
    uint32_t mbufSize;
    int64_t  mTimestamp;
    String8  mformat;
    const char* mName;
    IMemDrv *mpIMemDrv;
    IMEM_BUF_INFO mbufInfo;
};

#endif
