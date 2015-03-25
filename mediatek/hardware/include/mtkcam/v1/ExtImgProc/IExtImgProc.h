#ifndef I_EXT_IMG_PROC_H
#define I_EXT_IMG_PROC_H
//-----------------------------------------------------------------------------
class IExtImgProc
{
    public:
        typedef struct
        {
            MUINT32         bufType;
            const char*     format;
            MUINT32         width;
            MUINT32         height;
            MUINT32         stride[3];
            MUINT32         virtAddr; 
            MUINT32         bufSize;
        }ImgInfo;
        //
        virtual ~IExtImgProc() {}
    public:
        virtual MBOOL       init(void) = 0;
        virtual MBOOL       uninit(void) = 0;
        virtual MUINT32     getImgMask(void)= 0;
        virtual MBOOL       doImgProc(ImgInfo& img)= 0;
};
//-----------------------------------------------------------------------------
#endif

