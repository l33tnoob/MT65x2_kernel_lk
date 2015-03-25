#define LOG_TAG "iio/path"
//
//#define _LOG_TAG_LOCAL_DEFINED_
//#include <my_log.h>
//#undef  _LOG_TAG_LOCAL_DEFINED_
//
#include "cam_path.h"
//

#if 0

#include <cutils/properties.h>  // For property_get().


#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{Path}"
#include "imageio_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(path);
//EXTERN_DBG_LOG_VARIABLE(path);

// Clear previous define, use our own define.
#undef ISP_PATH_VRB
#undef ISP_PATH_DBG
#undef ISP_PATH_INF
#undef ISP_PATH_WRN
#undef ISP_PATH_ERR
#undef ISP_PATH_AST
#define ISP_PATH_VRB(fmt, arg...)        do { if (path_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define ISP_PATH_DBG(fmt, arg...)        do { if (path_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define ISP_PATH_INF(fmt, arg...)        do { if (path_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define ISP_PATH_WRN(fmt, arg...)        do { if (path_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define ISP_PATH_ERR(fmt, arg...)        do { if (path_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define ISP_PATH_AST(cond, fmt, arg...)  do { if (path_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)
#else
#include <cutils/properties.h>  // For property_get().

#include "imageio_log.h"                        // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        ""



DECLARE_DBG_LOG_VARIABLE(path);
//EXTERN_DBG_LOG_VARIABLE(path);

#endif


/*-----------------------------------------------------------------------------
    MACRO Definition
  -----------------------------------------------------------------------------*/

#define PRINT_ELEMENT_INVOLVED( _title_str_, _isplist_, _ispcount_, _mask_, _b_from_head_ )\
{\
    int  _i;    char _temp_str[512];\
    _temp_str[0] = '\0';\
    strcat( _temp_str, _title_str_ );\
    if( _b_from_head_ ) {\
        for( _i = 0; _i < _ispcount_; _i++ )    {\
            if( ( _mask_ !=0  )    &&   ( ( _mask_ & _isplist_[_i]->id() ) == 0 )    ){\
                    continue;       }\
            strcat( _temp_str,"->");\
            strcat( _temp_str,_isplist_[_i]->name_Str());\
        }\
    } else {\
        for( _i = (_ispcount_-1) ; _i >= 0 ; _i-- )    {\
            if( ( _mask_ !=0  )    &&   ( ( _mask_ & _isplist_[_i]->id() ) == 0 )    ){\
                    continue;       }\
            strcat( _temp_str,"->");\
            strcat( _temp_str,_isplist_[_i]->name_Str());\
        }\
    }\
    ISP_PATH_DBG("%s",_temp_str);\
}


/*-----------------------------------------------------------------------------
    Functions
  -----------------------------------------------------------------------------*/

CamPath_B::
CamPath_B():CQ(CAM_ISP_CQ_NONE)
{
    DBG_LOG_CONFIG(imageio, path);

}


int CamPath_B::start( void* pParam )
{
    ISP_PATH_DBG("{%s}::Start E" DEBUG_STR_BEGIN "", this->name_Str() );

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    int ret = 0;

    if( this->_start( pParam ) < 0 ) {
        ISP_PATH_ERR("[ERROR] start(%s)\n",this->name_Str());
        ret = -1;
        goto EXIT;
    }
    ISP_PATH_DBG(DEBUG_STR_END"%s::Start""", this->name_Str() );

EXIT:

    ISP_PATH_DBG(":X ");

    return ret;
}

int CamPath_B::stop( void* pParam )
{
    ISP_PATH_DBG("{%s}::Stop E"DEBUG_STR_BEGIN"", this->name_Str() );

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    int ret = 0;

    if( this->_stop( pParam ) < 0 ){
        ISP_PATH_ERR("[ERROR] stop(%s)\n",this->name_Str());
        //return -1;
        ret = -1;
        goto EXIT;
    }
    ISP_PATH_DBG(DEBUG_STR_END"%s::Stop""", this->name_Str() );

    EXIT:

    ISP_PATH_DBG(":X ");

    return ret;
}
//
int CamPath_B::dumpRegister( void* pParam )
{
    ISP_PATH_DBG("%s::dumpRegister E"DEBUG_STR_BEGIN"", this->name_Str() );


    int     i;
    IspFunction_B**  isplist;
    int             ispcount;

    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    for( i = 0; i < ispcount; i++ )
    {
        if( i == 0 )//Use 1st isp element to dump mmsys1 system register
            isplist[i]->dumpRegister(1);

        isplist[i]->dumpRegister(0);
    }

    ISP_PATH_DBG(DEBUG_STR_END"%s::dumpRegister""", this->name_Str() );

    EXIT:

    ISP_PATH_DBG(":X ");

    return 0;

}

int CamPath_B::end( void* pParam )
{
    ISP_PATH_DBG("{%s}: E "DEBUG_STR_BEGIN" ", this->name_Str() );

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);


    int ret = 0;

    if( this->_end( pParam ) < 0 ) {
        ISP_PATH_ERR("[ERROR] end(%s)\n",this->name_Str());
        ret = -1;
        goto EXIT;
    }
    ISP_PATH_DBG(DEBUG_STR_END"%s::End""", this->name_Str() );

    EXIT:

    ISP_PATH_DBG(":X ");

    return ret;
}

int CamPath_B::_config( void* pParam )
{
    ISP_PATH_DBG("[_config]: E cq(%d)\n",this->CQ);


    int             ret_val = ISP_ERROR_CODE_OK;
    int             i;
    IspFunction_B**  isplist;
    int             ispcount;
    struct CamPathPass2Parameter* pPass2 = (struct CamPathPass2Parameter*)pParam;


    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    //set all function register base
    //ret_val = IspStart( isplist , ispcount );
    //if( ret_val < 0 ) {
    //    ISP_PATH_ERR("[ERROR] _config\n");
        //return ret_val;
    //    goto EXIT;
    //}
    //
    if ( CAM_ISP_CQ_NONE != this->CQ ) {
        //this->ispTopCtrl.m_pIspDrvShell->getPhyIspDrv().getCQInstance(ISP_DRV_CQ_ENUM cq)
        //
        this->ispTopCtrl.ispDrvSwitch2Virtual(this->CQ);
    }//
    //config (From isp to cdpl)
    PRINT_ELEMENT_INVOLVED("[_config]:", isplist, ispcount, 0 , 1 );
    for( i = 0; i < ispcount; i++ )
    {
        if (isplist[i]->is_bypass()) {
            ISP_PATH_DBG("<%s> bypass:",isplist[i]->name_Str());
            continue;
        }

        ISP_PATH_DBG("<%s> config:",isplist[i]->name_Str());
        if( ( ret_val = isplist[i]->config() ) < 0 )
        {
            ISP_PATH_ERR("[ERROR] _config(%s)\n",isplist[i]->name_Str());
            //return ret_val;
            goto EXIT;
        }

        if ( CAM_ISP_CQ_NONE != this->CQ ) {
            if( ( ret_val = isplist[i]->write2CQ(this->CQ) ) < 0 )
            {
                ISP_PATH_ERR("[ERROR] _config CQ(%d)\n",this->CQ);
                //return ret_val;
                goto EXIT;
            }
        }
    }


EXIT:

    ISP_PATH_DBG(":X ");


    return ret_val;
}
//
int CamPath_B::_setZoom( void* pParam )
{
    ISP_PATH_DBG("_setZoom E\n");


    int             ret_val = ISP_ERROR_CODE_OK;
    int             i;
    IspFunction_B**  isplist;
    int             ispcount;


    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    //setZoom (From isp to cdp)
    PRINT_ELEMENT_INVOLVED("[_setZoom]:", isplist, ispcount, 0 , 1 );
    for( i = 0; i < ispcount; i++ )
    {
        if (isplist[i]->is_bypass()) {
            ISP_PATH_DBG("<%s> bypass:",isplist[i]->name_Str());
            continue;
        }

        ISP_PATH_DBG("<%s> setZoom:",isplist[i]->name_Str());
        if( ( ret_val = isplist[i]->setZoom() ) < 0 ) {
            ISP_PATH_ERR("[ERROR] _setZoom(%s)\n",isplist[i]->name_Str());
            //return ret_val;
            goto EXIT;
        }

    }

    EXIT:

    ISP_PATH_DBG(":X ");

    return ret_val;
}
//
int CamPath_B::_start( void* pParam )
{
    ISP_PATH_DBG("_start E\n");

    int             ret_val = ISP_ERROR_CODE_OK;
    int             i;
    IspFunction_B**  isplist;
    int             ispcount;
    int             isp_start = ISP_PASS1;

    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    //set all function register base
    //ret_val = IspStart( isplist , ispcount );

    if( ret_val < 0 ) {
        ISP_PATH_ERR("[ERROR] _start\n");
        goto EXIT;
        //return ret_val;
    }

    //config (From isp to cdpl)
    /*PRINT_ELEMENT_INVOLVED("[config]:", isplist, ispcount, 0 , 1 );
    for( i = 0; i < ispcount; i++ )
    {
        ISP_PATH_DBG("<%s> config:",isplist[i]->name_Str());
        if( ( ret_val = isplist[i]->config() ) < 0 )
        {
            return ret_val;
        }
    }*/

    //enable (From cdp to isp)
    PRINT_ELEMENT_INVOLVED("[enable]:", isplist, ispcount, 0, 0 );
    for( i = (ispcount-1) ; i >= 0 ; i-- )
    {
        //ISP_PATH_DBG("<%s> enable.",isplist[i]->name_Str() );
        if( ( ret_val = isplist[i]->enable((void*)pParam ) ) < 0 ) {
            ISP_PATH_ERR("[ERROR] _start enable\n");
            goto EXIT;
            //return ret_val;
        }
    }

    EXIT:

    ISP_PATH_DBG(":X ");

    return ret_val;
}

int CamPath_B::_stop( void* pParam )
{
    ISP_PATH_DBG("_stop E\n");

int ret = 0;
    int     ret_val = ISP_ERROR_CODE_OK;
    int     ret_val_temp = ISP_ERROR_CODE_OK;
    int     i;
    IspFunction_B**  isplist;
    int             ispcount;

    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    //3.disable (From head to tail)
    PRINT_ELEMENT_INVOLVED("[disable]:", isplist, ispcount, 0, 1 );
    for( i = 0; i < ispcount; i++ )
    {
        //ISP_PATH_DBG("<%s> disable.",isplist[i]->name_Str() );
        if( ( ret_val_temp = isplist[i]->disable() ) < 0 )
            ret_val = ret_val_temp;
    }

    EXIT:

    ISP_PATH_DBG(":X ");

    return ret;
}//

int CamPath_B::_waitIrq( int type,unsigned int irq  )
{
#if 1
    return 0;
#else
    int     i;
    int     ret = 0;
    const int           kWAIT_COUNT_DOWN = 0xFFFFFF;
    const unsigned long kTIMEOUT_MS = 1000;
    unsigned long   desc_read_pointer;
    IspFunction_B**  isplist;
    int             ispcount;

    isplist  =  this->isp_function_list();
    ispcount =  this->isp_function_count();

    //Check busy (From head to tail)
    PRINT_ELEMENT_INVOLVED("[checkBusy]:", isplist, ispcount, irq,1 );
    for( i = 0; i < ispcount; i++ )
    {
        int wait_count_down = kWAIT_COUNT_DOWN;

        //ISP_PATH_DBG("Waiting <%s> ",isplist[i]->name_Str());

        if( ( irq !=0  )    &&
            ( ( irq & isplist[i]->id() ) == 0 )    )
        {
            continue;
        }

#if defined(ISP_FLAG_1_SUPPORT_INT)

        if( isplist[i]->WaitIntDone( kTIMEOUT_MS ) < 0 )
        {
            ISP_PATH_ERR("Waiting <%s> Time Out",isplist[i]->name_Str());
            //ISP_PATH_DBG("Start to dump register...");
            //ISP_PATH_DBG("Dump registerFinish.");
            ret = -1;
        }
#else
        while( isplist[i]->checkBusy(&desc_read_pointer) )
        {
            wait_count_down--;

            if( wait_count_down == 0 )
            {
                ret = -1;
                wait_count_down = kWAIT_COUNT_DOWN;
                ISP_PATH_ERR("Waiting <%s> Time Out",isplist[i]->name_Str());
                //ISP_PATH_DBG("Start to dump register...");
                //dumpRegister( NULL );
                //ISP_PATH_DBG("Dump registerFinish.");
                break; //Stop busy waiting
            }
        }
#endif
    }

    if( ret == -1 )
    {
        ISP_PATH_DBG("Start to dump register...");
        dumpRegister( NULL );
        ISP_PATH_DBG("Dump registerFinish.");
    }

    return ret;
#endif

}

int CamPath_B::_end( void* pParam )
{
int ret = 0;
    return ret;
}

//
int CamPath_B::enqueueBuf( MUINT32 const dmaChannel, stISP_BUF_INFO bufInfo )
{
    ISP_PATH_DBG("enqueueBuf E\n");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    int ret = 0;
    //
    if ( 0 != this->ispBufCtrl.enqueueHwBuf(dmaChannel, bufInfo) ) {
        ISP_PATH_ERR("ERROR:enqueueHwBuf");
        goto EXIT;
        //return -1;
    }

    EXIT:

    ISP_PATH_DBG(":X ");

    //
    return ret;
}
//
int CamPath_B::freePhyBuf( MUINT32 const mode, stISP_BUF_INFO bufInfo )
{
    ISP_PATH_DBG("freePhyBuf E\n");

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    int ret = 0;
    if ( ePathFreeBufMode_SINGLE == mode ) {
        this->ispBufCtrl.freeSinglePhyBuf(bufInfo);
    }
    else if ( ePathFreeBufMode_ALL == mode ) {
        this->ispBufCtrl.freeAllPhyBuf();
    }

    EXIT:

    ISP_PATH_DBG(":X ");

    return ret;
}

//
int CamPath_B::setDMACurrBuf( MUINT32 const dmaChannel)
{
    ISP_PATH_DBG("[0x%08x] E",dmaChannel);

    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);

    int ret = 0;

    //isp_driver is in physical domain after _config()

    //->for pass1
    if ( ISP_DMA_IMGO & dmaChannel ) {
        this->DMAImgo.dma_cfg.memBuf.base_pAddr = this->ispBufCtrl.getCurrHwBuf((MUINT32) ISP_DMA_IMGO);
        ret = this->DMAImgo.config();
        if ( 0 != ret ) {
            ISP_PATH_ERR("ERROR config imgo ");
            goto EXIT;
        }
    }
    //
    if ( ISP_DMA_IMG2O & dmaChannel ) {
        this->DMAImg2o.dma_cfg.memBuf.base_pAddr = this->ispBufCtrl.getCurrHwBuf((MUINT32) ISP_DMA_IMG2O);
        ret = this->DMAImg2o.config();
        if ( 0 != ret ) {
            ISP_PATH_ERR("ERROR config img2o ");
            goto EXIT;
        }
    }

    //
    if ( CAM_ISP_CQ_NONE != this->CQ ) {
        this->ispTopCtrl.ispDrvSwitch2Virtual(this->CQ);
    }
    //
    //->for pass2
    //
    if ( ISP_DMA_IMGI & dmaChannel ) {
        this->DMAImgi.dma_cfg.memBuf.base_pAddr = this->ispBufCtrl.getCurrHwBuf((MUINT32) ISP_DMA_IMGI);
        ret = this->DMAImgi.config();
        //m_isImgPlaneByImgi == TRUE
        if (this->DMAImgi.dma_cfg.memBuf_c_ofst) {
            this->DMAVipi.dma_cfg.memBuf.base_pAddr = this->DMAImgi.dma_cfg.memBuf.base_pAddr + this->DMAImgi.dma_cfg.memBuf_c_ofst;
            ret = this->DMAVipi.config();
        }
        if (this->DMAImgi.dma_cfg.memBuf_v_ofst) {
            this->DMAVip2i.dma_cfg.memBuf.base_pAddr = this->DMAImgi.dma_cfg.memBuf.base_pAddr + this->DMAImgi.dma_cfg.memBuf_v_ofst;
            ret = this->DMAVip2i.config();
        }
    }
    //
    if ( ISP_DMA_VIPI & dmaChannel ) {
        this->DMAVipi.dma_cfg.memBuf.base_pAddr = this->ispBufCtrl.getCurrHwBuf((MUINT32) ISP_DMA_VIPI);
        ret = this->DMAVipi.config();
    }
    //
    if ( ISP_DMA_VIP2I & dmaChannel ) {
        this->DMAVip2i.dma_cfg.memBuf.base_pAddr = this->ispBufCtrl.getCurrHwBuf((MUINT32) ISP_DMA_VIP2I);
        ret = this->DMAVip2i.config();
    }
    //for MFB
    //
    if ( ISP_DMA_IMGCI & dmaChannel ) {
        this->DMAImgci.dma_cfg.memBuf.base_pAddr = this->ispBufCtrl.getCurrHwBuf((MUINT32) ISP_DMA_IMGCI);
        ret = this->DMAImgci.config();
    }
    //
    if ( ISP_DMA_LSCI & dmaChannel ) {
        this->DMALsci.dma_cfg.memBuf.base_pAddr = this->ispBufCtrl.getCurrHwBuf((MUINT32) ISP_DMA_LSCI);
        ret = this->DMALsci.config();
    }
    //
    if ( ISP_DMA_LCEI & dmaChannel ) {
        this->DMALcei.dma_cfg.memBuf.base_pAddr = this->ispBufCtrl.getCurrHwBuf((MUINT32) ISP_DMA_LCEI);
        ret = this->DMALcei.config();
    }
    //
    if ( ISP_DMA_DISPO & dmaChannel ) {
        cdpPipe.dispo_out.memBuf.base_pAddr = this->ispBufCtrl.getCurrHwBuf((MUINT32) ISP_DMA_DISPO);
//SL TEST_MDP removed. Should no used    ISP_PATH_DBG("SL : this->ispBufCtrl.getCurrHwBuf((MUINT32) ISP_DMA_DISPO)\n ");        
//SL TEST_MDP removed. Should no used    ISP_PATH_DBG("SL : should GET SW ADDR now ; 0x%8x \n ",cdpPipe.dispo_out.memBuf.base_pAddr);       
//SL TEST_MDP removed. Should no used        ret = this->DMADispo.config(); //SL
        //
        if ( 0 == cdpPipe.dispo_out.uv_plane_swap ) {
            cdpPipe.dispo_out.memBuf_c.base_pAddr = cdpPipe.dispo_out.memBuf.base_pAddr + \
                                                    cdpPipe.dispo_out.memBuf_c.ofst_addr;
            cdpPipe.dispo_out.memBuf_v.base_pAddr = cdpPipe.dispo_out.memBuf.base_pAddr + \
                                                    cdpPipe.dispo_out.memBuf_v.ofst_addr;
        }
        else {
            cdpPipe.dispo_out.memBuf_c.base_pAddr = cdpPipe.dispo_out.memBuf.base_pAddr + \
                                                    cdpPipe.dispo_out.memBuf_v.ofst_addr;
            cdpPipe.dispo_out.memBuf_v.base_pAddr = cdpPipe.dispo_out.memBuf.base_pAddr + \
                                                    cdpPipe.dispo_out.memBuf_c.ofst_addr;
        }
        //ret = this->cdpPipe.config(); //this func will induce sysram realloc.
        this->ispTopCtrl.m_pIspDrvShell->m_pCdpDrv->DISPO_SetOutputAddr(
                        this->cdpPipe.dispo_out.memBuf.base_pAddr,
                        this->cdpPipe.dispo_out.memBuf.ofst_addr,
                        this->cdpPipe.dispo_out.size.stride * this->cdpPipe.dispo_out.pixel_byte, //byte input
                        this->cdpPipe.dispo_out.memBuf_c.base_pAddr,
                        this->cdpPipe.dispo_out.memBuf_c.ofst_addr,
                        this->cdpPipe.dispo_out.size_c.stride * this->cdpPipe.dispo_out.pixel_byte, //byte input
                        this->cdpPipe.dispo_out.memBuf_v.base_pAddr,
                        this->cdpPipe.dispo_out.memBuf_v.ofst_addr,
                        this->cdpPipe.dispo_out.size_v.stride * this->cdpPipe.dispo_out.pixel_byte); //byte input

        ISP_PATH_DBG("[DISPO]base(0x%x)\n",this->cdpPipe.dispo_out.memBuf.base_pAddr);

    }
    //
    if ( ISP_DMA_VIDO & dmaChannel ) {
        cdpPipe.vido_out.memBuf.base_pAddr = this->ispBufCtrl.getCurrHwBuf((MUINT32) ISP_DMA_VIDO);
        //
        if ( 0 == cdpPipe.vido_out.uv_plane_swap ) {
            cdpPipe.vido_out.memBuf_c.base_pAddr = cdpPipe.vido_out.memBuf.base_pAddr + \
                                                    cdpPipe.vido_out.memBuf_c.ofst_addr;
            cdpPipe.vido_out.memBuf_v.base_pAddr = cdpPipe.vido_out.memBuf.base_pAddr + \
                                                    cdpPipe.vido_out.memBuf_v.ofst_addr;
        }
        else {
            cdpPipe.vido_out.memBuf_c.base_pAddr = cdpPipe.vido_out.memBuf.base_pAddr + \
                                                    cdpPipe.vido_out.memBuf_v.ofst_addr;
            cdpPipe.vido_out.memBuf_v.base_pAddr = cdpPipe.vido_out.memBuf.base_pAddr + \
                                                    cdpPipe.vido_out.memBuf_c.ofst_addr;
        }
        //ret = this->cdpPipe.config(); //this func will induce sysram realloc.
        this->ispTopCtrl.m_pIspDrvShell->m_pCdpDrv->VIDO_SetOutputAddr(
                        this->cdpPipe.vido_out.memBuf.base_pAddr,
                        this->cdpPipe.vido_out.memBuf.ofst_addr,
                        this->cdpPipe.vido_out.size.stride * this->cdpPipe.vido_out.pixel_byte, //byte input
                        this->cdpPipe.vido_out.memBuf_c.base_pAddr,
                        this->cdpPipe.vido_out.memBuf_c.ofst_addr,
                        this->cdpPipe.vido_out.size_c.stride * this->cdpPipe.vido_out.pixel_byte, //byte input
                        this->cdpPipe.vido_out.memBuf_v.base_pAddr,
                        this->cdpPipe.vido_out.memBuf_v.ofst_addr,
                        this->cdpPipe.vido_out.size_v.stride * this->cdpPipe.vido_out.pixel_byte); //byte input

        ISP_PATH_INF("[VIDO]base(0x%x)-0x%x-0x%x\n",this->cdpPipe.vido_out.memBuf.base_pAddr,
                this->cdpPipe.vido_out.memBuf_c.base_pAddr,this->cdpPipe.vido_out.memBuf_v.base_pAddr);

    }
    //
    if ( ISP_DMA_FDO & dmaChannel ) {
        //FD can use either port, choose one.
        cdpPipe.dispo_out.memBuf.base_pAddr = this->ispBufCtrl.getCurrHwBuf((MUINT32) ISP_DMA_FDO);
        //ret = this->cdpPipe.config();//this func will induce sysram realloc.
    }
    //
    if ( 0 != ret ) {
        ISP_PATH_ERR("ERROR config\n");
        goto EXIT;
    }
    //
    //if ( CAM_ISP_CQ_NONE != this->CQ ) {
    //    this->ispTopCtrl.ispDrvSwitch2Phy();
    //}
    EXIT:

    ISP_PATH_DBG(":X ");


    return ret;
}
//

int CamPath_B::flushCqDescriptor( MUINT32 cq )
{
    ISP_PATH_INF("cq(%d)",cq);

    int ret = 0;
    if(cq < ISP_DRV_CQ_NUM) {
        for(int i=0; i<CAM_DUMMY_; i++)
            this->ispTopCtrl.m_pIspDrvShell->cqDelModule((ISP_DRV_CQ_ENUM)cq, (CAM_MODULE_ENUM)i);
    } else {
        ISP_PATH_WRN("[warning]not support this cq(%d)",cq);
    }
    return ret;
}


int CamPath_B::setDMANextBuf( MUINT32 const dmaChannel )
{
    ISP_PATH_DBG("[0x%x] E",dmaChannel);


    Mutex::Autolock lock(this->ispTopCtrl.m_pIspDrvShell->gLock);


    int ret = 0;


    if ( CAM_ISP_CQ_NONE != this->CQ ) {
        this->ispTopCtrl.ispDrvSwitch2Virtual(this->CQ);
    }
    //isp_driver is in physical domain after _config()
    //no need to switch
    //
    //->for pass1
    if ( ISP_DMA_IMGO & dmaChannel ) {
        this->DMAImgo.dma_cfg.memBuf.base_pAddr = this->ispBufCtrl.getNextHwBuf((MUINT32) ISP_DMA_IMGO);
        ret = this->DMAImgo.config();
        if ( 0 != ret ) {
            ISP_PATH_ERR("ERROR config imgo ");
            goto EXIT;
        }
    }
    //
    if ( ISP_DMA_IMG2O & dmaChannel ) {
        this->DMAImg2o.dma_cfg.memBuf.base_pAddr = this->ispBufCtrl.getNextHwBuf((MUINT32) ISP_DMA_IMG2O);
        ret = this->DMAImg2o.config();
        if ( 0 != ret ) {
            ISP_PATH_ERR("ERROR config img2o ");
            goto EXIT;
        }
    }
    //->for pass2
    //
    if ( ISP_DMA_IMGI & dmaChannel ) {
        this->DMAImgi.dma_cfg.memBuf.base_pAddr = this->ispBufCtrl.getNextHwBuf((MUINT32) ISP_DMA_IMGI);
        ret = this->DMAImgi.config();
    }
    //
    if ( ISP_DMA_VIPI & dmaChannel ) {
        this->DMAVipi.dma_cfg.memBuf.base_pAddr = this->ispBufCtrl.getNextHwBuf((MUINT32) ISP_DMA_VIPI);
        ret = this->DMAVipi.config();
    }
    //
    if ( ISP_DMA_VIP2I & dmaChannel ) {
        this->DMAVip2i.dma_cfg.memBuf.base_pAddr = this->ispBufCtrl.getNextHwBuf((MUINT32) ISP_DMA_VIP2I);
        ret = this->DMAVip2i.config();
    }
    //
    if ( ISP_DMA_DISPO & dmaChannel ) {
        cdpPipe.dispo_out.memBuf.base_pAddr = this->ispBufCtrl.getNextHwBuf((MUINT32) ISP_DMA_DISPO);
        ret = this->cdpPipe.config();
    }
    //
    if ( ISP_DMA_VIDO & dmaChannel ) {
        cdpPipe.vido_out.memBuf.base_pAddr = this->ispBufCtrl.getNextHwBuf((MUINT32) ISP_DMA_VIDO);
        ret = this->cdpPipe.config();
    }
    //
    if ( ISP_DMA_FDO & dmaChannel ) {
        //FD can use either port, choose one.
        cdpPipe.dispo_out.memBuf.base_pAddr = this->ispBufCtrl.getNextHwBuf((MUINT32) ISP_DMA_FDO);
        ret = this->cdpPipe.config();
    }
    //
    if ( 0 != ret ) {
        ISP_PATH_ERR("ERROR config ");
        goto EXIT;
    }
    //
    //if ( CAM_ISP_CQ_NONE != this->CQ ) {
    //    this->ispTopCtrl.ispDrvSwitch2Phy();
    //}
    EXIT:

    ISP_PATH_DBG(":X ");

    return ret;
}
//
//





