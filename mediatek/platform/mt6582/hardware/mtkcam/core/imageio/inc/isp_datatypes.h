
/**
* @file isp_datatypes.h
*
* isp_datatypes Header File
*/


#ifndef __ISP_DATATYPES_H__
#define __ISP_DATATYPES_H__


//Kernel Includes
#include "camera_isp.h" //For ISP Function ID,and Kernel Flag
#include "cdp_drv.h"
#include <mtkcam/drv/tpipe_drv.h>

#include <mtkcam/common.h>//SL TEST_MDP_YUV
using namespace NSCam;

/*-----------------------------------------------------------------------------
    ISP ERROR CODE
  -----------------------------------------------------------------------------*/

#define ISP_ERROR_CODE_OK                      (0)
#define ISP_ERROR_CODE_FAIL                    (-1)

#define ISP_ERROR_CODE_FAIL_00                 (-1 & ~(0x1<<0))  /*-2 , (0xFFFFFFFE)*/
#define ISP_ERROR_CODE_FAIL_01                 (-1 & ~(0x1<<1))  /*-3 , (0xFFFFFFFD)*/
#define ISP_ERROR_CODE_FAIL_02                 (-1 & ~(0x1<<2))  /*-5 , (0xFFFFFFFB)*/
#define ISP_ERROR_CODE_FAIL_03                 (-1 & ~(0x1<<3))  /*-9 , (0xFFFFFFF7)*/
#define ISP_ERROR_CODE_FAIL_04                 (-1 & ~(0x1<<4))  /*-17 , (0xFFFFFFEF)*/
#define ISP_ERROR_CODE_LOCK_RESOURCE_FAIL      (-1 & ~(0x1<<5))  /*-33 , (0xFFFFFFDF)*/
#define ISP_ERROR_CODE_FAIL_06                 (-1 & ~(0x1<<6))  /*-65 , (0xFFFFFFBF)*/
#define ISP_ERROR_CODE_FAIL_07                 (-1 & ~(0x1<<7))  /*-129 , (0xFFFFFF7F)*/
#define ISP_ERROR_CODE_FAIL_08                 (-1 & ~(0x1<<8))  /*-257 , (0xFFFFFEFF)*/
#define ISP_ERROR_CODE_FAIL_09                 (-1 & ~(0x1<<9))  /*-513 , (0xFFFFFDFF)*/
#define ISP_ERROR_CODE_FAIL_10                 (-1 & ~(0x1<<10))  /*-1025 , (0xFFFFFBFF)*/
#define ISP_ERROR_CODE_FAIL_11                 (-1 & ~(0x1<<11))  /*-2049 , (0xFFFFF7FF)*/
#define ISP_ERROR_CODE_FAIL_12                 (-1 & ~(0x1<<12))  /*-4097 , (0xFFFFEFFF)*/
#define ISP_ERROR_CODE_FAIL_13                 (-1 & ~(0x1<<13))  /*-8193 , (0xFFFFDFFF)*/
#define ISP_ERROR_CODE_FAIL_14                 (-1 & ~(0x1<<14))  /*-16385 , (0xFFFFBFFF)*/
#define ISP_ERROR_CODE_FAIL_15                 (-1 & ~(0x1<<15))  /*-32769 , (0xFFFF7FFF)*/
#define ISP_ERROR_CODE_FAIL_16                 (-1 & ~(0x1<<16))  /*-65537 , (0xFFFEFFFF)*/
#define ISP_ERROR_CODE_FAIL_17                 (-1 & ~(0x1<<17))  /*-131073 , (0xFFFDFFFF)*/
#define ISP_ERROR_CODE_FAIL_18                 (-1 & ~(0x1<<18))  /*-262145 , (0xFFFBFFFF)*/
#define ISP_ERROR_CODE_FAIL_19                 (-1 & ~(0x1<<19))  /*-524289 , (0xFFF7FFFF)*/
#define ISP_ERROR_CODE_FAIL_20                 (-1 & ~(0x1<<20))  /*-1048577 , (0xFFEFFFFF)*/
#define ISP_ERROR_CODE_FAIL_21                 (-1 & ~(0x1<<21))  /*-2097153 , (0xFFDFFFFF)*/
#define ISP_ERROR_CODE_FAIL_22                 (-1 & ~(0x1<<22))  /*-4194305 , (0xFFBFFFFF)*/
#define ISP_ERROR_CODE_FAIL_23                 (-1 & ~(0x1<<23))  /*-8388609 , (0xFF7FFFFF)*/
#define ISP_ERROR_CODE_FAIL_24                 (-1 & ~(0x1<<24))  /*-16777217 , (0xFEFFFFFF)*/
#define ISP_ERROR_CODE_FAIL_25                 (-1 & ~(0x1<<25))  /*-33554433 , (0xFDFFFFFF)*/
#define ISP_ERROR_CODE_FAIL_26                 (-1 & ~(0x1<<26))  /*-67108865 , (0xFBFFFFFF)*/
#define ISP_ERROR_CODE_FAIL_27                 (-1 & ~(0x1<<27))  /*-134217729 , (0xF7FFFFFF)*/
#define ISP_ERROR_CODE_FAIL_28                 (-1 & ~(0x1<<28))  /*-268435457 , (0xEFFFFFFF)*/
#define ISP_ERROR_CODE_FAIL_29                 (-1 & ~(0x1<<29))  /*-536870913 , (0xDFFFFFFF)*/
#define ISP_ERROR_CODE_FAIL_30                 (-1 & ~(0x1<<30))  /*-1073741825 , (0xBFFFFFFF)*/


#define ISP_IS_ERROR_CODE( _retval_ , _errorcode_ ) \
            (   _retval_ >= 0    ?    0    :    ( (( _retval_|_errorcode_)==_errorcode_) ? 1 : 0 )     )


/**
*@brief  Size struct
*/
class IspSize
{
public:
    unsigned long w;
    unsigned long h;
    unsigned long stride;

public:

    /**
      *@brief  Constructor
      */
    IspSize():
        w(0),h(0), stride(0)
        {};

    /**
      *@brief  Constructor
      */
    IspSize(unsigned long _w, unsigned long _h )
        {
            w = _w; h = _h;
        };

};

/**
*@brief  Point struct
*/
class IspPoint
{
public:
    long    x;
    long    y;

public:

    /**
      *@brief  Constructor
      */
    IspPoint():
        x(0),y(0)
        {};

    /**
      *@brief  Constructor
      */
    IspPoint(unsigned long _x, unsigned long _y )
       {
           x = _x; y = _y;
       };
};

/**
*@brief  Rectangle struct
*/
class IspRect
{
public:
    MUINT32         x;
    MUINT32         y;
    MUINT32         floatX; ///< x float precise - 32 bit 
    MUINT32         floatY; ///< y float precise - 32 bit
    unsigned long   w;
    unsigned long   h;


public:

    /**
      *@brief  Constructor
      */
    IspRect():
        x(0),y(0),floatX(0),floatY(0),w(0),h(0)
        {};

    /**
      *@brief  Constructor
      */
    IspRect(long _x, long _y, long _floatX, long _floatY, unsigned long _w, unsigned long _h )
        {
            x = _x; y = _y; floatX = _floatX; floatY = _floatY; w = _w; h = _h;
        };
};

/**
*@brief  YUV address struct
*/
class IspYuvAddr
{
public:
    unsigned long   y;
    unsigned long   u;
    unsigned long   v;

    /*User need not fill in the data below!-----------------------------------------------------------*/
    /*Below data is auto fill by ISP driver calculation!----------------------------------------------*/

    unsigned long   y_buffer_size;
    unsigned long   u_buffer_size;
    unsigned long   v_buffer_size;

public:

    /**
      *@brief  Constructor
      */
    IspYuvAddr():
        y(0), u(0), v(0),
        y_buffer_size(0), u_buffer_size(0), v_buffer_size(0)
        {};
};

/**
*@brief  Memory buffer struct
*/
class IspMemBuffer
{
public:
    MUINT32 size;
    MUINT32 base_vAddr;
    MUINT32 base_pAddr;
    MUINT32 ofst_addr;
    MUINT32 alignment;
public:

    /**
      *@brief  Constructor
      */
    IspMemBuffer():
        size(0),base_vAddr(0),base_pAddr(0),ofst_addr(0), alignment(16)
        {};
};

/**
*@brief  ISP DMA configure struct
*/
class IspDMACfg
{
public:
    IspMemBuffer    memBuf;
    IspSize         size;
    IspRect         crop;
    int             pixel_byte;
    int             swap;
    int             format_en;
    int             format;
    int             bus_size_en;
    int             bus_size;
    int             ring_en;
    int             ring_size;
    int             memBuf_c_ofst;
    int             memBuf_v_ofst;
    EImageFormat  lIspColorfmt;  //SL TEST_MDP_YUV
};

/**
*@brief  CDP rotate DMA configure struct
*/
class CdpRotDMACfg
{
public:
    IspMemBuffer    memBuf;
    IspSize         size;
    IspMemBuffer    memBuf_c;
    IspSize         size_c;
    IspMemBuffer    memBuf_v;
    IspSize         size_v;
    int             crop_en;
    IspRect         crop;
    int             pixel_byte;
    int             uv_plane_swap;
    CDP_DRV_FORMAT_ENUM         Format; /* DISPO_FORMAT_1 */
    CDP_DRV_PLANE_ENUM          Plane;  /* DISPO_FORMAT_3 */
    CDP_DRV_SEQUENCE_ENUM       Sequence;
    CDP_DRV_ROTATION_ENUM       Rotation;
    MBOOL Flip;
};

/**
*@brief  ISP ring tdri configure struct
*/
class IspRingTdriCfg
{
public:
    int isCalculateTdri;
    MUINT32 ringBufferMcuRowNo;
    MUINT32 ringBufferMcuHeight;
    MUINT32 ringConfNumVa;
    MUINT32 ringConfVerNumVa;
    MUINT32 ringErrorControlVa;
    MUINT32 ringConfBufVa;
};

/**
*@brief  ISP cap tdri configure struct
*/

class IspCapTdriCfg
{
public:
    int isCalculateTpipe;
    MUINT32 isRunSegment;
    MUINT32 setSimpleConfIdxNumVa;
    MUINT32 segSimpleConfBufVa;
};

/**
*@brief  ISP bnr configure struct
*/
class IspBnrCfg
{
public:
    int bpc_en;
	int bpc_tbl_en;
    int bpc_tbl_size;/* bad pixel table width */
};

/**
*@brief  ISP lsc configure struct
*/
class IspLscCfg
{
public:
    int sdblk_width;
    int sdblk_xnum;
    int sdblk_last_width;
    int sdblk_height;
    int sdblk_ynum;
    int sdblk_last_height;
};
/**
*@brief  ISP lce configure struct
*/
class IspLceCfg
{
public:
    int lce_bc_mag_kubnx;
    int lce_offset_x;
    int lce_bias_x;
    int lce_slm_width;
    int lce_bc_mag_kubny;
    int lce_offset_y;
    int lce_bias_y;
    int lce_slm_height;
};

/**
*@brief  ISP nbc configure struct
*/
class IspNbcCfg
{
public:
	int anr_eny;
    int anr_enc;
    int anr_iir_mode;
    int anr_scale_mode;
};

/**
*@brief  ISP seee configure struct
*/
class IspSeeeCfg
{
public:
    int se_edge;
    int usm_over_shrink_en;
};

/**
*@brief  ISP imgo configure struct
*/
class IspImgoCfg
{
public:
    int imgo_stride;
    int imgo_crop_en;
};

/**
*@brief  ISP esfko configure struct
*/
class IspEsfkoCfg
{
public:
    int esfko_stride;
};

/**
*@brief  ISP aao configure struct
*/
class IspAaoCfg
{
public:
    int aao_stride;
};

/**
*@brief  ISP lcso configure struct
*/
class IspLcsoCfg
{
public:
    int lcso_stride;
    int lcso_crop_en;
};

/**
*@brief  ISP cdrz configure struct
*/
class IspCdrzCfg
{
public:
    int cdrz_input_crop_width;
    int cdrz_input_crop_height;
    int cdrz_output_width;
    int cdrz_output_height;
    int cdrz_horizontal_integer_offset;/* pixel base */
    int cdrz_horizontal_subpixel_offset;/* 20 bits base */
    int cdrz_vertical_integer_offset;/* pixel base */
    int cdrz_vertical_subpixel_offset;/* 20 bits base */
    int cdrz_horizontal_luma_algorithm;
    int cdrz_vertical_luma_algorithm;
    int cdrz_horizontal_coeff_step;
    int cdrz_vertical_coeff_step;
};

/**
*@brief  ISP curz configure struct
*/
class IspCurzCfg
{
public:
    int curz_input_crop_width;
    int curz_input_crop_height;
    int curz_output_width;
    int curz_output_height;
    int curz_horizontal_integer_offset;/* pixel base */
    int curz_horizontal_subpixel_offset;/* 20 bits base */
    int curz_vertical_integer_offset;/* pixel base */
    int curz_vertical_subpixel_offset;/* 20 bits base */
    int curz_horizontal_coeff_step;
    int curz_vertical_coeff_step;
};

/**
*@brief  ISP fe configure struct
*/
class IspFeCfg
{
public:
    int fem_harris_tpipe_mode;
};

/**
*@brief  ISP img2o configure struct
*/
class IspImg2oCfg
{
public:
    int img2o_stride;
    int img2o_crop_en;
};

/**
*@brief  ISP prz configure struct
*/
class IspPrzCfg
{
public:
    int prz_output_width;
    int prz_output_height;
    int prz_horizontal_integer_offset;/* pixel base */
    int prz_horizontal_subpixel_offset;/* 20 bits base */
    int prz_vertical_integer_offset;/* pixel base */
    int prz_vertical_subpixel_offset;/* 20 bits base */
    int prz_horizontal_luma_algorithm;
    int prz_vertical_luma_algorithm;
    int prz_horizontal_coeff_step;
    int prz_vertical_coeff_step;
};

/**
*@brief  ISP mfb configure struct
*/
class IspMfbCfg
{
public:
    int bld_mode;
    int bld_deblock_en;
};

/**
*@brief  ISP flki configure struct
*/
class IspFlkiCfg
{
public:
    int flki_stride;
};

/**
*@brief  ISP cfa configure struct
*/
class IspCfaCfg
{
public:
    int bayer_bypass;
};

/**
*@brief  ISP sl2 configure struct
*/
class IspSl2Cfg
{
	public:
		  int sl2_hrz_comp;
};

/**
*@brief  ISP top configure struct
*/
class IspTopCfg
{
public:
    unsigned int enable1;
    unsigned int enable2;
    unsigned int dma;
};


// tdri update flag
#define TPIPE_UPDATE_BNR         TPIPE_DRV_UPDATE_BNR
#define TPIPE_UPDATE_LSC         TPIPE_DRV_UPDATE_LSC
#define TPIPE_UPDATE_MFB         TPIPE_DRV_UPDATE_MFB
#define TPIPE_UPDATE_CFA         TPIPE_DRV_UPDATE_CFA
#define TPIPE_UPDATE_NBC         TPIPE_DRV_UPDATE_NBC
#define TPIPE_UPDATE_SEEE        TPIPE_DRV_UPDATE_SEEE
#define TPIPE_UPDATE_LCE         TPIPE_DRV_UPDATE_LCE
#define TPIPE_UPDATE_NR3D        TPIPE_DRV_UPDATE_NR3D

//
#define TPIPE_UPDATE_IMGI        TPIPE_DRV_UPDATE_IMGI
#define TPIPE_UPDATE_IMGCI       TPIPE_DRV_UPDATE_IMGCI
#define TPIPE_UPDATE_VIPI        TPIPE_DRV_UPDATE_VIPI
#define TPIPE_UPDATE_VIP2I       TPIPE_DRV_UPDATE_VIP2I
#define TPIPE_UPDATE_FLKI        TPIPE_DRV_UPDATE_FLKI
#define TPIPE_UPDATE_LCEI        TPIPE_DRV_UPDATE_LCEI
#define TPIPE_UPDATE_LSCI        TPIPE_DRV_UPDATE_LSCI
#define TPIPE_UPDATE_IMGO        TPIPE_DRV_UPDATE_IMGO
#define TPIPE_UPDATE_IMG2O       TPIPE_DRV_UPDATE_IMG2O
#define TPIPE_UPDATE_ESFKO       TPIPE_DRV_UPDATE_ESFKO
#define TPIPE_UPDATE_AAO         TPIPE_DRV_UPDATE_AAO
#define TPIPE_UPDATE_LCSO        TPIPE_DRV_UPDATE_LCSO
#define TPIPE_UPDATE_VIDO        TPIPE_DRV_UPDATE_VIDO
#define TPIPE_UPDATE_DISPO       TPIPE_DRV_UPDATE_DISPO
#define TPIPE_UPDATE_MAX_NUM     TPIPE_DRV_UPDATE_MAX_NUM


// tdri update type
#define TPIPE_UPDATE_TYPE_CQ1_FULL_SAVE      TPIPE_DRV_UPDATE_TYPE_CQ1_FULL_SAVE      // fully update and save tdri parameter
#define TPIPE_UPDATE_TYPE_CQ1_PARTIAL_SAVE   TPIPE_DRV_UPDATE_TYPE_CQ1_PARTIAL_SAVE   // partial update and save tdri parameter
#define TPIPE_UPDATE_TYPE_CQ1_TURNING_SAVE   TPIPE_DRV_UPDATE_TYPE_CQ1_TURNING_SAVE   // partial update for turning path
#define TPIPE_UPDATE_TYPE_CQ2_FULL_SAVE      TPIPE_DRV_UPDATE_TYPE_CQ2_FULL_SAVE      // fully update and save tdri parameter
#define TPIPE_UPDATE_TYPE_CQ2_PARTIAL_SAVE   TPIPE_DRV_UPDATE_TYPE_CQ2_PARTIAL_SAVE   // partial update and save tdri parameter
#define TPIPE_UPDATE_TYPE_CQ2_TURNING_SAVE   TPIPE_DRV_UPDATE_TYPE_CQ2_TURNING_SAVE   // partial update for turning path
#define TPIPE_UPDATE_TYPE_CQ3_FULL_SAVE      TPIPE_DRV_UPDATE_TYPE_CQ3_FULL_SAVE      // fully update and save tdri parameter
#define TPIPE_UPDATE_TYPE_CQ3_PARTIAL_SAVE   TPIPE_DRV_UPDATE_TYPE_CQ3_PARTIAL_SAVE   // partial update and save tdri parameter
#define TPIPE_UPDATE_TYPE_CQ3_TURNING_SAVE   TPIPE_DRV_UPDATE_TYPE_CQ3_TURNING_SAVE   // partial update for turning path
#define TPIPE_UPDATE_TYPE_FULL               TPIPE_DRV_UPDATE_TYPE_FULL               // fully update and do no save tdri parameter (for jpeg)
#define TPIPE_UPDATE_TYPE                    TPIPE_DRV_UPDATE_TYPE      // enum

/**
*@brief  ISP tdri update configure struct
*/
class IspTdriUpdateCfg
{
public:
    TPIPE_UPDATE_TYPE updateType;
    int   partUpdateFlag;
};

/**
*@brief  ISP tdri configure struct
*/
class IspTdriCfg
{
public:
    //enable table
    IspTdriUpdateCfg updateTdri;
    IspTopCfg   en_Top;

    /*===DMA===*/
    IspDMACfg imgi;
    IspDMACfg vipi;
    IspDMACfg vip2i;
    IspDMACfg imgci;
    IspDMACfg lcei;
    IspDMACfg lsci;
    CdpRotDMACfg dispo;
    CdpRotDMACfg vido;

    IspRingTdriCfg ringTdriCfg;
    IspCapTdriCfg capTdriCfg;
    IspBnrCfg bnr;
    IspLscCfg lsc;
    IspLceCfg lce;
    IspNbcCfg nbc;
    IspSeeeCfg seee;
    IspImgoCfg imgo;
    IspEsfkoCfg esfko;
    IspAaoCfg aao;
    IspLcsoCfg lcso;
    IspCdrzCfg cdrz;
    IspCurzCfg curz;
    IspFeCfg fe;
    IspImg2oCfg img2o;
    IspPrzCfg prz;
    IspMfbCfg mfb;
    IspFlkiCfg flki;
    IspCfaCfg cfa;
	IspSl2Cfg sl2;
};

typedef int (*pfCBFuncCfg)(void);

/**
*@brief  ISP color format enum
*/
typedef enum IspColorFormat
{
    /*-----------------------------------------------------------------------------
        RAW
      -----------------------------------------------------------------------------*/
    RAW8 = 0,
    RAW10,
    RAW12,
    YUV422_1P,
    YUV422_2P,
    YUV422_3P,
    YUV422_Y,   // 2 or 3 plane
    YUV422_UV,  // 2 plane
    YUV422_Cb,  // 3 plane
    YUV422_Cr,  // 3 plane
    YUV420_3P,
    YUV420_Y,   // 2 or 3 plane
    YUV420_UV,  // 2 plane
    YUV420_Cb,  // 3 plane
    YUV420_Cr,  // 3 plane
    JPG8,

    /*-----------------------------------------------------------------------------
        RGB
      -----------------------------------------------------------------------------*/
    RGB888,
    BGR888,
    RGB565,
    BGR565,
    ABGR8888,
    ARGB8888,
    BGRA8888,
    RGBA8888,

    /*-----------------------------------------------------------------------------
        YUV Packed
      -----------------------------------------------------------------------------*/
    //UYVY = YUV 4:2:2 (Y sample at every pixel, U and V sampled at every second pixel horizontally on each line). A macropixel contains 2 pixels in 1 u_int32
    UYVY_Pack,
    YUV422_Pack     = UYVY_Pack,

    //YUYV = YUY2 = YUV 4:2:2 as for UYVY but with different component ordering within the u_int32 macropixel.
    YUYV_Pack,
    YUY2_Pack       = YUYV_Pack,
    YUV422_2_Pack   = YUYV_Pack,

    //YUV 4:1:1 with a packed, 6 byte/4 pixel macroblock structure.
    Y411_Pack,

    /*-----------------------------------------------------------------------------
        YUV Plane
      -----------------------------------------------------------------------------*/
    //YUV422, 2x1 subsampled U/V planes,only ROTDMA0
    YV16_Planar,
    YUV422_Planar   = YV16_Planar,

    //YUV420, 2x2 subsampled U/V planes,only ROTDMA0
    YV12_Planar,
    YUV420_Planar   = YV12_Planar,

    //Y plan only,only ROTDMA0
    Y800,
    Y8  = Y800,
    GREY= Y800,

    /*-----------------------------------------------------------------------------
        YUV Interleave
      -----------------------------------------------------------------------------*/
    //YUV420, 2x2 subsampled , interleaved U/V plane,only ROTDMA0
    NV12,
    YUV420_Inter    = NV12,

    //YUV420, 2x2 subsampled , interleaved V/U plane,only ROTDMA0
    NV21,
    YVU420_Inter    = NV21,

    /*-----------------------------------------------------------------------------
        Misc
      -----------------------------------------------------------------------------*/
    //For encoder use,only ROTDMA0 (So called "MTK YUV")
    YUV420_4x4BLK,
    MTK_YUV420  = YUV420_4x4BLK,

    //Android defined YV12 (YUV420 Planar, 2x2 subsampled, with 16x aligned Y,U,V stride
    ANDROID_YV12,

    //Output the same data with input
    RAW,
} IspColorFormat;

/*-----------------------------------------------------------------------------
    UTILITY MACRO
  -----------------------------------------------------------------------------*/
#define ISP_IS_ALIGN( _number_, _power_of_2_ )      ( ( _number_ & ( (0x1<<_power_of_2_)-1 ) ) ? 0 : 1 )
#define ISP_ROUND_UP( _number_, _power_of_2_ )      _number_ = ( ( (_number_ + (( 0x1 << _power_of_2_ )-1)) >> _power_of_2_ ) << _power_of_2_ )

#endif

