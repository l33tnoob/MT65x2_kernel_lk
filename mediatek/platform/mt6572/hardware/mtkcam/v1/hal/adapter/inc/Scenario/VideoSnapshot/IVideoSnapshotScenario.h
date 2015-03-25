#ifndef I_VIDEO_SNAPSHOT_SCENARIO_H
#define I_VIDEO_SNAPSHOT_SCENARIO_H
//-----------------------------------------------------------------------------
namespace android
{
//-----------------------------------------------------------------------------
class IVideoSnapshotScenario
{
    public:
        typedef enum
        {
            Status_Idle,
            Status_WaitImage,
            Status_Process,
            Status_Done
        }Status;
        //
        typedef struct
        {
            MUINT32     width;
            MUINT32     height;
            MUINT32     stride;
        }ImageSize;
        //
        typedef struct
        {
            MUINT32     id;
            MUINT32     vir;
            MUINT32     phy;
            MUINT32     size;
        }ImageMem;
        //
        typedef struct
        {
            MUINT32     x;
            MUINT32     y;
            MUINT32     w;
            MUINT32     h;
        }ImageCrop;
        //
        typedef struct
        {
            ImageSize       size;
            ImageMem        mem;
            ImageCrop       crop;
        }ImageInfo;
    //
    protected:
        virtual ~IVideoSnapshotScenario(){};
    //
    public:
        static IVideoSnapshotScenario* createInstance(void);
        virtual void        destroyInstance(void) = 0;
        virtual MBOOL       init(
            MINT32              sensorId,
            sp<IParamsManager>  pParamsMgr,
            Hal3ABase*          p3AHal,
            ImageSize*          pImageSize)= 0;
        virtual MBOOL       uninit(void)= 0;
        virtual MBOOL       setCallback(sp<IShotCallback> pShotCallback)= 0;
        virtual Status      getStatus(void) = 0;
        virtual MBOOL       enable(MBOOL en) = 0;
        virtual MBOOL       setImage(ImageInfo &img) = 0;
        virtual MBOOL       callbackJpg(void)= 0;
        virtual MBOOL       process(void) = 0;
        virtual void        transImg(void) = 0;
        virtual MBOOL       saveData(
            MUINT32         addr,
            MUINT32         size,
            char*           pFileName) = 0;
};
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
#endif

