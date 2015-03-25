/**
* @file isp_function.h
*
* isp_function Header File
*/

#ifndef __ISP_FUNCTION_H__
#define __ISP_FUNCTION_H__

#include <vector>
#include <map>
#include <list>
using namespace std;

#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
//#include <cutils/pmem.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#include "isp_datatypes.h"
#include "camera_isp.h" //For Isp function ID
#include <mtkcam/drv/isp_reg.h>
#include <mtkcam/drv/isp_drv.h>
//#include "isp_drv_imp.h"
#include "cdp_drv.h"
#include <mtkcam/drv/tpipe_drv.h>
#include <mtkcam/drv/imem_drv.h>

#include <mtkcam/imageio/ispio_pipe_scenario.h>    // For enum EScenarioID.

/*
    //path
        -IC
            -IC_PASS1
                -TG->Dram(imgo)
                    |
                    ->isp(raw)->lcs0/aao/flko/afo/eiso
            -IC_PASS2(CQ/Tpipe)
                -Dram->isp(raw/rgb)->isp(yuv)->cdrz->prz/vrz->dispo/vido/vrzo->Dram
        -VR
            -VR_PASS1
                -TG->isp(raw)->isp(rgb)->isp(yuv)->cdrz->Dram
            -VR_PASS2(CQ)
                -Dram->curz->prz/vrz->Dram
        -IP
            -IP_PASS2(CQ/Tpipe)
                -Dram->isp(raw/rgb)->isp(yuv)->cdrz->prz/vrz->dispo/vido/vrzo->Dram
        -ZSD
            -ZSD_PASS1
                -TG->isp(raw)->isp(rgb)->isp(yuv)->cdrz->Dram(img2o)
                    |
                    ->Dram(imgo)
            -ZSD_PASS2(CQ)
                -Dram->curz->prz/vrz->dispo/vido/vrzo->Dram
        -N3D_IC
            -N3D_IC_PASS1
                -TG-> Dram(imgo)
                    |
                    ->Dram(img2o)
                    |
                    ->isp(raw)->Dram(lsco/aao/afo/eiso)
            -N3D_IC_PASS2(CQ1/CQ2/Tpipe)
                -Dram->isp(raw)->isp(rgb)->isp(yuv)->cdrz->prz/vrz->dispo/vido/vrzo->Dram
        -N3D_VR
            -N3D_VR_PASS1
                -TG-> Dram(imgo)
                    |
                    ->Dram(img2o)
                    |
                    ->isp(raw)->Dram(lsco/aao/nr3o/afo/eiso)
            -N3D_VR_PASS2(CQ1/CQ2)
                -Dram->isp(raw)->isp(rgb)->isp(yuv)->cdrz->prz/vrz->dispo/vido/vrzo->Dram


    //CQ
        -set 1 register
            -0x00004000 -> {command_set[5:0],command_cnt[9:0],ini_addr[15:0]}
            -0x12345678 -> register set 0xC2084000 = 0x12345678
            -0xFC000000 -> End CQ release imgi,trigger camera
        -set 3 registers
            -0x00024100 -> {command_set[5:0],command_cnt[9:0],ini_addr[15:0]}
            -0x01234567 -> register set 0xC2084100 = 0x12345678
            -0x12345678 -> register set 0xC2084104 = 0x12345678
            -0x23456789 -> register set 0xC2084108 = 0x23456789
            -0xFC000000 -> End CQ release imgi,trigger camera

    //TDR
        -put tpipe_map data in Dram
        -set CAM_TDRI_BASE_ADDR
        -set CAM_TDRI_OFST_ADDR = 0;
        -set CAM_TDRI_XSIZE = sizeof(tile_map)
        -enable CAM_CTL_TCM_EN[31]
        -enable specific DMA
            -CAM_CTL_DMA_EN

    //DMA programming guide
        -enable specific DMA
            -CAM_CTL_DMA_EN
        -enable DMA internal error/OTF overflow error interrupt
            -CAM_CTL_INT_EN[30][24:20]
        -enable DMA status interrupt
            -CAM_CTL_DMA_INT[24:16][8:0]
        -config used i/o DMA
            -input data format for alignment check(BASE_ADDR/OFST_ADDR/STRIDE)
                -YUV422 one plane - 4 Bytes
                -YUV420/422 two plane - 2 Bytes
                -JPG - 16 Bytes
                -CAM_CTL_FMT_SEL[10:8]
            -CAM_IMGI_BASE_ADDR
            -CAM_IMGI_OFST_ADDR
            -CAM_IMGI_XSIZE (from 0)
                -multiple of data bus width(6589_ISP_DMA_Description.xls)
            -CAM_IMGI_YSIZE (from 0)
            -CAM_IMGI_STRIDE (>=(XSIZE+1))
            -CAM_IMGI_CON = 0x08404040
                -max_burst_len = {1,2,4,8}

            -CAM_IMGO_BASE_ADDR
            -CAM_IMGO_OFST_ADDR
            -CAM_IMGO_XSIZE (from 0)
            -CAM_IMGO_YSIZE (from 0)
            -CAM_IMGO_STRIDE (>=(XSIZE+1))
            -CAM_IMGO_CON = 0x08505050
                -max_burst_len = {1,2,4,8}
            -CAM_IMGO_CROP
    //SW RST
        -CAM_CTL_SW_CTL[0] = 1
        -wait
            -CAM_CTL_SW_CTL[1] = 1
            -CAM_DMA_SOFT_RSTSTAT = 0x003F01FE
        -CAM_CTL_SW_CTL[2] = 1
        -delay
        -CAM_CTL_SW_CTL[0] = 0
        -CAM_CTL_SW_CTL[2] = 0
*/

// for crop function
#define CROP_FLOAT_PECISE_BIT   31    // precise 31 bit
#define CROP_TPIPE_PECISE_BIT   20
#define CROP_CDP_HARDWARE_BIT   8

#define ISP_PASS1    (0x01)
#define ISP_PASS2    (0x02)
#define ISP_PASS2B   (0x04)
#define ISP_PASS2C   (0x08)
#define ISP_PASS2FMT (0x10)

//start bit
#define CAM_ISP_PASS1_START         0
#define CAM_ISP_PASS2_START         1
#define CAM_ISP_PASS2B_START        2
#define CAM_ISP_PASS2C_START        3
#define CAM_ISP_FMT_START           4
#define CAM_ISP_PASS1_CQ0_START     5
#define CAM_ISP_PASS1_CQ0B_START    6

#define ISP_TOP             ( 1u << 0 )
#define ISP_RAW             ( 1u << 1 )
#define ISP_RGB             ( 1u << 2 )
#define ISP_YUV             ( 1u << 3 )
#define ISP_CDP             ( 1u << 4 )
#define ISP_TDRI            ( 1u << 5 )
#define ISP_DMA_IMGI        ( 1u << 6 )
#define ISP_DMA_IMGCI       ( 1u << 7 )
#define ISP_DMA_FLKI        ( 1u << 8 )
#define ISP_DMA_LSCI        ( 1u << 9 )
#define ISP_DMA_LCEI        ( 1u << 10)
#define ISP_DMA_VIPI        ( 1u << 11)
#define ISP_DMA_VIP2I       ( 1u << 12)
#define ISP_DMA_CQI         ( 1u << 13)
#define ISP_DMA_TDRI        ( 1u << 14)
#define ISP_DMA_IMGO        ( 1u << 15)
#define ISP_DMA_IMG2O       ( 1u << 16)
#define ISP_DMA_LCSO        ( 1u << 17)
#define ISP_DMA_AAO         ( 1u << 18)
#define ISP_DMA_ESFKO       ( 1u << 19)
#define ISP_DMA_AFO         ( 1u << 20)
#define ISP_DMA_EISO        ( 1u << 21)
#define ISP_DMA_DISPO       ( 1u << 22)
#define ISP_DMA_VIDO        ( 1u << 23)
#define ISP_DMA_FDO         ( 1u << 24)
#define ISP_BUFFER          ( 1u << 25)
#define ISP_TURNING         ( 1u << 26)

#define ISP_FUNCTION_MAX_NUM (27)

//Tpipe Driver
#define ISP_MAX_TDRI_HEX_SIZE           (TPIPE_DRV_MAX_TPIPE_HEX_SIZE)  //36K
#define ISP_MAX_RING_TDRI_SIZE          (768000) // 750K

//isp top register range
#define MT6589_ISP_TOP_BASE             ISP_BASE_HW
#define MT6589_ISP_TOP_REG_RANGE        ISP_BASE_RANGE

//ISP_RAW
#define MT6589_ISP_RAW_BASE             (MT6589_ISP_TOP_BASE + 0x0)
#define MT6589_ISP_RAW_REG_RANGE        MT6589_ISP_TOP_REG_RANGE
//ISP_RGB
#define MT6589_ISP_RGB_BASE             (MT6589_ISP_TOP_BASE + 0x0)
#define MT6589_ISP_RGB_REG_RANGE        MT6589_ISP_TOP_REG_RANGE
//ISP_YUV
#define MT6589_ISP_YUV_BASE             (MT6589_ISP_TOP_BASE + 0x0)
#define MT6589_ISP_YUV_REG_RANGE        MT6589_ISP_TOP_REG_RANGE
//ISP_CDP
#define MT6589_ISP_CDP_BASE             (MT6589_ISP_TOP_BASE + 0xB00)
#define MT6589_ISP_CDP_REG_RANGE        0x1000
//ISP_DMA_IMGI
#define MT6589_DMA_IMGI_BASE            (MT6589_ISP_TOP_BASE + 0x230)
#define MT6589_DMA_IMGI_REG_RANGE       0x20
//ISP_DMA_IMGCI
#define MT6589_DMA_IMGCI_BASE           (MT6589_ISP_TOP_BASE + 0x24C)
#define MT6589_DMA_IMGCI_REG_RANGE      0x20
//ISP_DMA_FLKI
#define MT6589_DMA_FLKI_BASE            (MT6589_ISP_TOP_BASE + 0x290)
#define MT6589_DMA_FLKI_REG_RANGE       0x20
//ISP_DMA_LSCI
#define MT6589_DMA_LSCI_BASE            (MT6589_ISP_TOP_BASE + 0x278)
#define MT6589_DMA_LSCI_REG_RANGE       0x20
//ISP_DMA_LCEI
#define MT6589_DMA_LCEI_BASE            (MT6589_ISP_TOP_BASE + 0x2A8)
#define MT6589_DMA_LCEI_REG_RANGE       0x20
//ISP_DMA_VIPI
#define MT6589_DMA_VIPI_BASE            (MT6589_ISP_TOP_BASE + 0x2C0)
#define MT6589_DMA_VIPI_REG_RANGE       0x20
//ISP_DMA_VIP2I
#define MT6589_DMA_VIP2I_BASE            (MT6589_ISP_TOP_BASE + 0x2D8)
#define MT6589_DMA_VIP2I_REG_RANGE       0x20
//ISP_DMA_IMGO
#define MT6589_DMA_IMGO_BASE            (MT6589_ISP_TOP_BASE + 0x2F0)
#define MT6589_DMA_IMGO_REG_RANGE       0x20
//ISP_DMA_IMG2O
#define MT6589_DMA_IMG2O_BASE            (MT6589_ISP_TOP_BASE + 0x30C)
#define MT6589_DMA_IMG2O_REG_RANGE       0x20
//ISP_DMA_LCSO
#define MT6589_DMA_LCSO_BASE            (MT6589_ISP_TOP_BASE + 0x328)
#define MT6589_DMA_LCSO_REG_RANGE       0x20
//ISP_DMA_AAO
#define MT6589_DMA_AAO_BASE            (MT6589_ISP_TOP_BASE + 0x368)
#define MT6589_DMA_AAO_REG_RANGE       0x20
//ISP_DMA_ESFKO
#define MT6589_DMA_ESFKO_BASE            (MT6589_ISP_TOP_BASE + 0x350)
#define MT6589_DMA_ESFKO_REG_RANGE       0x20
//ISP_DMA_AFO
#define MT6589_DMA_AFO_BASE            (MT6589_ISP_TOP_BASE + 0x348)
#define MT6589_DMA_AFO_REG_RANGE       0x20
//ISP_DMA_EISO
#define MT6589_DMA_EISO_BASE            (MT6589_ISP_TOP_BASE + 0x340)
#define MT6589_DMA_EISO_REG_RANGE       0x20
//ISP_DMA_DISPO
#define MT6589_DMA_DISPO_BASE            (MT6589_ISP_TOP_BASE + 0xD40)
#define MT6589_DMA_DISPO_REG_RANGE       0x100
//ISP_DMA_VIDO
#define MT6589_DMA_VIDO_BASE            (MT6589_ISP_TOP_BASE + 0xCC0)
#define MT6589_DMA_VIDO_REG_RANGE       0x100
//ISP_DMA_CQ0I
#define MT6589_DMA_CQI_BASE            (MT6589_ISP_TOP_BASE + 0x210)
#define MT6589_DMA_CQI_REG_RANGE       0x08
//ISP_DMA_TDRI
#define MT6589_DMA_TDRI_BASE            (MT6589_ISP_TOP_BASE + 0x204)
#define MT6589_DMA_TDRI_REG_RANGE       0x20

#define ISP_MAX_RINGBUFFER_CNT  16


//-->definition compatible to mt6589_scenario.xlsx
//cam_fmt_sel
//->//
#define ISP_SCENARIO_IC         0
#define ISP_SCENARIO_VR         1
#define ISP_SCENARIO_ZSD        2
#define ISP_SCENARIO_IP         3
#define ISP_SCENARIO_VEC        4
#define ISP_SCENARIO_RESERVE01  5
#define ISP_SCENARIO_N3D_IC     6
#define ISP_SCENARIO_N3D_VR     7
#define ISP_SCENARIO_MAX        8
//->//
#define ISP_SUB_MODE_RAW        0
#define ISP_SUB_MODE_YUV        1
#define ISP_SUB_MODE_RGB        2
#define ISP_SUB_MODE_JPG        3
#define ISP_SUB_MODE_MFB        4
#define ISP_SUB_MODE_VEC        0
#define ISP_SUB_MODE_RGB_LOAD   3
#define ISP_SUB_MODE_MAX        5
//->//
#define CAM_FMT_SEL_TG_FMT_RAW8   0
#define CAM_FMT_SEL_TG_FMT_RAW10  1
#define CAM_FMT_SEL_TG_FMT_RAW12  2
#define CAM_FMT_SEL_TG_FMT_YUV422 3
#define CAM_FMT_SEL_TG_FMT_RGB565 5
#define CAM_FMT_SEL_TG_FMT_JPEG   7
//->//
#define CAM_FMT_SEL_YUV420_2P    0
#define CAM_FMT_SEL_YUV420_3P    1
#define CAM_FMT_SEL_YUV422_1P    2
#define CAM_FMT_SEL_YUV422_2P    3
#define CAM_FMT_SEL_YUV422_3P    4
#define CAM_FMT_SEL_Y_ONLY       12
#define CAM_FMT_SEL_RGB565       0
#define CAM_FMT_SEL_RGB888       1
#define CAM_FMT_SEL_XRGB8888     2
#define CAM_FMT_SEL_BAYER8       0
#define CAM_FMT_SEL_BAYER10      1
#define CAM_FMT_SEL_BAYER12      2
//->//
#define CAM_FMT_SEL_TG_SW_UYVY      0
#define CAM_FMT_SEL_TG_SW_YUYV      1
#define CAM_FMT_SEL_TG_SW_VYUY      2
#define CAM_FMT_SEL_TG_SW_YVYU      3
#define CAM_FMT_SEL_TG_SW_RGB       0
#define CAM_FMT_SEL_TG_SW_BGR       2
//pixel id
#define CAM_PIX_ID_B    0
#define CAM_PIX_ID_Gb   1
#define CAM_PIX_ID_Gr   2
#define CAM_PIX_ID_R    3
//enable table EN1
#define CAM_CTL_EN1_TG1_EN  (1L<<0)
#define CAM_CTL_EN1_TG2_EN  (1L<<1)
#define CAM_CTL_EN1_BIN_EN  (1L<<2)
#define CAM_CTL_EN1_OB_EN   (1L<<3)
#define CAM_CTL_EN1_LSC_EN  (1L<<5)
#define CAM_CTL_EN1_BNR_EN  (1L<<7)
#define CAM_CTL_EN1_SL2_EN  (1L<<8)
#define CAM_CTL_EN1_HRZ_EN  (1L<<9)
#define CAM_CTL_EN1_PGN_EN  (1L<<11)
#define CAM_CTL_EN1_PAK_EN  (1L<<12)
#define CAM_CTL_EN1_PAK2_EN (1L<<13)
#define CAM_CTL_EN1_SGG_EN  (1L<<15)
#define CAM_CTL_EN1_AF_EN   (1L<<16)
#define CAM_CTL_EN1_FLK_EN  (1L<<17)
#define CAM_CTL_EN1_AA_EN   (1L<<18)
#define CAM_CTL_EN1_LCS_EN  (1L<<19)
#define CAM_CTL_EN1_UNP_EN  (1L<<20)
#define CAM_CTL_EN1_CFA_EN  (1L<<21)
#define CAM_CTL_EN1_CCL_EN  (1L<<22)
#define CAM_CTL_EN1_G2G_EN  (1L<<23)
#define CAM_CTL_EN1_DGM_EN  (1L<<24)
#define CAM_CTL_EN1_LCE_EN  (1L<<25)
#define CAM_CTL_EN1_GGM_EN  (1L<<26)
#define CAM_CTL_EN1_C02_EN  (1L<<27)
#define CAM_CTL_EN1_MFB_EN  (1L<<28)
#define CAM_CTL_EN1_C24_EN  (1L<<29)
#define CAM_CTL_EN1_CAM_EN  (1L<<30)
#define CAM_CTL_EN1_BIN2_EN  (1L<<31)

//enable table EN2
#define CAM_CTL_EN2_G2C_EN     (1L<<0)
#define CAM_CTL_EN2_C42_EN     (1L<<1)
#define CAM_CTL_EN2_NBC_EN     (1L<<2)
#define CAM_CTL_EN2_PCA_EN     (1L<<3)
#define CAM_CTL_EN2_SEEE_EN    (1L<<4)
#define CAM_CTL_EN2_NR3D_EN    (1L<<5)
#define CAM_CTL_EN2_CQ0C_EN    (1L<<14)
#define CAM_CTL_EN2_CQ0B_EN    (1L<<15)
#define CAM_CTL_EN2_EIS_EN     (1L<<16)
#define CAM_CTL_EN2_CDRZ_EN    (1L<<17)
#define CAM_CTL_EN2_CURZ_EN    (1L<<18)
#define CAM_CTL_EN2_PRZ_EN     (1L<<21)
#define CAM_CTL_EN2_UV_CRSA_EN (1L<<23)
#define CAM_CTL_EN2_FE_EN      (1L<<24)
#define CAM_CTL_EN2_GDMA_EN    (1L<<25)
#define CAM_CTL_EN2_FMT_EN     (1L<<26)
#define CAM_CTL_EN2_CQ1_EN     (1L<<27)
#define CAM_CTL_EN2_CQ2_EN     (1L<<28)
#define CAM_CTL_EN2_CQ3_EN     (1L<<29)
#define CAM_CTL_EN2_G2G2_EN    (1L<<30)
#define CAM_CTL_EN2_CQ0_EN     (1L<<31)

//enable table DMA_EN   reg_400C
#define CAM_CTL_DMA_EN_IMGO_EN  (1L<<0)
#define CAM_CTL_DMA_EN_LSCI_EN  (1L<<1)
#define CAM_CTL_DMA_EN_ESFKO_EN (1L<<3)
#define CAM_CTL_DMA_EN_AAO_EN   (1L<<5)
#define CAM_CTL_DMA_EN_LCSO_EN  (1L<<6)
#define CAM_CTL_DMA_EN_IMGI_EN  (1L<<7)
#define CAM_CTL_DMA_EN_IMGCI_EN (1L<<8)
#define CAM_CTL_DMA_EN_IMG2O_EN (1L<<10)
#define CAM_CTL_DMA_EN_FLKI_EN  (1L<<11)
#define CAM_CTL_DMA_EN_LCEI_EN  (1L<<12)
#define CAM_CTL_DMA_EN_VIPI_EN  (1L<<13)
#define CAM_CTL_DMA_EN_VIP2I_EN (1L<<14)
#define CAM_CTL_DMA_EN_VIDO_EN  (1L<<19)
#define CAM_CTL_DMA_EN_DISPO_EN (1L<<21)

//enable table DMA_INT   reg_4028
#define CAM_CTL_DMA_INT_IMGO_DONE_EN    (1L<<16)
#define CAM_CTL_DMA_INT_IMG2O_DONE_EN   (1L<<17)
#define CAM_CTL_DMA_INT_AAO_DONE_EN     (1L<<18)
#define CAM_CTL_DMA_INT_LCSO_DONE_EN    (1L<<19)
#define CAM_CTL_DMA_INT_ESFKO_DONE_EN   (1L<<20)
#define CAM_CTL_DMA_INT_DISPO_DONE_EN   (1L<<21)
#define CAM_CTL_DMA_INT_VIDO_DONE_EN    (1L<<22)
#define CAM_CTL_DMA_INT_CQ0_VR_SNAP_EN  (1L<<23)
#define CAM_CTL_DMA_INT_CQ0_ERR_EN      (1L<<24)
#define CAM_CTL_DMA_INT_CQ0_DONE_EN     (1L<<25)
#define CAM_CTL_DMA_INT_SOF2_INT_EN     (1L<<26)
#define CAM_CTL_DMA_INT_DROP2_INT_EN    (1L<<27)
#define CAM_CTL_DMA_INT_TG1_GBERR_EN    (1L<<28)
#define CAM_CTL_DMA_INT_TG2_GBERR_EN    (1L<<29)
#define CAM_CTL_DMA_INT_CQ0C_DONE_EN    (1L<<30)
#define CAM_CTL_DMA_INT_CQ0B_DONE_EN    (1L<<31)

//#define WDMA_PORT (0)
//#define WROT_PORT (1)
//#define IMG2O_PORT (2)
/*/////////////////////////////////////////////////////////////////////////////
    TODO: temp from tg_common
  /////////////////////////////////////////////////////////////////////////////*/
typedef int (*default_func)(void);
struct stCam_Id_Enable {
    int id;
    int en_bit;
};
//
#define CAM_ISP_SETTING_DONT_CARE 0
//CDP
#define CAM_CDP_CDRZ_8_TAP 0
#define CAM_CDP_CDRZ_N_TAP 1
#define CAM_CDP_CDRZ_4N_TAP 2
//
#define CAM_MODE_FRAME  0
#define CAM_MODE_TPIPE  1
#define CAM_MODE_VRMRG  2
//
#define _FMT_YUV420_2P_ 0
#define _FMT_YUV420_3P_ 1
#define _FMT_YUV422_1P_ 2
#define _FMT_YUV422_2P_ 3
#define _FMT_YUV422_3P_ 4

#define _FMT_SEQ_422_UYVY_  0
#define _FMT_SEQ_422_VYUY_  1
#define _FMT_SEQ_422_YUYV_  2
#define _FMT_SEQ_422_YVYU_  3
#define _FMT_SEQ_420_UV_    0
#define _FMT_SEQ_420_VU_    1

//
#define CAM_ISP_CQ_NONE (-1)
#define CAM_ISP_CQ0     ISP_DRV_CQ0
#define CAM_ISP_CQ0B    ISP_DRV_CQ0B
#define CAM_ISP_CQ0C    ISP_DRV_CQ0C
#define CAM_ISP_CQ1     ISP_DRV_CQ01
#define CAM_ISP_CQ2     ISP_DRV_CQ02
#define CAM_ISP_CQ3     ISP_DRV_CQ03
//
#define CAM_CQ_SINGLE_IMMEDIATE_TRIGGER CQ_SINGLE_IMMEDIATE_TRIGGER
#define CAM_CQ_SINGLE_EVENT_TRIGGER     CQ_SINGLE_EVENT_TRIGGER
#define CAM_CQ_CONTINUOUS_EVENT_TRIGGER CQ_CONTINUOUS_EVENT_TRIGGER
//
#define CAM_CQ_TRIG_BY_START        CQ_TRIG_BY_START
#define CAM_CQ_TRIG_BY_PASS1_DONE   CQ_TRIG_BY_PASS1_DONE
#define CAM_CQ_TRIG_BY_PASS2_DONE   CQ_TRIG_BY_PASS2_DONE
#define CAM_CQ_TRIG_BY_IMGO_DONE    CQ_TRIG_BY_IMGO_DONE
#define CAM_CQ_TRIG_BY_IMG2O_DONE   CQ_TRIG_BY_IMG2O_DONE
//
#define CAM_ISP_PIXEL_BYTE_FP 2
//isp line buffer
#define CAM_ISP_MAX_LINE_BUFFER_IN_PIXEL (2304)
//
#define CAM_CDP_PRZ_CONN_TO_DISPO 0
#define CAM_CDP_PRZ_CONN_TO_VIDO  1
//interrupt timeout time
#define CAM_INT_WAIT_TIMEOUT_MS 5000
#define CAM_INT_PASS2_WAIT_TIMEOUT_MS 3000

/**
*@brief  ISP enable configure struct
*/
struct stIspTopEnTbl{
    unsigned int enable1;
    unsigned int enable2;
    unsigned int dma;
	//unsigned int tcm;
	//unsigned int pixid;
	//unsigned int mdpcrop;
	//unsigned int interlace;
};

/**
*@brief  ISP interrupt configure struct
*/
struct stIspTopINT{
    unsigned int int_en;
    unsigned int int_status;
    unsigned int dma_int;
    unsigned int intb_en;
    unsigned int intb_status;
    unsigned int dmab_int;
    unsigned int intc_en;
    unsigned int intc_status;
    unsigned int dmac_int;
    unsigned int int_statusx;
    unsigned int dma_intx;
};

/**
*@brief  ISP format select configure struct
*/
struct stIspTopFmtSel {
    union {
        struct {
            unsigned int scenario       :3;
            unsigned int dummy0         :1;
            unsigned int sub_mode       :3;
            unsigned int dummy1         :1;
            unsigned int cam_in_fmt     :4;
            unsigned int cam_out_fmt    :4;
            unsigned int tg1_fmt        :3;
            unsigned int dummy2         :1;
            unsigned int tg2_fmt        :3;
            unsigned int dummy3         :1;
            unsigned int two_pix        :1;
            unsigned int two_pix2       :1;
            unsigned int tg1_sw         :2;
            unsigned int tg2_sw         :2;
            unsigned int dummy4         :2;
        }bit_field;
        unsigned int reg_val;
    };
};

/**
*@brief  ISP top select configure struct
*/
struct stIspTopSel {
    union {
        struct {
            unsigned int crz_prz_mrg        :1;
            unsigned int gdma_link          :1;
            unsigned int n3d_sof_reset_en   :1;
            unsigned int n3d_dbg_en         :1;
            unsigned int pass1_db_en        :1;
            unsigned int pass2_db_en        :1;
            unsigned int tdr_sel            :1;
            unsigned int dummy0             :1;
            unsigned int tg_sel             :1;
            unsigned int dummy1             :3;
            unsigned int dbg_sel            :3;
            unsigned int eis_sel            :1;
            unsigned int eis_raw_sel        :1;
            unsigned int CQ0_MODE           :1;
            unsigned int CQ0_VR_SNAP_MODE   :2;
            unsigned int prz_opt_sel        :2;
            unsigned int CQ0C_IMG2O_SEL     :1;
            unsigned int disp_vid_sel       :1;
            unsigned int fe_sel             :1;
            unsigned int CQ0B_MODE          :1;
            unsigned int CQ0B_VR_SNAP_MODE  :2;
            unsigned int CURZ_BORROW        :1;
            unsigned int RAW_PASS1          :1;
            unsigned int JPG_PASS1          :1;
            unsigned int CQ1_INT_SEL        :1;
        }bit_field;
        unsigned int reg_val;
    };
};

/**
*@brief  ISP top mux select configure struct
*/
struct stIspTopMuxSel {
    union {
        struct {
            unsigned int PAK_SEL        : 2;
            unsigned int UNP_SEL        : 1;
            unsigned int AA_SEL         : 1;
            unsigned int LCS_SEL        : 2;
            unsigned int SGG_SEL        : 2;
            unsigned int BIN_SEL        : 1;
            unsigned int rsv_9          : 1;
            unsigned int C02_SEL        : 2;
            unsigned int G2G_SEL        : 1;
            unsigned int GGM_SEL        : 1;
            unsigned int YUV_SEL        : 1;
            unsigned int CDP_SEL        : 1;
            unsigned int NR3O_SEL       : 1;
            unsigned int rsv_17         : 1;
            unsigned int PAK_SEL_EN     : 1;
            unsigned int UNP_SEL_EN     : 1;
            unsigned int AA_SEL_EN      : 1;
            unsigned int LCS_SEL_EN     : 1;
            unsigned int SGG_SEL_EN     : 1;
            unsigned int BIN_SEL_EN     : 1;
            unsigned int SOF_SEL_EN     : 1;
            unsigned int C02_SEL_EN     : 1;
            unsigned int G2G_SEL_EN     : 1;
            unsigned int GGM_SEL_EN     : 1;
            unsigned int YUV_SEL_EN     : 1;
            unsigned int CDP_SEL_EN     : 1;
            unsigned int NR3O_SEL_EN    : 1;
            unsigned int rsv_31         : 1;
        }bit_field;
        unsigned int reg_val;
    };
};

/**
*@brief  ISP top mux select 2 configure struct
*/
struct stIspTopMuxSel2 {
    union {
        struct {
            unsigned int CCL_SEL            : 2;
            unsigned int BIN_OUT_SEL        : 2;
            unsigned int IMGO_MUX           : 1;
            unsigned int FLKI_SOF_SEL       : 1;
            unsigned int IMG2O_MUX          : 1;
            unsigned int IMGCI_SOF_SEL      : 1;
            unsigned int PASS1_DONE_MUX     : 5;
            unsigned int PASS2_DONE_MUX     : 5;
            unsigned int CCL_SEL_EN         : 1;
            unsigned int BIN_OUT_SEL_EN     : 1;
            unsigned int IMGO_MUX_EN        : 1;
            unsigned int IMG2O_MUX_EN       : 1;
            unsigned int IMGCI_SOF_SEL_EN   : 1;
            unsigned int FLKI_SOF_SEL_EN    : 1;
            unsigned int LCEI_SOF_SEL_EN    : 1;
            unsigned int IMGI_MUX_EN        : 1;
            unsigned int IMGCI_MUX_EN       : 1;
            unsigned int LCEI_SOF_SEL       : 1;
            unsigned int LSCI_SOF_SEL       : 1;
            unsigned int LSCI_SOF_SEL_EN    : 1;
            unsigned int PASS1_DONE_MUX_EN  : 1;
            unsigned int PASS2_DONE_MUX_EN  : 1;
        }bit_field;
        unsigned int reg_val;
    };
};

/**
*@brief  ISP top sram mux configure struct
*/
struct stIspTopSramMuxCfg {
    union {
        struct {
            unsigned int SRAM_MUX_SCENARIO  : 3;
            unsigned int rsv_3              : 1;
            unsigned int SRAM_MUX_MODE      : 3;
            unsigned int SRAM_MUX_TPIPE     : 1;
            unsigned int SRAM_MUX_SET_EN    : 1;
            unsigned int rsv_9              : 1;
            unsigned int IMGO_SOF_SEL       : 1;
            unsigned int LCSO_SOF_SEL       : 1;
            unsigned int ESFKO_SOF_SEL      : 1;
            unsigned int AAO_SOF_SEL        : 1;
            unsigned int RGB_SOF_SEL        : 1;
            unsigned int rsv_15             : 1;
            unsigned int LCSO_SOF_SEL_EN    : 1;
            unsigned int ESFKO_SOF_SEL_EN   : 1;
            unsigned int AAO_SOF_SEL_EN     : 1;
            unsigned int RGB_SOF_SEL_EN     : 1;
            unsigned int GDMA_SEL_EN        : 1;
            unsigned int GDMA_SYNC_EN       : 1;
            unsigned int GDMA_REPEAT        : 1;
            unsigned int GDMA_IMGCI_SEL     : 1;
            unsigned int PREGAIN_SEL        : 1;
            unsigned int CAM_OUT_FMT2_EN    : 1;
            unsigned int CAM_OUT_FMT2       : 2;
            unsigned int SGG_HRZ_SEL        : 1;
            unsigned int rsv_29             : 1;
            unsigned int IMG2O_SOF_SEL      : 2;
        }bit_field;
        unsigned int reg_val;
    };
};

/**
*@brief  ISP top enable class
*/
class IspEnFunc 
{
public:
    struct stIspTopEnTbl        en_Top;
public:

    /**
      *@brief  Constructor
      */
    IspEnFunc()
    {en_Top.enable1 = 0;en_Top.enable2 = 0;en_Top.dma = 0;}
/*    {en_Raw.u.pipe=0;en_Rgb.u.pipe=0;en_Yuv.u.pipe=0;
     en_Cdp.u.pipe=0;en_DMA.u.pipe=0;};*/
};

/**
*@brief  ISP top interrupt class
*/
class IspEnInt 
{
public:
    struct stIspTopINT en_Int;
public:
    
    /**
      *@brief  Constructor
      */
    IspEnInt()
    {en_Int.int_en=0;en_Int.intb_en=0;en_Int.intc_en=0;en_Int.dma_int=0;en_Int.dmab_int=0;en_Int.dmac_int=0;}
};

/**
*@brief  ISP data source enum
*/
typedef enum  _IspInDataSrc_
{
    ISP_IN_DATA_SRC_SENSOR = 0,
    ISP_IN_DATA_SRC_MEM
}IspInDataSrc;


class IspFunction_B;
class IspEventThread;

/**
*@brief  ISP driver basic class
*/
class IspDrv_B
{
    public:

        /**
             *@brief  Constructor
             */
        IspDrv_B():
            m_pPhyIspDrv_bak(NULL),
            m_pPhyIspReg_bak(NULL),
            m_pCdpDrv(NULL)
            {}

        /**
             *@brief  Destructor
             */
        virtual ~IspDrv_B(){}
    public:
        static IspDrv*      m_pIspDrv;
        static isp_reg_t*   m_pIspReg;
    public:
        IspDrv*         m_pVirtIspDrv[ISP_DRV_CQ_NUM];  ///< virtual isp driver for CQ 
        isp_reg_t*      m_pVirtIspReg[ISP_DRV_CQ_NUM];  ///< virtual isp driver for CQ 
        IspDrv*         m_pPhyIspDrv_bak;
        isp_reg_t*      m_pPhyIspReg_bak;        
        CdpDrv*         m_pCdpDrv;      ///< cdp driver 
        TpipeDrv*       m_pTpipeDrv;    ///< tdri driver 
        IMemDrv*        m_pIMemDrv;     ///< imem driver      
        IspEventThread* m_pIspEventThread;  ///< ispEventThread
    public:
        static default_func default_setting_function[32];

        /**
             *@brief  hrz configure
             *@param[in] iHeight : input height
             *@param[in] resize : resize
             *@param[in] oSize : ouput height
             */
        static MINT32 cam_isp_hrz_cfg(MINT32 iHeight,MINT32 resize,MINT32 oSize);

        /**
             *@brief  cfa configure
             */
        static MINT32 cam_isp_cfa_cfg(void);

        /**
             *@brief  g2g configure
             */
        static MINT32 cam_isp_g2g_cfg(void);

        /**
             *@brief  dgm configure
             */
        static MINT32 cam_isp_dgm_cfg(void);

        /**
             *@brief  ggm configure
             */
        static MINT32 cam_isp_ggm_cfg(void);

        /**
             *@brief  c02 configure
             *@param[in] ysize : y size
             *@param[in] yofst : y offset
             *@param[in] mode : mode
             */
        static MINT32 cam_isp_c02_cfg(MINT32 ysize,MINT32 yofst,MINT32 mode);

        /**
             *@brief  c24 configure
             */
        static MINT32 cam_isp_c24_cfg(void);

        /**
             *@brief  g2c configure
             */
        static MINT32 cam_isp_g2c_cfg(void);

        /**
             *@brief  c42 configure
             */
        static MINT32 cam_isp_c42_cfg(void);

        /**
             *@brief  mfb configure
             */
        static MINT32 cam_isp_mfb_cfg(void);
};


/*/////////////////////////////////////////////////////////////////////////////
    CommandQ control
/////////////////////////////////////////////////////////////////////////////*/

    //pass2 fixed
    // bit
    #define CAM_CTL_EN1_C24_BIT               0x20000000  //0x4004
    #define CAM_CTL_EN1_MFB_BIT               0x10000000  //0x4004
    #define CAM_CTL_EN1_C02_BIT               0x08000000  //0x4004
    #define CAM_CTL_EN1_CFA_BIT               0x00200000  //0x4004
    #define CAM_CTL_EN1_HRZ_BIT               0x00000200  //0x4004
    #define CAM_CTL_EN1_AAA_GROP_BIT          0x00000040  //0x4004
    #define CAM_CTL_EN2_CDRZ_BIT              0x00020000  //0x4008
    #define CAM_CTL_EN2_G2C_BIT               0x00000001  //0x4008
    #define CAM_CTL_EN2_C42_BIT               0x00000002  //0x4008
    #define CAM_CTL_DMA_IMG2O_BIT             0x00000400  //0x400c
    #define CAM_CTL_DMA_IMGO_BIT              0x00000001  //0x400c
    #define CAM_CTL_DMA_AAO_BIT               0x00000020  //0x400c
    #define CAM_CTL_DMA_ESFKO_BIT             0x00000008  //0x400c
    #define CAM_CTL_DMA_FLKI_BIT              0x00000800  //0x400c
    #define CAM_CTL_DMA_LCSO_BIT              0x00000040  //0x400c

    #define CAM_CTL_FMT_SCENARIO_BIT          0x00000007  //0x4010
    #define CAM_CTL_FMT_SUB_MODE_BIT          0x00000038  //0x4010
    // register
    #define CAM_CTL_FIXED_DMA_EN_SET_PASS2    0x00000401  //0x400C /* IMGO,IMG2O */
    #define CAM_CTL_FIXED_FMT_SEL_SET_PASS2   0x0000007F  //0x4018 /* SCENARIO, SUB_MODE */
    //
//    #define CAM_CTL_EN1_FOR_TURN    0x00D008a8// TEST_MDPjust for RAW_POSTPROC 0x00C008a8 //SL TEST_MDP remove CLEAR GGM 0x07C008a8      //0x4004
//    #define CAM_CTL_EN2_FOR_TURN    0x0000003C      //0x4008
//    #define CAM_CTL_DMA_FOR_TURN    0x00001102      //0x400C
    //
    #define CAM_CTL_EN1_FOR_ISP     0x38303200      //0x4004 //org
//    #define CAM_CTL_EN1_FOR_ISP     0x3D203200//SL TEST_MDP 0x3D303200      //0x4004
    #define CAM_CTL_EN2_FOR_ISP     0x00020023      //0x4008  // NR3D control by isp
    #define CAM_CTL_DMA_EN_FOR_ISP  0x00006DE9      //0x400C
    #define CAM_CTL_FMT_SEL_FOR_ISP 0x0000FF77      //0x4010
    //#define CAM_CTL_SEL_FOR_ISP     0x00000000      //0x4018
    #define CAM_CTL_MUX_SEL_FOR_ISP         0xFFFFFFFF //0x4074
    #define CAM_CTL_MUX_SEL2_FOR_ISP        0xFFFFFFFF //0x4078
    #define CAM_CTL_SRAM_MUX_CFG_FOR_ISP    0xFFFFFFFF //0x407C
    #define CAM_CTL_PIX_ID_FOR_ISP  0x00000003      //0x401C
    //#define CAM_CTL_INT_EN_FOR_ISP  0x00000000      //0x4020
    //#define CAM_CTL_TPIPE_FOR_ISP    0x00000000      //0x4050
    //#define CAM_CTL_TCM_EN_FOR_ISP  0x00000000      //0x4054
    //
    #define CAM_CTL_EN1_FOR_CDP     0x00000000      //0x4004
    #define CAM_CTL_EN2_FOR_CDP     0x43A40000      //0x4008  // NR3D control by isp
//    #define CAM_CTL_EN2_FOR_CDP     0x43A60020      //0x4008

    #define CAM_CTL_DMA_EN_FOR_CDP  0x00286480      //0x400C
    #define CAM_CTL_FMT_SEL_FOR_CDP CAM_CTL_FMT_SEL_FOR_ISP      //0x4010
    //#define CAM_CTL_SEL_FOR_ISP     0x00000000      //0x4018
    #define CAM_CTL_MUX_SEL_FOR_CDP         0x00000000 //0x4074
    #define CAM_CTL_MUX_SEL2_FOR_CDP        0x00000000 //0x4078
    #define CAM_CTL_SRAM_MUX_CFG_FOR_CDP    0x00000000 //0x407C
    #define CAM_CTL_PIX_ID_FOR_CDP  CAM_CTL_PIX_ID_FOR_ISP      //0x401C
    //#define CAM_CTL_INT_EN_FOR_CDP  0x00000000      //0x4020
    //#define CAM_CTL_TPIPE_FOR_CDP    0x00000000      //0x4050
    //#define CAM_CTL_TCM_EN_FOR_CDP  0x00000000      //0x4054
    //set 1
    #define CAM_CTL_SEL_SET_1       0x80000000  //0x4018  bit31=1(set CQ1 related interrupt include all interrupt)
    #define CAM_CTL_SPARE3_SET_1    0x00000080  //0x406c  bit7=1(merge pass2, pass2b and pass2c)

    /* engine can be clear or not */
    #define CQ_CAM_CTL_CAN_BE_CLR_BITS(a,isp)       ((isp)?CAM_CTL_##a##_FOR_ISP : CAM_CTL_##a##_FOR_CDP)

    #define CQ_CAM_CTL_BIT_FIXED(_module_,_bit_,_en_) ((_en_)? (~CAM_CTL_##_module_##_##_bit_##_BIT) : 0xffffffff)

    // check for turning
    #define CQ_CAM_CTL_CHECK_TURNING_BITS(_a_,_en_turn_)       ((_en_turn_)?CAM_CTL_##_a_##_FOR_TURN : 0x00000000)

/**
*@brief  Isp Driver Shell class
*/
class IspDrvShell:virtual public IspDrv_B
{
    protected:

        /**
             *@brief  Destructor
             */
        virtual ~IspDrvShell() {};
    public:

        /**
             *@brief  Create Isp Driver Shell class object
             *@param[in] eScenarioID : scenario ID enum
             */
        static IspDrvShell* createInstance(NSImageio::NSIspio::EScenarioID eScenarioID = NSImageio::NSIspio::eScenarioID_VR);

        /**
             *@brief  Destory Isp Driver Shell class object
             */
        virtual void destroyInstance(void) = 0;

        /**
             *@brief  Initialize
             */
        virtual MBOOL init(void) = 0;

        /**
             *@brief  Uninitialize
             */
        virtual MBOOL uninit(void) = 0;

        /**
             *@brief  Get ISP physical driver
             */
        virtual IspDrv* getPhyIspDrv() = 0;

        /**
             *@brief  Get ISP physical register
             */
        virtual isp_reg_t*  getPhyIspReg() = 0;
        
        friend class IspFunction_B;
    private:

        /**
             *@brief  phy<->virt ISP switch
             */
        virtual MBOOL ispDrvSwitch2Virtual(MINT32 cq) = 0;
        //virtual MBOOL       ispDrvSwitch2Phy() = 0;
    public://commandQ operation

        /**
             *@brief  Command queue configure
             *@param[in] cq : cq enum
             */
        virtual MBOOL cam_cq_cfg(MINT32 cq) = 0;

        /**
             *@brief  Add module to command queue
             *@param[in] cq : cq enum
             *@param[in] moduleId : module enum
             */
        virtual MBOOL cqAddModule(ISP_DRV_CQ_ENUM cq, CAM_MODULE_ENUM moduleId) = 0;

        /**
             *@brief  Delete module to command queue
             *@param[in] cq : cq enum
             *@param[in] moduleId : module enum
             */
        virtual MBOOL cqDelModule(ISP_DRV_CQ_ENUM cq, CAM_MODULE_ENUM moduleId) = 0;

        /**
             *@brief  Set command queue trigger mode
             *@param[in] cq : cq 
             *@param[in] moduleId : trigger mode
             *@param[in] moduleId : trigger source
             */
        virtual MBOOL setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src) = 0;
        
        mutable Mutex   mLock;
        mutable Mutex   gLock;  // for multi-thread
};

/**
*@brief  Isp Function basicl class
*/
class IspFunction_B
{
private:
    //isp top camera base address
    static unsigned long    m_Isp_Top_Reg_Base_Addr;   //maybe virtual address
    //function base address
    unsigned long           m_Reg_Base_Addr;          //maybe virtual address
    mutable Mutex           mLock;   
protected://isp driver operation
    static IspDrv*          m_pIspDrv;
    static isp_reg_t*       m_pIspReg;
    static IspDrv*          m_pPhyIspDrv;
    static isp_reg_t*       m_pPhyIspReg;
public://isp driver operation
    int                     bypass;
    static IspDrvShell*     m_pIspDrvShell;
public:

    /**
      *@brief  Constructor
      */
    IspFunction_B(): m_Reg_Base_Addr(0),bypass(0){};

    /**
      *@brief  Destructor
      */
    virtual ~IspFunction_B(){};
public:

    /**
      *@brief  Check busy
      */
    virtual int checkBusy( unsigned long* param ) = 0;

    /**
      *@brief  Get ID
      *@return
      *- ID
      */
    virtual unsigned long id( void ) = 0;

    /**
      *@brief Name string of module
      */
    virtual const char* name_Str( void ) = 0;

    /**
      *@brief  Phy register base address, usually a fixed value
      */
    virtual unsigned long reg_Base_Addr_Phy( void ) = 0;

    /**
      *@brief  Register range
      */
    virtual unsigned long reg_Range( void ) = 0;

    /**
      *@brief  Check module is bypass or not
      */
    virtual int is_bypass( void ) {return bypass;}

    /**
      *@brief  Decimal ID number
      */
    unsigned long           dec_Id( void );

    /**
      *@brief  Get isp top register base addr
      *@return
      *- isp top register base addr
      */
    static unsigned long isp_Top_Reg_Base_Addr( void ) {   return m_Isp_Top_Reg_Base_Addr;   }

    /**
      *@brief  Get isp top register physical base addr
      *@return
      *- isp top register physical base addr
      */
    static unsigned long isp_Top_Reg_Base_Addr_Phy( void ) {   return MT6589_ISP_TOP_BASE;  }//Phy reg base address, usually a fixed value

    /**
      *@brief  Get isp top register range
      *@return
      *- isp top register range
      */
    static unsigned long isp_Top_Reg_Range( void )  {   return MT6589_ISP_TOP_REG_RANGE;  }

    /**
      *@brief  Get register base address
      *@return
      *- register base address
      */
    unsigned long reg_Base_Addr( void )  {   return m_Reg_Base_Addr;   }

    /**
      *@brief  Remap top register base address
      */
    static void remap_Top_Reg_Base_Addr( unsigned long new_addr )  {   m_Isp_Top_Reg_Base_Addr = new_addr;}

    /**
      *@brief  Remap register base address
      */
    void  remap_Reg_Base_Addr( unsigned long new_addr )  {   m_Reg_Base_Addr = new_addr; }
public: //isp driver operation

    /**
      *@brief  Set ISP driver shell
      */
    static void setIspDrvShell(IspDrvShell* pIspDrvShell){m_pIspDrvShell = pIspDrvShell;m_pIspDrv = pIspDrvShell->getPhyIspDrv();
            m_pIspReg = pIspDrvShell->getPhyIspReg();m_pPhyIspDrv = pIspDrvShell->getPhyIspDrv();m_pPhyIspReg = pIspDrvShell->getPhyIspReg();}

    /**
      *@brief  Wait irq
      *@param[in] WaitIrq : ISP_DRV_WAIT_IRQ_STRUCT
      */
    inline int waitIrq( ISP_DRV_WAIT_IRQ_STRUCT WaitIrq ){return (int)m_pPhyIspDrv->waitIrq( WaitIrq );}

    /**
      *@brief  Write register
      *@param[in] offset : register offset
      *@param[in] value : write value
      */
    inline int writeReg( unsigned long offset, unsigned long value ){return (int)m_pPhyIspDrv->writeReg(offset,value);}

    /**
      *@brief  Read register
      *@param[in] offset : register offset
      *@return
      *- register value
      */
    inline unsigned long readReg( unsigned long offset ){return (unsigned long)m_pPhyIspDrv->readReg(offset);}

    /**
      *@brief  Read irq
      *@param[in] pReadIrq : ISP_DRV_READ_IRQ_STRUCT
      */
    inline int readIrq(ISP_DRV_READ_IRQ_STRUCT *pReadIrq){return (int)m_pPhyIspDrv->readIrq(pReadIrq);}

    /**
      *@brief  Check irq
      *@param[in] CheckIrq : ISP_DRV_CHECK_IRQ_STRUCT
      */
    inline int checkIrq(ISP_DRV_CHECK_IRQ_STRUCT CheckIrq){return (int)m_pPhyIspDrv->checkIrq(CheckIrq);}

    /**
      *@brief  Clear irq
      *@param[in] ClearIrq : ISP_DRV_CLEAR_IRQ_STRUCT
      */
    inline int clearIrq(ISP_DRV_CLEAR_IRQ_STRUCT ClearIrq){return (int)m_pPhyIspDrv->clearIrq(ClearIrq);}

    /**
      *@brief  Dump register
      */
    inline int dumpRegister( int mode ){return (int)m_pPhyIspDrv->dumpReg();}

    mutable Mutex           queHwLock;
    mutable Mutex           queSwLock;

    /**
      *@brief  Check if virtual isp driver or not
      */
    MBOOL ispDrvSwitch2Virtual(MINT32 cq);
    //MBOOL                   ispDrvSwitch2Phy();

public:
    int             config( void );
    int             enable( void* pParam  );
    int             disable( void );
    int             write2CQ(int cq);
    int             setZoom( void );
protected:
    virtual int     _config( void ) = 0;
    virtual int     _enable( void* pParam  ) = 0;
    virtual int     _disable( void ) = 0;
    virtual int     _write2CQ( int cq ) {return 0;}
    virtual int     _setZoom( void ) {return 0;}
};

/**
*@brief  Input DMA basic class
*/
class DMAI_B:public IspFunction_B
{
public:/*[member veriable]*/
    /*
        ->image input(imgi,imgci,nr3i,lsci,flki,lcei,vipi,vip2i)
                IspMemBuffer.base_pAddr          ->base_pAddr
                IspMemBuffer.ofst_addr           ->ofst_addr
                IspSize.w                              -> XSIZE
                IspSize.h                               -> YSIZW
                IspSize.stride                          ->stride
        ->others(TDR,...)
                IspMemBuffer.base_pAddr         ->base_pAddr
                IspMemBuffer.ofst_addr          ->ofst_addr
                IspSize.w                              -> XSIZE
        ->others(CQ,...)
                IspMemBuffer.base_pAddr          ->base_pAddr
                IspSize.w                              -> XSIZE
        */

    IspDMACfg   dma_cfg;
/*
    IspMemBuffer    img_buf;
    IspSize         size;
    IspRect         crop;
    int             pixel_byte;
*/
    IspColorFormat  color_format;
    int CQ;
    MUINT32 sceID;
    
private:    
    
public:

    /**
      *@brief  Constructor
      */
    DMAI_B() {dma_cfg.pixel_byte = 1;};
public: /*[IspFunction_B]*/

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param ){return 0;}
private: /*[IspFunction_B]*/

    /**
      *@brief  Configure
      */
    virtual int _config( void );

    /**
      *@brief  Enable
      */
    virtual int _enable( void* pParam ){return 0;}

    /**
      *@brief  Disable
      */
    virtual int _disable( void );

    /**
      *@brief  Write to command queue
      */
    virtual int _write2CQ( int cq );
public: /*[DMAI_B]*/

private: /*[DMAI_B]*/

};

/**
*@brief  Output DMA basic class
*/
class DMAO_B:public IspFunction_B
{
public:/*[member veriable]*/
    /*
        ->image ouput(imgo,img2o,)
                IspMemBuffer.base_pAddr           ->base_pAddr
                IspMemBuffer.ofst_addr            ->ofst_addr
                IspSize.w                              -> XSIZE
                IspSize.h                               -> YSIZE
                IspSize.stride                          ->stride
                IspRect.x                               ->crop xoffset
                IspRect.y                               ->crop yoffset
        ->others(lcso,ESFKO(FLKO),AAO,NR3O,...)
                IspMemBuffer.base_pAddr          ->base_pAddr
                IspMemBuffer.ofst_addr            ->ofst_addr
                IspSize.w                              -> XSIZE
                IspSize.h                               -> YSIZE
                IspSize.stride                          ->stride
        ->others(EISO,AFO...)
                IspMemBuffer.base_pAddr          ->base_pAddr
                IspSize.w                              -> XSIZE
        */

    IspDMACfg   dma_cfg;
    int CQ;

/*
    IspMemBuffer    img_buf;
    IspSize         size;
    IspRect         crop;
    int             pixel_byte;
*/
private:

public:

    /**
      *@brief  Constructor
      */
    DMAO_B() {dma_cfg.pixel_byte = 1;};

public: /*[IspFunction_B]*/
    //virtual int is_bypass( void )                   {   return 0;       }

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param ){return 0;}

private:/*[IspFunction_B]*/

    /**
      *@brief  Configure
      */
    virtual int _config( void );

    /**
      *@brief  Enable
      */
    virtual int _enable( void* pParam ){return 0;}

    /**
      *@brief  Disable
      */
    virtual int _disable( void ){return 0;}

    /**
      *@brief  Write to command queue
      */
    virtual int _write2CQ( int cq );

public: /*[DMAO_B]*/


private: /*[DMAO_B]*/


};

/**
*@brief  ISP tuning class
*/
class ISP_TURNING_CTRL:public IspFunction_B
{
public:
    int CQ;
    int isApplyTurn;

public:

    /**
      *@brief  Constructor
      */
    ISP_TURNING_CTRL():
        isApplyTurn(0),
        CQ(CAM_ISP_CQ_NONE){};

public:

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param ){return 0;}

public:

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )                    {   return ISP_TURNING;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )              {   return "ISP_TURNING";}

    /**
      *@brief  Get register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )     {   return (unsigned long)NULL;  }

    /**
      *@brief  Get register range
      */
    virtual unsigned long reg_Range( void )             {   return (unsigned long)NULL;  }
protected:

    /**
      *@brief  Configure
      */
    virtual int _config( void );

    /**
      *@brief  Enable
      */
    virtual int _enable( void* pParam ){return 0;}

    /**
      *@brief  Disable
      */
    virtual int _disable( void ){ return 0;}

    /**
      *@brief  Write to command queue
      */
    virtual int _write2CQ( int cq ){ return 0;}

    /**
      *@brief  Set zoom
      */
    virtual int _setZoom( void ){ return 0;}
};



/**
*@brief  ISP TOP control class
*/
class ISP_TOP_CTRL : public IspFunction_B
{
public:
    struct stIspTopEnTbl en_Top;
    struct stIspTopINT ctl_int;
    struct stIspTopFmtSel fmt_sel;
    struct stIspTopSel ctl_sel;
    struct stIspTopMuxSel ctl_mux_sel;
    struct stIspTopMuxSel2 ctl_mux_sel2;
    struct stIspTopSramMuxCfg ctl_sram_mux_cfg;
    static int pix_id;
    int path;
    int isConcurrency;
    int isEn1AaaGropStatusFixed;
    int isEn1C24StatusFixed;
    int isEn1C02StatusFixed;
    int isEn1CfaStatusFixed;
    int isEn1HrzStatusFixed;
    int isEn1MfbStatusFixed;
    int isEn2CdrzStatusFixed;
    int isEn2G2cStatusFixed;
    int isEn2C42StatusFixed;
    int isImg2oStatusFixed;
    int isImgoStatusFixed;
    int isAaoStatusFixed;
    int isEsfkoStatusFixed;
    int isFlkiStatusFixed;
    int isLcsoStatusFixed;
    int isApplyTurn;
    int isShareDmaCtlByTurn;
    int b_continuous;
    int tcm_en;
    int tpipe_w;
    int tpipe_h;
    int CQ;
    int isIspOn;
public:

    /**
      *@brief  Constructor
      */
    ISP_TOP_CTRL():
        path(0),
        isConcurrency(0),
        isEn1AaaGropStatusFixed(0),
        isEn1C24StatusFixed(0),
        isEn1C02StatusFixed(0),
        isEn1CfaStatusFixed(0),
        isEn1HrzStatusFixed(0),
        isEn1MfbStatusFixed(0),
        isEn2CdrzStatusFixed(0),
        isEn2G2cStatusFixed(0),
        isEn2C42StatusFixed(0),
        isImg2oStatusFixed(0),
        isImgoStatusFixed(0),
        isAaoStatusFixed(0),
        isEsfkoStatusFixed(0),
        isFlkiStatusFixed(0),
        isLcsoStatusFixed(0),
        isApplyTurn(0),
        isShareDmaCtlByTurn(0),
        b_continuous(0),
        tcm_en(0),
        tpipe_w(0),
        tpipe_h(0),
        CQ(CAM_ISP_CQ_NONE),
        isIspOn(0)
        {ctl_mux_sel.reg_val=0;ctl_mux_sel2.reg_val=0;ctl_sram_mux_cfg.reg_val=0;};

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  { return ISP_TOP; }

    /**
      *@brief  Return module name
      */
    virtual const char* name_Str( void )  { return "ISP_TOP";}

    /**
      *@brief  Return TOP register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void ) {  return MT6589_ISP_TOP_BASE;}

    /**
      *@brief  Return TOP register range
      */
    virtual unsigned long reg_Range( void )  {   return MT6589_ISP_TOP_REG_RANGE; }
    //virtual int is_bypass( void )                       {   return 0;       }

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param );

    /**
      *@brief  Set command trigger mode
      *@param[in] cq : CQ
      *@param[in] mode : trigger mode
      *@param[in] trig_src : trigger source
      */
    virtual MBOOL setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src);

    /**
      *@brief  kk test (wil be removed in the future)
      */
    MBOOL tmpSimulatePath(void);

    /**
      *@brief  Return Isp register
      */
    inline isp_reg_t* getPhyIspReg(){return this->m_pIspReg;}
    
protected:

    /**
      *@brief  Configure
      */
    virtual int _config( void );

    /**
      *@brief  Enable
      */
    virtual int _enable( void* pParam  );

    /**
      *@brief  Disable
      */
    virtual int _disable( void );

    /**
      *@brief  Write to command queue
      */
    virtual int _write2CQ( int cq );
};

/**
*@brief  ISP RAW pipe control class
*/
class ISP_RAW_PIPE:public IspFunction_B
{
public:
    unsigned int enable1;
    int src_img_w;
    int src_img_h;
    int cdrz_in_w;
    int CQ;
public:

    /**
      *@brief  Constructor
      */
    ISP_RAW_PIPE():
        enable1(0),src_img_w(0),src_img_h(0),CQ(CAM_ISP_CQ_NONE){};

public:

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param ){return 0;}

public:

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  {   return ISP_RAW;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  {   return "ISP_RAW";}

    /**
      *@brief Return RAW pipe register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  {   return MT6589_ISP_RAW_BASE;  }

    /**
      *@brief  Return  RAW pipe register range
      */
    virtual unsigned long reg_Range( void )  {   return MT6589_ISP_RAW_REG_RANGE;  }
protected:

    /**
      *@brief  Configure
      */
    virtual int _config( void );

    /**
      *@brief  Enable
      */
    virtual int _enable( void* pParam ){return 0;}

    /**
      *@brief  Disable
      */
    virtual int _disable( void ){return 0;}

    /**
      *@brief  Write to command queue
      */
    virtual int _write2CQ( int cq );
};

/**
*@brief  ISP RGB pipe control class
*/
class ISP_RGB_PIPE:public IspFunction_B
{
public:
    unsigned int enable1;
    int src_img_h;
    int CQ;
public:
    
    /**
      *@brief  Constructor
      */
    ISP_RGB_PIPE():enable1(0),src_img_h(0),CQ(CAM_ISP_CQ_NONE){};

    /**
      *@brief  Destructor
      */
    virtual ~ISP_RGB_PIPE(){};

public:

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param ){return 0;}

public:

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void ) {   return ISP_RGB;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  {   return "ISP_RGB";}

    /**
      *@brief Return RGB pipe register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void ) {   return MT6589_ISP_RGB_BASE;  }

    /**
      *@brief  Return  RGB pipe register range
      */
    virtual unsigned long reg_Range( void ) {   return MT6589_ISP_RGB_REG_RANGE;  }
protected:

    /**
      *@brief  Configure
      */
    virtual int _config( void );

    /**
      *@brief  Enable
      */
    virtual int _enable( void* pParam ){return 0;}

    /**
      *@brief  Disable
      */
    virtual int _disable( void ){return 0;}

    /**
      *@brief  Write to command queue
      */
    virtual int _write2CQ( int cq );
};

/**
*@brief  ISP YUV pipe control class
*/
class ISP_YUV_PIPE:public IspFunction_B
{
public:
    unsigned int enable2;
    int CQ;
public:

    /**
      *@brief  Constructor
      */
    ISP_YUV_PIPE():enable2(0),CQ(CAM_ISP_CQ_NONE){};

    /**
      *@brief  Destructor
      */
    ~ISP_YUV_PIPE(){};

public:

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param ){return 0;}

public:

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void ) {   return ISP_YUV;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  {   return "ISP_YUV";}

    /**
      *@brief Return YUV pipe register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void ) {   return MT6589_ISP_YUV_BASE;  }

    /**
      *@brief  Return  YUV pipe register range
      */
    virtual unsigned long reg_Range( void ) {   return MT6589_ISP_YUV_REG_RANGE;  }
protected:

    /**
      *@brief  Configure
      */
    virtual int _config( void );

    /**
      *@brief  Enable
      */
    virtual int _enable( void* pParam ){return 0;}

    /**
      *@brief  Disable
      */
    virtual int _disable( void ){return 0;}

    /**
      *@brief  Write to command queue
      */
    virtual int _write2CQ( int cq );
};

/**
*@brief  ISP CDP pipe control class
*/
class CAM_CDP_PIPE:public IspFunction_B
{
public:
    int cdrz_filter;
    IspSize cdrz_in;
    IspSize cdrz_out;
    IspRect cdrz_crop;
    IspSize prz_in;
    IspSize prz_out;
    IspRect prz_crop;
    IspSize curz_in;
    IspSize curz_out;
    IspRect curz_crop;
    CdpRotDMACfg vido_out;
    CdpRotDMACfg dispo_out;
    int tpipe_w;
    int disp_vid_sel;
    int conf_cdrz;
    int conf_rotDMA;
    int CQ;
    unsigned int enable2;
    unsigned int dma_enable;
    int path;
    CDP_DRV_MODE_ENUM tpipeMode;

public:

    /**
      *@brief  Contructor
      */
    CAM_CDP_PIPE():conf_cdrz(0),conf_rotDMA(0),CQ(CAM_ISP_CQ_NONE){};
public:

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param ){return 0;}

public:

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  {   return ISP_CDP;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )   {   return "ISP_CDP";}

    /**
      *@brief Return CDP pipe register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  {   return MT6589_ISP_CDP_BASE;  }

    /**
      *@brief  Return  CDP pipe register range
      */
    virtual unsigned long reg_Range( void )  {   return MT6589_ISP_CDP_REG_RANGE;  }

    /**
      *@brief  Create MdpMgr object
      */
    MBOOL createMdpMgr();    
protected:

    /**
      *@brief  Configure
      */
    virtual int _config( void );

    /**
      *@brief  Enable
      */
    virtual int _enable( void* pParam ){return 0;}

    /**
      *@brief  Disable
      */
    virtual int _disable( void );

    /**
      *@brief  Write to command queue
      */
    virtual int _write2CQ( int cq );

    /**
      *@brief  Set zoom
      */
    virtual int _setZoom( void );
};

/**
*@brief  ISP TDRI pipe control class
*/
class CAM_TDRI_PIPE:public IspFunction_B
{
public:
    int enTdri;
    TdriDrvCfg tdri;
    int CQ;
/*
    int cdrz_filter;
    IspSize cdrz_in;
    IspSize cdrz_out;
    IspRect cdrz_crop;
    IspSize prz_in;
    IspSize prz_out;
    IspRect prz_crop;
    IspSize curz_in;
    IspSize curz_out;
    IspRect curz_crop;
    CdpRotDMACfg vido_out;
    CdpRotDMACfg dispo_out;
    int disp_vid_sel;
    int conf_cdrz;
    int conf_rotDMA;
    int CQ;
    unsigned int enable2;
    unsigned int dma_enable;
*/

public:

    /**
      *@brief  Constructor
      */
    CAM_TDRI_PIPE():CQ(CAM_ISP_CQ_NONE){};

public:

    /**
      *@brief  Check busy or not
      */
    virtual int checkBusy( unsigned long* param ){return 0;}

public:

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )   {   return ISP_TDRI;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )   {   return "ISP_TDRI";}

    /**
      *@brief Return TDRI pipe register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  {   return MT6589_DMA_TDRI_BASE;  }

    /**
      *@brief  Return TDRI pipe register range
      */
    virtual unsigned long reg_Range( void )  {   return MT6589_DMA_TDRI_REG_RANGE;  }

    /**
      *@brief  Get NR3D TOP
      *@param[in] ispCq : cq
      *@param[in] pEn : enable
      */
    virtual MBOOL getNr3dTop(MINT32 ispCq,MUINT32 *pEn);
protected:

    /**
      *@brief  Configure
      */
    virtual int _config( void );
	  
	/**
      *@brief  Enable
      */
	virtual int _enable( void* pParam ){return 0;}
    
	/**
      *@brief  Disable
      */
	virtual int _disable( void ){ return 0;}
    
	/**
      *@brief  Write to command queue
      */
	virtual int _write2CQ( int cq );
    
	/**
      *@brief  Set zoom
      */
    virtual int _setZoom( void ){ return 0;}    
};



/**
*@brief DMA IMGI control class
*/
class DMA_IMGI:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_IMGI(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void ) {  return   ISP_DMA_IMGI;  }
	  
	/**
      *@brief  Return module name
      */
	virtual const char*   name_Str( void )  {  return   "ISP_DMA_IMGI";}
    
	/**
      *@brief Return DMA IMGI register physical base address
      */
	virtual unsigned long reg_Base_Addr_Phy( void ) { return     MT6589_DMA_IMGI_BASE;  }
    
	/**
      *@brief  Return DMA IMGI register range
      */
    virtual unsigned long reg_Range( void )  {  return     MT6589_DMA_IMGI_REG_RANGE; }    
protected:

};

/**
*@brief DMA IMGCI control class
*/
class DMA_IMGCI:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_IMGCI(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )   {  return   ISP_DMA_IMGCI;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  {  return  "ISP_DMA_IMGCI";}

    /**
      *@brief Return DMA IMGCI register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  {  return  MT6589_DMA_IMGCI_BASE; }

    /**
      *@brief  Return DMA IMGCI register range
      */
    virtual unsigned long reg_Range( void )  { return  MT6589_DMA_IMGCI_REG_RANGE;  }
protected:

};

/**
*@brief  DMA FLKI control class
*/
class DMA_FLKI:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_FLKI(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  {  return  ISP_DMA_FLKI;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return   "ISP_DMA_FLKI";}

    /**
      *@brief Return DMA FLKI register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_FLKI_BASE;     }

    /**
      *@brief  Return DMA FLKI register range
      */
    virtual unsigned long reg_Range( void )   {  return   MT6589_DMA_FLKI_REG_RANGE;     }
protected:
};

/**
*@brief  DMA LSCI control class
*/
class DMA_LSCI:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_LSCI(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  { return  ISP_DMA_LSCI;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return  "ISP_DMA_LSCI";}

    /**
      *@brief Return DMA LSCI register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_LSCI_BASE; }

    /**
      *@brief  Return DMA LSCI register range
      */
    virtual unsigned long reg_Range( void )  { return  MT6589_DMA_LSCI_REG_RANGE; }
protected:
};

/**
*@brief  DMA LSEI control class
*/
class DMA_LCEI:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_LCEI(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  { return  ISP_DMA_LCEI;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return  "ISP_DMA_LCEI";}

    /**
      *@brief Return DMA LSEI register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_LCEI_BASE;  }

    /**
      *@brief  Return  DMA LSEI register range
      */
    virtual unsigned long reg_Range( void )  {  return  MT6589_DMA_LCEI_REG_RANGE;  }
protected:
};

/**
*@brief DMA VIPI control class
*/
class DMA_VIPI:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_VIPI(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  { return  ISP_DMA_VIPI;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )   { return  "ISP_DMA_VIPI";}

    /**
      *@brief Return DMA VIPI register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void ) { return  MT6589_DMA_VIPI_BASE; }

    /**
      *@brief  Return DMA VIPI register range
      */
    virtual unsigned long reg_Range( void )  { return  MT6589_DMA_VIPI_REG_RANGE; }
protected:
};

/**
*@brief  DMA VIP2I control class
*/
class DMA_VIP2I:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_VIP2I(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )   { return  ISP_DMA_VIP2I;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )   { return   "ISP_DMA_VIP2I";}

    /**
      *@brief Return DMA VIP2I register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_VIP2I_BASE; }

    /**
      *@brief  Return DMA VIP2I register range
      */
    virtual unsigned long reg_Range( void )  {  return  MT6589_DMA_VIP2I_REG_RANGE;  }
protected:
};

/**
*@brief  DMA TDRI control class
*/
class DMA_TDRI:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_TDRI() {};
public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  {  return     ISP_DMA_TDRI;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  {  return  "ISP_DMA_TDRI";}

    /**
      *@brief Return DMA TDRI register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return   MT6589_DMA_TDRI_BASE;     }

    /**
      *@brief  Return DMA TDRI register range
      */
    virtual unsigned long reg_Range( void )   { return  MT6589_DMA_TDRI_REG_RANGE;     }
protected:
    //virtual int _config( void );
    //virtual int _write2CQ( int cq );
};

/**
*@brief  DMA CQ control class
*/
class DMA_CQ:public DMAI_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_CQ(){};
public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )   { return  ISP_DMA_CQI;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return  "ISP_DMA_CQI";}

    /**
      *@brief Return DMA CQ register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_CQI_BASE;  }

    /**
      *@brief  Return DMA CQ register range
      */
    virtual unsigned long reg_Range( void )   {  return  MT6589_DMA_CQI_REG_RANGE; }
protected:
    //virtual int _enable( void* pParam );
    //virtual int _disable( void );
};


/**
*@brief  DMA IMGO control class
*/
class DMA_IMGO:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_IMGO(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )   { return  ISP_DMA_IMGO;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return  "ISP_DMA_IMGO";}

    /**
      *@brief Return DMA IMGO register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_IMGO_BASE; }

    /**
      *@brief  Return DMA IMGO register range
      */
    virtual unsigned long reg_Range( void )   {  return   MT6589_DMA_IMGO_REG_RANGE;  }
protected:
};

/**
*@brief  DMA IMG2O control class
*/
class DMA_IMG2O:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_IMG2O(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  { return  ISP_DMA_IMG2O;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return  "ISP_DMA_IMG2O";}

    /**
      *@brief Return DMA IMG2O register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_IMG2O_BASE; }

    /**
      *@brief  Return DMA IMG2O register range
      */
    virtual unsigned long reg_Range( void )  {  return  MT6589_DMA_IMG2O_REG_RANGE;  }
protected:
};

/**
*@brief  DMA LCSO control class
*/
class DMA_LCSO:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_LCSO(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  { return  ISP_DMA_LCSO;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )   { return  "ISP_DMA_LCSO";}

    /**
      *@brief Return DMA LCSO register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_LCSO_BASE;     }

    /**
      *@brief  Return DMA LCSO register range
      */
    virtual unsigned long reg_Range( void )   { return  MT6589_DMA_LCSO_REG_RANGE;     }
protected:
};

/**
*@brief  DMA AAO control class
*/
class DMA_AAO:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_AAO(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  { return  ISP_DMA_AAO; }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return   "ISP_DMA_AAO";}

    /**
      *@brief Return DMA AAO register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_AAO_BASE; }

    /**
      *@brief  Return DMA AAO register range
      */
    virtual unsigned long reg_Range( void )  { return  MT6589_DMA_AAO_REG_RANGE;  }
protected:
};

/**
*@brief  DMA ESFKO control class
*/
class DMA_ESFKO:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_ESFKO(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )   { return   ISP_DMA_ESFKO;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return  "ISP_DMA_ESFKO";}

    /**
      *@brief Return DMA ESFKO register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  {  return  MT6589_DMA_ESFKO_BASE; }

    /**
      *@brief  Return DMA ESFKO register range
      */
    virtual unsigned long reg_Range( void ) {  return  MT6589_DMA_ESFKO_REG_RANGE; }
protected:
};


/**
*@brief  DMA AFO control class
*/
class DMA_AFO:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_AFO(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  {  return  ISP_DMA_AFO;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )   { return  "ISP_DMA_AFO";}

    /**
      *@brief Return DMA AFO register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_AFO_BASE; }

    /**
      *@brief  Return DMA AFO register range
      */
    virtual unsigned long reg_Range( void )   { return  MT6589_DMA_AFO_REG_RANGE;  }
protected:
};

/**
*@brief  DMA EISO control class
*/
class DMA_EISO:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_EISO(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void ) {  return  ISP_DMA_EISO;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  { return  "ISP_DMA_EISO";}

    /**
      *@brief Return DMA EISO register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_EISO_BASE; }

    /**
      *@brief  Return DMA EISO register range
      */
    virtual unsigned long reg_Range( void )  { return  MT6589_DMA_EISO_REG_RANGE; }
protected:
};

/**
*@brief  DMA DISPO control class
*/
class DMA_DISPO:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_DISPO(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  { return   ISP_DMA_DISPO;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )   { return   "ISP_DMA_DISPO";}

    /**
      *@brief Return DMA DISPO  register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_DISPO_BASE; }

    /**
      *@brief  Return DMA DISPO  register range
      */
    virtual unsigned long reg_Range( void )   { return  MT6589_DMA_DISPO_REG_RANGE; }
protected:
};

/**
*@brief  DMA VIDO control class
*/
class DMA_VIDO:public DMAO_B
{
public:

    /**
      *@brief  Constructor
      */
    DMA_VIDO(){};

public:/*[IspFunction_B]*/

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )   { return  ISP_DMA_VIDO;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )   { return  "ISP_DMA_VIDO";}

    /**
      *@brief Return DMA VIDO  register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  { return  MT6589_DMA_VIDO_BASE; }

    /**
      *@brief  Return DMA VIDO  register range
      */
    virtual unsigned long reg_Range( void )  { return  MT6589_DMA_VIDO_REG_RANGE; }
protected:
};


/*/////////////////////////////////////////////////////////////////////////////
    ISP_BUF_CTRL
  /////////////////////////////////////////////////////////////////////////////*/
//
#if 0
typedef enum
{
    _imgi_  = 0,
    _imgci_,    // 1
    _vipi_ ,    // 2
    _vip2i_,    // 3
    _imgo_,     // 4
    _img2o_,    // 5
    _dispo_,    // 6
    _vido_,     // 7
    _fdo_,      // 8
    _rt_dma_max_
}_isp_dma_enum_;
#endif

/**
*@brief  buffer status enum
*/
typedef enum
{
    ISP_BUF_EMPTY = 0,
    ISP_BUF_FILLED,
}ISP_BUF_STATUS;

#if 0
typedef enum
{
    ISP_BUF_TYPE_PMEM    = BUF_TYPE_PMEM,
    ISP_BUF_TYPE_STD_M4U = BUF_TYPE_STD_M4U,
    ISP_BUF_TYPE_ION     = BUF_TYPE_ION,
}ISP_BUF_TYPE;
#endif

/**
*@brief  buffer info struct
*/
struct stISP_BUF_INFO
{
    ISP_BUF_STATUS          status;
    MUINT32                 base_vAddr;
    MUINT32                 base_pAddr;
    MUINT32                 size;
    MUINT32                 memID;
    MINT32                  bufSecu;
    MINT32                  bufCohe;
    MUINT64                 timeStampS;
    MUINT32                 timeStampUs;
    MVOID                   *private_info;
    struct stISP_BUF_INFO   *next;

    /**
      *@brief  Constructor
      */
    stISP_BUF_INFO(
        ISP_BUF_STATUS          _status = ISP_BUF_EMPTY,
        MUINT32                 _base_vAddr = 0,
        MUINT32                 _base_pAddr = 0,
        MUINT32                 _size = 0,
        MUINT32                 _memID = -1,
        MINT32                  _bufSecu = 0,
        MINT32                  _bufCohe = 0,
        MUINT64                 _timeStampS = 0,
        MUINT32                 _timeStampUs = 0,
        MVOID                   *_private_info = NULL,
        struct stISP_BUF_INFO   *_next = 0)
    : status(_status)
    , base_vAddr(_base_vAddr)
    , base_pAddr(_base_pAddr)
    , size(_size)
    , memID(_memID)
    , bufSecu(_bufSecu)
    , bufCohe(_bufCohe)
    , timeStampS(_timeStampS)
    , timeStampUs(_timeStampUs)
    , private_info(_private_info)
    , next(_next)
    {}
};

/*************************************************
*************************************************/
//
typedef list<stISP_BUF_INFO> ISP_BUF_INFO_L;

/**
*@brief  buffer list struct
*/
typedef struct  _isp_buf_list_{
    MUINT32         filledCnt;            //  fill count
    ISP_BUF_INFO_L  bufInfoList;
}stISP_BUF_LIST;

/**
*@brief  filled buffer list struct
*/
typedef struct  _isp_filled_buf_list_{
    ISP_BUF_INFO_L  *pBufList;
}stISP_FILLED_BUF_LIST;

/**
*@brief  Buffer control class
*/
class ISP_BUF_CTRL : public IspFunction_B
{
friend class IspDrvShellImp;
friend class ISP_TOP_CTRL;

private:
    static stISP_BUF_LIST m_hwbufL[_rt_dma_max_];
    static stISP_BUF_LIST m_swbufL[_rt_dma_max_];
public:
    int path;
public:

    /**
      *@brief  Constructor
      */
    ISP_BUF_CTRL():path(0)
        {/*memset(&m_hwBuf,0x00,sizeof(_isp_buf_status_));
         memset(&m_swBuf,0x00,sizeof(_isp_buf_status_));
         memset(m_Buffer,0x00,sizeof(m_Buffer));*/         };

    /**
      *@brief  Return ID
      */
    virtual unsigned long id( void )  {   return ISP_BUFFER;  }

    /**
      *@brief  Return module name
      */
    virtual const char*   name_Str( void )  {   return "ISP_BUFFER";}

    /**
      *@brief Return buffer control register physical base address
      */
    virtual unsigned long reg_Base_Addr_Phy( void )  {   return (unsigned long)NULL;  }

    /**
      *@brief  Return buffer control  register range
      */
    virtual unsigned long reg_Range( void )   {   return (unsigned long)NULL;  }

    /**
      *@brief Check bust or not
      */
    virtual int checkBusy( unsigned long* param ) {return 0;}

    /**
      *@brief Initialize
      */
    virtual int init( MUINT32 dmaChannel );

    /**
      *@brief Wait buffer ready
      */
    virtual MBOOL waitBufReady( MUINT32 dmaChannel );

    /**
      *@brief Append new buffer to the end of hwBuf list
      */
    virtual MINT32 enqueueHwBuf( MUINT32 dmaChannel, stISP_BUF_INFO bufInfo );

    /**
      *@brief Set buffer FILLED type and move filled Hw buffer to sw the end of list
      */
    virtual MINT32 dequeueHwBuf( MUINT32 dmaChannel);

    /**
      *@brief  Delete all swBuf list after inform caller
      */
    virtual MINT32 dequeueSwBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufList );

    /**
      *@brief Get 1st NOT filled HW buffer address
      */
    virtual MUINT32 getCurrHwBuf( MUINT32 dmaChannel );

    /**
      *@brief Get 2nd NOT filled HW buffer address
      */
    virtual MUINT32 getNextHwBuf( MUINT32 dmaChannel );

    /**
      *@brief Free single physical buffer
      */
    virtual MUINT32 freeSinglePhyBuf(
        stISP_BUF_INFO bufInfo);

    /**
      *@brief Free all physical buffer
      */
    virtual MUINT32 freeAllPhyBuf( void );

private:

    /**
      *@brief Get DMA buffer index
      */
    int getDmaBufIdx( MUINT32 dmaChannel );

    /**
      *@brief 82 tpipe function 
      */
    int getMdpFmtEnum( MUINT32 fmtEnum );

    /**
      *@brief Debug usage
      */
    int debugPrint( MUINT32 dmaChannel );
protected:

    /**
      *@brief  Configure
      */
    virtual int     _config( void ) {return 0;}

    /**
      *@brief  Enable
      */
    virtual int     _enable( void* pParam  ) {return 0;}

    /**
      *@brief  Disable
      */
    virtual int     _disable( void ) {return 0;}

    /**
      *@brief  Write to command queue
      */
    virtual int     _write2CQ( int cq ) {return 0;}

    /**
      *@brief Set zoom
      */
    virtual int     _setZoom( void ) {return 0;}

};


/**
*@brief  IspEventThreadl class
*/
class IspEventThread
{
    protected:

        /**
            *@brief Constructor
            */
        IspEventThread(){};

        /**
            *@brief Destructor
            */
        virtual ~IspEventThread() {};

    public:

        /**
            *@brief Create IspEventThread object
            */
        static  IspEventThread*  createInstance(IspDrv* pIspDrv);

    public:

        /**
            *@brief Initialize
            */
        virtual MBOOL   init(void) = 0;

        /**
            *@brief Uninitialize
            */
        virtual MBOOL   uninit(void) = 0;

};


#endif /*__ISP_FUNCTION_H__*/


