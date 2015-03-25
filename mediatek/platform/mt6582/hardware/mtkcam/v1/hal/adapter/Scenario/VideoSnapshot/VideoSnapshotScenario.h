#ifndef VIDEO_SNAPSHOT_SCENARIO_H
#define VIDEO_SNAPSHOT_SCENARIO_H
//-----------------------------------------------------------------------------
class VideoSnapshotScenario :   public IVideoSnapshotScenario,
                                        public Thread 
{
    public:
        typedef enum
        {
            JpgType_Img,
            JpgType_Main,
            JpgType_Thumb,
            JpgType_ThumbRotate,
            JpgType_Amount
        }JpgTypeEnum;
        //
        typedef enum
        {
            MemType_Pass1Out,
            MemType_Jpg,
            MemType_JpgThumb,
            MemType_JpgMain,
            MemType_YuvThumb,
            MemType_YuvThumbRotate,
            MemType_YuvMain,
            MemType_Amount
        }MemTypeEnum;
        //
        typedef enum
        {
            ProcStep_Idle,
            ProcStep_Main_Thumb_Init,
            ProcStep_Main_Thumb,
            ProcStep_ThumbRotate_Init,
            ProcStep_ThumbRotate,
            ProcStep_HandleJpg
        }ProcStepEnum;
        //
        typedef struct
        {
            MUINT32 width;
            MUINT32 height;
            MUINT32 bitStrSize;
        }JpgInfo;
    //
    private:
        VideoSnapshotScenario();
        virtual ~VideoSnapshotScenario();
    //
    public:
        static IVideoSnapshotScenario* getInstance(void);
        virtual void        destroyInstance(void);        
        //
        virtual MBOOL       init(
            MINT32              sensorId,
            sp<IParamsManager>  pParamsMgr,
            Hal3ABase*          p3AHal,
            ImageSize*          pImageSize);
        virtual MBOOL       uninit(void);
        virtual MBOOL       setCallback(sp<IShotCallback> pShotCallback);
        virtual MBOOL       allocMem(MemTypeEnum type);
        virtual MBOOL       freeMem(MemTypeEnum type);
        virtual Status      getStatus(void);
        virtual MBOOL       enable(MBOOL en);
        virtual MBOOL       setImage(ImageInfo &img);
        virtual MBOOL       transMainThumb(MBOOL en);
        virtual MBOOL       rotateThumb(MBOOL en);
        virtual MBOOL       encodeJpg(
            JpgTypeEnum     jpgType,
            MemTypeEnum     srcMemType,
            MemTypeEnum     dstMemType,
            MUINT32         quality,
            MBOOL           enableSOI);
        virtual MBOOL       encodeJpgMain(void);
        virtual MBOOL       encodeJpgThumb(void);
        virtual MBOOL       integrateJpg(void);
        virtual MBOOL       callbackJpg(void);
        virtual MBOOL       process(void);
        virtual void        transImg(void);
        virtual MBOOL       saveData(
            MUINT32         addr,
            MUINT32         size,
            char*           pFileName);
    //Thread class API
    private:
        virtual bool        threadLoop();
    public:
        virtual void        requestExit();
        virtual status_t    readyToRun();
    //Member variable
    private:
        #define ALIGN_SIZE(in,align)    ((in+align-1) & ~(align-1))
        #define YUV_PRE_ALLOC_WIDTH     (1920)
        #define YUV_PRE_ALLOC_HEIGHT    (1088)
        #define JPG_COMPRESSION_RATIO   (1)
        #define JPG_EXIF_SIZE           (2*1024)
        #define JPG_LOCK_TIMEOUT_CNT    (10)
        #define JPG_LOCK_TIMEOUT_SLEEP  (1000) //us
        #define JPG_IMG_ALIGN_SIZE      (16)
        #define YUV_IMG_STRIDE_Y        (16)
        #define YUV_IMG_STRIDE_U        (16)
        #define YUV_IMG_STRIDE_V        (16)
        //
        volatile MINT32     mUsers;
        volatile Status     mStatus;
        mutable Mutex       mLock;
        Condition           mCond;
        ImageInfo           mImage;
        Hal3ABase*          mp3AHal;
        SensorHal*          mpSensorHal;
        VssImgTrans*        mpVssImgTrans;
        IMemDrv*            mpIMemDrv;
        sp<IShotCallback>   mpShotCallback; 
        sp<IParamsManager>  mpParamsMgr;
        halSensorType_e     meSensorType;
        IMEM_BUF_INFO       mIMemBufInfo[MemType_Amount];
        JpgInfo             mJpgInfo[JpgType_Amount];
        MUINT32             mRotate;
        ProcStepEnum        mProcStep;
        MINT32              mThreadId;
        MINT32              mSensorId;
        MBOOL               mIsThumb;
        MUINT32             mPreAllocYuvWidth;
        MUINT32             mPreAllocYuvHeight;
        EImageFormat        mImgiFormat;
        MUINT32             mProcessCnt;
};
//-----------------------------------------------------------------------------
#endif

