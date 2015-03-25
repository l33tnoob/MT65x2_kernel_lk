#ifndef EXT_IMG_PROC_H
#define EXT_IMG_PROC_H
//-----------------------------------------------------------------------------
class ExtImgProc : public IExtImgProc
{
    public:
        typedef enum
        {
            BufType_Display     = 0x00000001,
            BufType_PreviewCB   = 0x00000002,
            BufType_Record      = 0x00000004
        }BufTypeEnum;
    //
    protected:
        virtual ~ExtImgProc() {};
    //
    public:
        static ExtImgProc*  createInstance(void);
        virtual void        destroyInstance(void) = 0;
        //
        virtual MBOOL       init(void) = 0;
        virtual MBOOL       uninit(void) = 0;
        virtual MUINT32     getImgMask(void) = 0;
        virtual MBOOL       doImgProc(ImgInfo& img) = 0;
};
//-----------------------------------------------------------------------------
#endif

