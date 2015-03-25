/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef VSS_IMG_TRANS_H
#define VSS_IMG_TRANS_H
//-----------------------------------------------------------------------------
class VssImgTrans
{
    protected:
        virtual ~VssImgTrans() {};
    //
    public:
        typedef struct
        {
            MUINT32             Width;
            MUINT32             Height;
            MUINT32             Stride;
        }IMG_SIZE_STRUCT;
        //
        typedef struct
        {
            MINT32              id; //ION FD
            MUINT32             Vir;
            MUINT32             Phy;
        }IMG_MEM_STRUCT;
        //
        typedef struct
        {
            MUINT32             X;
            MUINT32             Y;
            MUINT32             W;
            MUINT32             H;
        }IMG_CROP_STRUCT;
        //
        typedef struct
        {
            IMG_SIZE_STRUCT     Size;
            IMG_MEM_STRUCT      Mem;
            EImageFormat        Format;
            IMG_CROP_STRUCT     Crop;
        }IMAGE_IN_STRUCT;
        //
        typedef struct
        {
            MBOOL               Enable;
            IMG_SIZE_STRUCT     Size;
            IMG_MEM_STRUCT      Mem;
            EImageFormat        Format;
        }DISPO_OUT_STRUCT;
        //
        typedef struct
        {
            MBOOL               Enable;
            IMG_SIZE_STRUCT     Size;
            IMG_MEM_STRUCT      Mem;
            EImageFormat        Format;
            EImageRotation      Rotate;
            EImageFlip          Flip;
        }VIDO_OUT_STRUCT;
        //
        typedef struct
        {
            IMAGE_IN_STRUCT     ImageIn;
            DISPO_OUT_STRUCT    DispoOut;
            VIDO_OUT_STRUCT     VidoOut;
        }CONFIG_STRUCT;
        //
        static VssImgTrans* CreateInstance(void);
        virtual MVOID   DestroyInstance(void) = 0;
        virtual MBOOL   Init(CONFIG_STRUCT& Config, MUINT32& TpipeNum) = 0;
        virtual MBOOL   Uninit(void) = 0;
        virtual MBOOL   Start(MUINT32 TpipeIndex) = 0;
        virtual MBOOL   WaitDone(void) = 0;
};
//-----------------------------------------------------------------------------
#endif

