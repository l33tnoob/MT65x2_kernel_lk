#ifndef __DDP_HAL_H__
#define __DDP_HAL_H__


#if 1
#include "DpDataType.h"

#else
//FMT GROUP , 0-RGB , 1-YUV , 2-Bayer raw , 3-compressed format
#define DP_COLORFMT_PACK(PLANE, COPLANE, HFACTOR, VFACTOR, BITS, GROUP , UNIQUEID)                              \
    ((PLANE    << 24) |                                                                                         \
     (COPLANE  << 22) |                                                                                         \
     (HFACTOR  << 20) |                                                                                         \
     (VFACTOR  << 18)  |                                                                                        \
     (BITS     << 8) |                                                                                          \
     (GROUP    << 6) |                                                                                          \
     (UNIQUEID << 0))                                                                                           \

#define DP_COLOR_GET_PLANE_COUNT(color)   ((0x0F000000 & color) >> 24)
#define DP_COLOR_IS_UV_COPLANE(color)     ((0x00C00000 & color) >> 22)
#define DP_COLOR_GET_H_SUBSAMPLE(color)   ((0x00300000 & color) >> 20)
#define DP_COLOR_GET_V_SUBSAMPLE(color)   ((0x000C0000 & color) >> 18)
#define DP_COLOR_BITS_PER_PIXEL(color)    ((0x0003FF00 & color) >>  8)
#define DP_COLOR_GET_COLOR_GROUP(color)   ((0x000000C0 & color) >>  6)
#define DP_COLOR_GET_UNIQUE_ID(color)     ((0x0000003F & color) >>  0)

typedef enum _DP_COLOR_ENUM
{
    DP_COLOR_BAYER8         = DP_COLORFMT_PACK(1,  0, 0, 0,  8, 2,  0),
    DP_COLOR_BAYER10        = DP_COLORFMT_PACK(1,  0, 0, 0, 10, 2,  1),
    DP_COLOR_BAYER12        = DP_COLORFMT_PACK(1,  0, 0, 0, 12, 2,  2),
    DP_COLOR_RGB565         = DP_COLORFMT_PACK(1,  0, 0, 0, 16, 0,  3),
    DP_COLOR_BGR565         = DP_COLORFMT_PACK(1,  0, 0, 0, 16, 0,  4),
    DP_COLOR_RGB888         = DP_COLORFMT_PACK(1,  0, 0, 0, 24, 0,  5),
    DP_COLOR_BGR888         = DP_COLORFMT_PACK(1,  0, 0, 0, 24, 0,  6),
    DP_COLOR_RGBX8888       = DP_COLORFMT_PACK(1,  0, 0, 0, 32, 0,  7),
    DP_COLOR_BGRX8888       = DP_COLORFMT_PACK(1,  0, 0, 0, 32, 0,  8),
    DP_COLOR_RGBA8888       = DP_COLORFMT_PACK(1,  0, 0, 0, 32, 0,  9),
    DP_COLOR_BGRA8888       = DP_COLORFMT_PACK(1,  0, 0, 0, 32, 0, 10),
    DP_COLOR_XRGB8888       = DP_COLORFMT_PACK(1,  0, 0, 0, 32, 0, 11),
    DP_COLOR_XBGR8888       = DP_COLORFMT_PACK(1,  0, 0, 0, 32, 0, 12),
    DP_COLOR_ARGB8888       = DP_COLORFMT_PACK(1,  0, 0, 0, 32, 0, 13),
    DP_COLOR_ABGR8888       = DP_COLORFMT_PACK(1,  0, 0, 0, 32, 0, 14),
    DP_COLOR_I420           = DP_COLORFMT_PACK(3,  0, 1, 1,  8, 1, 15),
    DP_COLOR_YV12           = DP_COLORFMT_PACK(3,  0, 1, 1,  8, 1, 16),
    DP_COLOR_NV12           = DP_COLORFMT_PACK(2,  1, 1, 1,  8, 1, 17),
    DP_COLOR_NV21           = DP_COLORFMT_PACK(2,  1, 1, 1,  8, 1, 18),
    DP_COLOR_I422           = DP_COLORFMT_PACK(3,  0, 1, 0,  8, 1, 19),
    DP_COLOR_YV16           = DP_COLORFMT_PACK(3,  0, 1, 0,  8, 1, 20),
    DP_COLOR_NV16           = DP_COLORFMT_PACK(2,  1, 1, 0,  8, 1, 21),
    DP_COLOR_NV61           = DP_COLORFMT_PACK(2,  1, 1, 0,  8, 1, 22),
    DP_COLOR_YUYV           = DP_COLORFMT_PACK(1,  0, 1, 0, 16, 1, 23),
    DP_COLOR_YVYU           = DP_COLORFMT_PACK(1,  0, 1, 0, 16, 1, 24),
    DP_COLOR_UYVY           = DP_COLORFMT_PACK(1,  0, 1, 0, 16, 1, 25),
    DP_COLOR_VYUY           = DP_COLORFMT_PACK(1,  0, 1, 0, 16, 1, 26),
    DP_COLOR_I444           = DP_COLORFMT_PACK(3,  0, 0, 0,  8, 1, 27),
    DP_COLOR_IYU2           = DP_COLORFMT_PACK(3,  0, 0, 0, 24, 1, 28),
    DP_COLOR_NV24           = DP_COLORFMT_PACK(2,  1, 0, 0,  8, 1, 29),
    DP_COLOR_NV42           = DP_COLORFMT_PACK(2,  1, 0, 0,  8, 1, 30),
    DP_COLOR_GREY           = DP_COLORFMT_PACK(1,  0, 0, 0,  8, 1, 31),

    // Mediatek proprietary format
    DP_COLOR_420_BLKP       = DP_COLORFMT_PACK(2,  1, 1, 1, 256, 1, 32),
    DP_COLOR_420_BLKI       = DP_COLORFMT_PACK(2,  1, 1, 1, 256, 1, 33),
    DP_COLOR_422_BLKP       = DP_COLORFMT_PACK(1,  0, 1, 0, 512, 1, 34),
    DP_COLOR_YUY2           = DP_COLORFMT_PACK(1 , 0, 1, 0, 16 , 1 , 35),
    DP_COLOR_PARGB8888      = DP_COLORFMT_PACK(1 , 0, 0, 0, 32 , 0 , 36),
    DP_COLOR_XARGB8888      = DP_COLORFMT_PACK(1 , 0, 0, 0, 32 , 0 , 37),
    DP_COLOR_PABGR8888      = DP_COLORFMT_PACK(1 , 0, 0, 0, 32 , 0 , 38),
    DP_COLOR_XABGR8888      = DP_COLORFMT_PACK(1 , 0, 0, 0, 32 , 0 , 39),

    DP_COLOR_YUV444         = DP_COLORFMT_PACK(1,  0, 0, 0, 24, 1, 40),
//    DP_COLOR_YUV422I 		    = DP_COLORFMT_PACK(1,  0, 1, 0, 16, 1, 41),//Dup to DP_COLOR_YUYV
//    DP_COLOR_Y800   	      = DP_COLORFMT_PACK(1,  0, 1, 0, 8, 1, 42),//Dup to DP_COLOR_GREY
//    DP_COLOR_COMPACT_RAW1	  = DP_COLORFMT_PACK(1,  0, 1, 0, 10, 2, 43),//Dup to Bayer10
//    DP_COLOR_420_3P_YVU     = DP_COLORFMT_PACK(3,  0, 1, 1,  8, 1, 44),//Dup to DP_COLOR_YV12
} DP_COLOR_ENUM;


// Legacy for 6589 compatible
typedef DP_COLOR_ENUM DpColorFormat;

#define eYUV_420_3P             DP_COLOR_I420
#define eYUV_420_2P_YUYV        DP_COLOR_YUYV
#define eYUV_420_2P_UYVY        DP_COLOR_UYVY
#define eYUV_420_2P_YVYU        DP_COLOR_YVYU
#define eYUV_420_2P_VYUY        DP_COLOR_VYUY
#define eYUV_420_2P_ISP_BLK     DP_COLOR_420_BLKP
#define eYUV_420_2P_VDO_BLK     DP_COLOR_420_BLKI
#define eYUV_422_3P             DP_COLOR_I422
#define eYUV_422_2P             DP_COLOR_NV16
#define eYUV_422_I              DP_COLOR_YUYV
#define eYUV_422_I_BLK          DP_COLOR_422_BLKP
#define eYUV_444_3P             DP_COLOR_I444
#define eYUV_444_2P             DP_COLOR_NV24
#define eYUV_444_1P             DP_COLOR_YUV444
#define eBAYER8                 DP_COLOR_BAYER8
#define eBAYER10                DP_COLOR_BAYER10
#define eBAYER12                DP_COLOR_BAYER12
#define eRGB565                 DP_COLOR_RGB565
#define eBGR565                 DP_COLOR_BGR565
#define eRGB888                 DP_COLOR_RGB888
#define eBGR888                 DP_COLOR_BGR888
#define eARGB8888               DP_COLOR_ARGB8888
#define eABGR8888               DP_COLOR_ABGR8888
#define eRGBA8888               DP_COLOR_RGBA8888
#define eBGRA8888               DP_COLOR_BGRA8888
#define eXRGB8888               DP_COLOR_XRGB8888
#define eXBGR8888               DP_COLOR_XBGR8888
#define eRGBX8888               DP_COLOR_RGBX8888
#define eBGRX8888               DP_COLOR_BGRX8888
#define ePARGB8888              DP_COLOR_PARGB8888
#define eXARGB8888              DP_COLOR_XARGB8888
#define ePABGR8888              DP_COLOR_PABGR8888
#define eXABGR8888              DP_COLOR_XABGR8888
#define eGREY                   DP_COLOR_GREY
#define eI420                   DP_COLOR_I420
#define eYV12                   DP_COLOR_YV12
#define eYV21                   DP_COLOR_I420
#define eNV12_BLK               DP_COLOR_420_BLKP
#define eNV12_BLK_FCM           DP_COLOR_420_BLKI
#define eYUV_420_3P_YVU         DP_COLOR_YV12

#define eNV12_BP                DP_COLOR_420_BLKP
#define eNV12_BI                DP_COLOR_420_BLKI
#define eNV12                   DP_COLOR_NV12
#define eNV21                   DP_COLOR_NV21
#define eI422                   DP_COLOR_I422
#define eYV16                   DP_COLOR_YV16
#define eNV16                   DP_COLOR_NV16
#define eNV61                   DP_COLOR_NV61
#define eUYVY                   DP_COLOR_UYVY
#define eVYUY                   DP_COLOR_VYUY
#define eYUYV                   DP_COLOR_YUYV
#define eYVYU                   DP_COLOR_YVYU
#define eUYVY_BP                DP_COLOR_422_BLKP
#define eI444                   DP_COLOR_I444
#define eNV24                   DP_COLOR_NV24
#define eNV42                   DP_COLOR_NV42
#define eYUY2                   DP_COLOR_YUY2
#define eY800                   DP_COLOR_GREY
//#define eIYU2
#define eMTKYUV                 DP_COLOR_422_BLKP

#define eCompactRaw1            DP_COLOR_BAYER10

#endif


struct DISP_REGION
{
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;
};

enum OVL_LAYER_SOURCE {
    OVL_LAYER_SOURCE_MEM    = 0,
    OVL_LAYER_SOURCE_RESERVED = 1,
    OVL_LAYER_SOURCE_SCL     = 2,
    OVL_LAYER_SOURCE_PQ     = 3,
};

enum OVL_LAYER_SECURE_MODE {
    OVL_LAYER_NORMAL_BUFFER    = 0,
    OVL_LAYER_SECURE_BUFFER    = 1,
    OVL_LAYER_PROTECTED_BUFFER = 2
};

typedef struct _OVL_CONFIG_STRUCT
{
    unsigned int layer;
    unsigned int layer_en;
    enum OVL_LAYER_SOURCE source;
    unsigned int fmt;
    unsigned int addr; 
    unsigned int vaddr;
    unsigned int src_x;
    unsigned int src_y;
    unsigned int src_w;
    unsigned int src_h;
    unsigned int src_pitch;
    unsigned int dst_x;
    unsigned int dst_y;
    unsigned int dst_w;
    unsigned int dst_h;                  // clip region
    unsigned int keyEn;
    unsigned int key; 
    unsigned int aen; 
    unsigned char alpha;  

    unsigned int isTdshp;
    unsigned int isDirty;

    int buff_idx;
    int identity;
    int connected_type;
    unsigned int security;
}OVL_CONFIG_STRUCT;

struct disp_path_config_struct
{
    unsigned int srcModule; // DISP_MODULE_ENUM

    // if srcModule=RDMA0, set following value, else do not have to set following value
    unsigned int addr; 
    unsigned int inFormat; 
    unsigned int pitch;
    struct DISP_REGION srcROI;        // ROI

    OVL_CONFIG_STRUCT ovl_config;

    struct DISP_REGION bgROI;         // background ROI
    unsigned int bgColor;  // background color

    unsigned int dstModule; // DISP_MODULE_ENUM
    unsigned int outFormat; 
    unsigned int dstAddr;  // only take effect when dstModule=DISP_MODULE_WDMA0 or DISP_MODULE_WDMA1

    int srcWidth, srcHeight;
    int dstWidth, dstHeight;
    int dstPitch;
};

struct disp_path_config_mem_out_struct
{
    unsigned int enable;
    unsigned int dirty;
    unsigned int outFormat; 
    unsigned int dstAddr;
    struct DISP_REGION srcROI;        // ROI
};

struct disp_path_config_ovl_mode_t
{
    unsigned int mode;
    unsigned int pitch;
	unsigned int format;
    unsigned int address;
    struct DISP_REGION roi;
};

enum RDMA_OUTPUT_FORMAT {
    RDMA_OUTPUT_FORMAT_ARGB   = 0,
    RDMA_OUTPUT_FORMAT_YUV444 = 1,
};

enum RDMA_MODE {
    RDMA_MODE_DIRECT_LINK = 0,
    RDMA_MODE_MEMORY      = 1,
};
typedef struct _RDMA_CONFIG_STRUCT
{
    unsigned idx;            // instance index
    enum RDMA_MODE mode;          // data mode
    DpColorFormat inputFormat;
    unsigned address;
    unsigned pitch;
    bool isByteSwap;
    enum RDMA_OUTPUT_FORMAT outputFormat;
    unsigned width;
    unsigned height;
    bool isRGBSwap;
}RDMA_CONFIG_STRUCT;


int disp_wait_timeout(bool flag, unsigned int timeout);
int disp_delay_timeout(bool flag, unsigned int delay_ms);

int disp_path_config(struct disp_path_config_struct* pConfig);
int disp_path_config_layer(OVL_CONFIG_STRUCT* pOvlConfig);
int disp_path_config_layer_addr(unsigned int layer, unsigned int addr);
int disp_path_get_mutex(void);
int disp_path_release_mutex(void);
int disp_path_wait_reg_update(void);

int disp_path_get_mutex_(int mutexId);
int disp_path_release_mutex_(int mutexId);
int disp_path_config_(struct disp_path_config_struct* pConfig, int mutexId);

int disp_path_config_mem_out(struct disp_path_config_mem_out_struct* pConfig);
int disp_path_config_mem_out_without_lcd(struct disp_path_config_mem_out_struct* pConfig);
void disp_path_wait_mem_out_done(void);
int disp_path_clock_on(char* name);
int disp_path_clock_off(char* name);
int disp_path_change_tdshp_status(unsigned int layer, unsigned int enable);

void disp_path_clear_mem_out_done_flag(void);
int disp_path_query(void); // return different functions according to chip type
int disp_bls_set_max_backlight(unsigned int level);

int disp_path_config_rdma (RDMA_CONFIG_STRUCT* pRdmaConfig);
int disp_path_config_wdma (struct disp_path_config_mem_out_struct* pConfig);
int disp_path_switch_ovl_mode (struct disp_path_config_ovl_mode_t *pConfig);
int disp_path_get_mem_read_mutex (void);
int disp_path_release_mem_read_mutex (void);
int disp_path_get_mem_write_mutex (void);
int disp_path_release_mem_write_mutex (void);
int disp_path_wait_frame_done(void);
#endif

