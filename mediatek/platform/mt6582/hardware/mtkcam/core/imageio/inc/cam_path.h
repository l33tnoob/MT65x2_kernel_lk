
/**
* @file cam_path.h
*
* cam_path Header File
*/

#ifndef __ISP_PATH_H__
#define __ISP_PATH_H__

#include "isp_function.h"

#define ISP_MAX_TPIPE_SIMPLE_CONF_SIZE  (128*sizeof(int))

#define DEBUG_STR_BEGIN "EEEEEEEEEEEEEEEEEEEEE"
#define DEBUG_STR_END   "XXXXXXXXXXXXXXXXXXXXX"

/**
*@brief  Path free buffer mode
*/
enum EPathFreeBufMode 
{
    ePathFreeBufMode_SINGLE = 0x0000,
    ePathFreeBufMode_ALL    = 0x0001
};


class IspFunction_B; //pre declaration

/**
*@brief  CamPath basic class
*/
class CamPath_B
{
public:
    ISP_TURNING_CTRL    ispTurningCtrl;
    ISP_TOP_CTRL        ispTopCtrl;
    ISP_BUF_CTRL        ispBufCtrl;
    int CQ;

    //common
    ISP_RAW_PIPE    ispRawPipe;
    ISP_RGB_PIPE    ispRgbPipe;
    ISP_YUV_PIPE    ispYuvPipe;
    CAM_CDP_PIPE    cdpPipe;
    CAM_TDRI_PIPE   tdriPipe;
    DMA_CQ          DMACQ;
    //for pass1
    DMA_IMGO        DMAImgo;
    DMA_IMG2O       DMAImg2o;
    //for pass2
    DMA_IMGI        DMAImgi;
    DMA_TDRI        DMATdri;
    DMA_VIPI        DMAVipi;
    DMA_VIP2I       DMAVip2i;
    DMA_IMGCI       DMAImgci;
    DMA_LSCI        DMALsci;
    DMA_LCEI        DMALcei;
//SL TEST_MDP removed. Should no used    DMA_DISPO   DMADispo; //SL
//SL TEST_MDP removed. Should no used        DMA_VIDO    DMAVido; //SL

/*

    DMA_FLKI    if_DMAFlki;
    DMA_DISPO   if_DMADispo;
    DMA_VIDO    if_DMAVido;
*/

public:

    /**
      *@brief  Constructor
      */
    CamPath_B();

    /**
      *@brief  Destructor      
      */
    virtual ~CamPath_B(){};
    
public:

    /**
      *@brief  Trigger cam path
      */
    int start( void* pParam );

    /**
      *@brief  Stop cam path
      */
    int stop( void* pParam );

    /**
      *@brief  Wait irq
      */
    inline int waitIrq( int type, unsigned int irq ){return this->_waitIrq( type,irq );}

    /**
      *@brief  Write register
      */
    inline int writeReg( unsigned long offset, unsigned long value ){return ispTopCtrl.writeReg(offset,value);}

    /**
      *@brief  Read register
      */
    inline unsigned long readReg( unsigned long offset ){return ispTopCtrl.readReg(offset);}

    /**
      *@brief  Read irq
      */
    inline int readIrq(ISP_DRV_READ_IRQ_STRUCT *pReadIrq){return ispTopCtrl.readIrq(pReadIrq);}

    /**
      *@brief  Check irq
      */
    inline int checkIrq(ISP_DRV_CHECK_IRQ_STRUCT CheckIrq){return ispTopCtrl.checkIrq(CheckIrq);}

    /**
      *@brief  Clear irq
      */
    inline int clearIrq(ISP_DRV_CLEAR_IRQ_STRUCT ClearIrq){return ispTopCtrl.clearIrq(ClearIrq);}

    /**
      *@brief  Dump register
      */
    int dumpRegister( void* pRaram );

    /**
      *@brief
      */
    int end( void* pParam );
    
protected:

    /**
      *@brief  Get isp function list
      *@return
      *-Pointer to isp function
      */
    virtual IspFunction_B**  isp_function_list() = 0;

    /**
      *@brief  Count how many isp function
      *@return
      *-Number of isp function
      */
    virtual int isp_function_count() = 0;
public:

    /**
      *@brief Return name string
      */
    virtual const char* name_Str(){ return  "IspPath";  }
protected:

    /**
      *@brief Implementation of config
      *@param[in] pParam : configure data
      */
    virtual int _config( void* pParam );

    /**
      *@brief Implementation of start
      *@param[in] pParam : trigger data
      */
    virtual int _start( void* pParam );

    /**
      *@brief Implementation of stop
      *@param[in] pParam : trigger data
      */
    virtual int _stop( void* pParam );

    /**
      *@brief Implementation of waitIrq
      *@param[in] type : wait type
      *@param[in] irq : irq type
      */
    virtual int _waitIrq( int type,unsigned int irq );

    /**
      *@brief Implementation of end
      *@param[in] pParam : data
      */
    virtual int _end( void* pParam );

    /**
      *@brief Implementation of setZoom
      *@param[in] pParam : zoom data
      */
    virtual int _setZoom( void* pParam );
public:

    /**
      *@brief Flush command queue descriptor
      *@param[in] cq : specific command queue
      */
    virtual int flushCqDescriptor( MUINT32 cq );

    /**
      *@brief Set current DMA buffer
       *@param[in] dmaChannel : dma channel
      */
    virtual int setDMACurrBuf( MUINT32 const dmaChannel );

    /**
      *@brief Set next DMA buffer
      *@param[in] dmaChannel : dma channel
      */
    virtual int setDMANextBuf( MUINT32 const dmaChannel );

    /**
      *@brief Enqueue buffer
      *@param[in] dmaChannel : dma channel
      *@param[in] bufInfo : buffer info
      */
    virtual int enqueueBuf( MUINT32 const dmaChannel, stISP_BUF_INFO bufInfo );

    /**
      *@brief Free physical buffer
      *@param[in] mode : free buffer mode
      *@param[in] bufInfo : buffer info
      */
    virtual int freePhyBuf( MUINT32 const mode, stISP_BUF_INFO bufInfo );

};

/**
*@brief  ISP pass1 path  parameter
*/
struct CamPathPass1Parameter
{
    //scenario/sub_mode
    int scenario;
    int subMode;
    int CQ;
    int isIspOn;
    int isEn1C24StatusFixed;
    int isEn1C02StatusFixed;
    int isEn1CfaStatusFixed;
    int isEn1HrzStatusFixed;
    int isEn1MfbStatusFixed;
    int isEn2CdrzStatusFixed;
    int isEn2G2cStatusFixed;
    int isEn2C42StatusFixed;
    int isImg2oStatusFixed;
    int isAaoStatusFixed;
    int isEsfkoStatusFixed;
    int isFlkiStatusFixed;
    int isLcsoStatusFixed;
    int isShareDmaCtlByTurn;
    int isEn1AaaGropStatusFixed;
    int bypass_ispRawPipe;
    int bypass_ispRgbPipe;
    int bypass_ispYuvPipe;
    int bypass_ispCdpPipe;

    //Misc
    int     b_continuous;

    //enable table
    struct stIspTopEnTbl      en_Top;
    struct stIspTopINT        ctl_int;
    struct stIspTopFmtSel     fmt_sel;
    struct stIspTopSel        ctl_sel;
    struct stIspTopMuxSel     ctl_mux_sel;
    struct stIspTopMuxSel2    ctl_mux_sel2;
    struct stIspTopSramMuxCfg ctl_sram_mux_cfg;
    //update function mask
    struct stIspTopFmtSel fixed_mask_cdp_fmt;
    //
    int                       pix_id;
    //source -> from TG
    IspSize         src_img_size;
    IspRect         src_img_roi;
    IspColorFormat  src_color_format;
    //
    IspSize         cdrz_in_size;
    /*===DMA===*/
    IspDMACfg imgo;     //dst00
    IspDMACfg img2o;    //dst01
    IspDMACfg lcso;
    IspDMACfg aao;
    IspDMACfg nr3o;
    IspDMACfg esfko;
    IspDMACfg afo;
    IspDMACfg eiso;
    IspDMACfg imgci;
    IspDMACfg nr3i;
    IspDMACfg flki;
    IspDMACfg lsci;
    IspDMACfg lcei;
    //
};

/**
*@brief  ISP pass1 path class
*/
class CamPathPass1:public CamPath_B
{
private:
    int             m_isp_function_count;
    IspFunction_B*   m_isp_function_list[ISP_FUNCTION_MAX_NUM];
    
public:

    /**
      *@brief  Constructor
      */
    CamPathPass1() :
        m_isp_function_count(0)
        {};

    /**
      *@brief  Destructor
      */
    virtual ~CamPathPass1(){};
private:

    /**
      *@brief Wait irq
      *@param[in] type : wait type
      *@param[in] irq : irq type
      */
    virtual int _waitIrq( int type,unsigned int irq );
protected:

    /**
      *@brief  Get isp function list
      *@return
      *-Pointer to isp function
      */
    virtual IspFunction_B**  isp_function_list()  { return m_isp_function_list; }

    /**
      *@brief  Count how many isp function
      *@return
      *-Number of isp function
      */
    virtual int isp_function_count() { return m_isp_function_count; }
public:

    /**
      *@brief Return name string
      */
    virtual const char* name_Str(){ return  "CamPathPass1";  }
public:

    /**
      *@brief Configure pass1 path
      *@param[in] p_parameter : configure data
      */
    int  config( struct CamPathPass1Parameter* p_parameter );

    /**
      *@brief Configure CDRZ
      *@param[in] out_size : IspSize
      */
    int     setCdrz( IspSize out_size );

    /**
      *@brief Configure IMGO
      *@param[in] out_dma : IspDMACfg
      */
    int     setDMAImgo( IspDMACfg const out_dma );

    /**
      *@brief Dequeue buffer
      *@param[in] dmaChannel : dma channel
      *@param[in] bufInfo : buffer info
      */
    int     dequeueBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufInfo );

    /**
      *@brief Set command queue trigger mode
      *@param[in] cq : specific command queue
      *@param[in] mode : trigger mode
      *@param[in] trig_src : trigger source
      */
    int     setCQTriggerMode(MINT32 cq, MINT32 mode, MINT32 trig_src);
private:
};

/**
*@brief  ISP pass2 path  parameter
*/
struct CamPathPass2Parameter
{
    //scenario/sub_mode    
    //int subMode;
    unsigned int scenario;
    int CQ;
    int tpipe;
    int tcm_en;
    int isIspOn;
    int isConcurrency;
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
    int isEn1AaaGropStatusFixed;
    int isApplyTurn;
    int isShareDmaCtlByTurn;
    int bypass_ispRawPipe;
    int bypass_ispRgbPipe;
    int bypass_ispYuvPipe;
    int bypass_ispCdpPipe;

    //enable table
    struct stIspTopEnTbl    en_Top;
    struct stIspTopINT      ctl_int;
    struct stIspTopFmtSel   fmt_sel;
    struct stIspTopSel      ctl_sel;
    struct stIspTopMuxSel     ctl_mux_sel;
    struct stIspTopMuxSel2    ctl_mux_sel2;
    struct stIspTopSramMuxCfg ctl_sram_mux_cfg;
    //update function mask
    struct stIspTopFmtSel fixed_mask_cdp_fmt;
    //
    int                     pix_id;
    //source ->  mem. in
    IspSize         src_img_size;
    IspRect         src_img_roi;
    IspColorFormat  src_color_format;
    //
    //IspSize         cdrz_out;
    /*===DMA===*/
    IspDMACfg tdri;
    IspDMACfg cqi;
    IspDMACfg imgi;
    IspDMACfg vipi;
    IspDMACfg vip2i;
    IspDMACfg imgci;
    IspDMACfg lcei;
    IspDMACfg lsci;
    CdpRotDMACfg dispo;
    CdpRotDMACfg vido;
    //

    IspTdriUpdateCfg updateTdri;
    IspRingTdriCfg ringTdriCfg;
    IspCapTdriCfg capTdriCfg;
    IspBnrCfg bnr;
    IspLscCfg lsc;
    IspLceCfg lce;
    IspNbcCfg nbc;
    IspSeeeCfg seee;
    IspDMACfg imgo_dma;
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
/*    IspDMACfg imgo;
    IspDMACfg img2o;
    IspDMACfg lcso;
    IspDMACfg aao;
    IspDMACfg nr3o;
    IspDMACfg esfko;
    IspDMACfg nr3i;
    IspDMACfg flki;
    IspDMACfg lsci;
    IspDMACfg lcei;
*/    //
};

/**
*@brief  ISP pass1 path class
*/
class CamPathPass2:public CamPath_B
{
private:
    int             m_isp_function_count;
    IspFunction_B*   m_isp_function_list[ISP_FUNCTION_MAX_NUM];

public:

    /**
      *@brief  Constructor
      */
    CamPathPass2() :
        m_isp_function_count(0)
        {};

    /**
      *@brief  Destructor
      */
    virtual ~CamPathPass2(){};
private:

    /**
      *@brief Wait irq
      *@param[in] type : wait type
      *@param[in] irq : irq type
      */
    virtual int _waitIrq( int type,unsigned int irq );
protected:

    /**
      *@brief  Get isp function list
      *@return
      *-Pointer to isp function
      */    
    virtual IspFunction_B**  isp_function_list()  {   return m_isp_function_list; }

    /**
      *@brief  Count how many isp function
      *@return
      *-Number of isp function
      */
    virtual int isp_function_count() {   return m_isp_function_count; }
public:

    /**
      *@brief Return name string
      */
    virtual const char* name_Str(){     return  "CamPathPass2";  }
public:

    /**
      *@brief Configure pass1 path
      *@param[in] p_parameter : configure data
      */
    int config( struct CamPathPass2Parameter* p_parameter );

    /**
      *@brief Set zoom
      *@param[in] zoomRatio : zoom ratio
      */
    int setZoom( MUINT32 zoomRatio );

    /**
      *@brief Dequeue buffer
      *@param[in] dmaChannel : dma channel
      *@param[in] bufInfo : buffer info
      */
    int dequeueBuf( MUINT32 dmaChannel ,stISP_FILLED_BUF_LIST& bufInfo );

    /**
      *@brief Configure ting tpipe
      *@param[in] p_parameter : configure data
      */
    int configRingTpipe( struct CamPathPass2Parameter* p_parameter);
private:

    /**
      *@brief Configure tpipe
      *@param[in] p_parameter : configure data
      */
    int configTpipeData( struct CamPathPass2Parameter* p_parameter );

    /**
      *@brief Get tpipe perform
      *@param[in] p_parameter : configure data
      */
    int getTpipePerform( struct CamPathPass2Parameter* p_parameter );

    /**
      *@brief Get CDP mux setting
      *@param[in] pass2Parameter : configure data
      *@param[in] pDispVidSel : mus setting
      */
    int getCdpMuxSetting(struct CamPathPass2Parameter pass2Parameter, MINT32 *pDispVidSel);
};

#endif







