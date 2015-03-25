#include <platform/mt_reg_base.h>
#include <platform/mt_gpt.h>
#include <platform/disp_drv_platform.h>
#include <platform/ddp_reg.h>
#include <platform/ddp_path.h>

#include "disp_drv_log.h"
         
unsigned int gMutexID = 0;
         
         
//50us -> 1G needs 50000times
int disp_wait_timeout(BOOL flag, unsigned int timeout)
{
   unsigned int cnt=0;
   
   while(cnt<timeout)
   {
      if(flag)
      {
         return 0;
      }
      cnt++;
   }

   return -1;
}

int disp_delay_timeout(BOOL flag, unsigned int delay_ms)
{
   unsigned int cnt=0;
   
   while(cnt<delay_ms)
   {
      if(flag)
      {
         return 0;
      }
      cnt++;
      mdelay(1);
   }

   return -1;
}

int disp_path_get_mutex()
{
#if 0
    int cnt = 0;
   
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX_EN(gMutexID), 1);        
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX(gMutexID), 1);

    while((DISP_REG_GET(DISP_REG_CONFIG_MUTEX(gMutexID)) & DISP_INT_MUTEX_BIT_MASK) != DISP_INT_MUTEX_BIT_MASK)
    {
        cnt++;
        if(cnt>1000*1000*500)
        {
            printf("error: disp_mutex_lock timeout! \n");
            return -1;
        }
    }

#else
   DISP_REG_SET(DISP_REG_CONFIG_MUTEX_EN(gMutexID), 1);        
   DISP_REG_SET(DISP_REG_CONFIG_MUTEX(gMutexID), 1);

   if(disp_delay_timeout(((DISP_REG_GET(DISP_REG_CONFIG_MUTEX(gMutexID))& DISP_INT_MUTEX_BIT_MASK) == DISP_INT_MUTEX_BIT_MASK), 5))
   {
      printf("[DDP] error! disp_path_get_mutex(), get mutex timeout! \n");
      disp_dump_reg(DISP_MODULE_MUTEX);
      disp_dump_reg(DISP_MODULE_CONFIG);
      return - 1;
   } 
#endif

   return 0;
}

int disp_path_release_mutex()
{
#if 0
    unsigned int reg = 0;
    unsigned int cnt = 0;
    
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX(gMutexID), 0);

    while((DISP_REG_GET(DISP_REG_CONFIG_MUTEX(gMutexID)) & DISP_INT_MUTEX_BIT_MASK) != 0)
    {
        cnt++;
        if(cnt>1000*1000*500)
        {
            printf("error: disp_mutex_unlock timeout! \n");
            return - 1;
        }
    }

    cnt=0;
    while(((DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA) & (1<<gMutexID)) != (1<<gMutexID)))
    {
        cnt++;
        if(cnt>1000*100*500)
        {
            printf("error: disp_path_release_mutex()-2 timeout! \n");
            break;
        }
    }

#else
    unsigned int reg = 0;
    
    DISP_REG_SET(DISP_REG_CONFIG_MUTEX(gMutexID), 0);

   if(disp_delay_timeout(((DISP_REG_GET(DISP_REG_CONFIG_MUTEX(gMutexID)) & DISP_INT_MUTEX_BIT_MASK) == 0), 5))
   {
      if((DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA) & (1<<(gMutexID+8))) == (unsigned int)(1<<(gMutexID+8)))
      {
          printf("[DDP] error! disp_path_release_mutex(), release mutex timeout! \n");
          disp_dump_reg(DISP_MODULE_CONFIG);
          //print error engine
          reg = DISP_REG_GET(DISP_REG_CONFIG_REG_COMMIT);
          if(reg!=0)
          {
                if(reg&(1<<3))  { printf(" OVL update reg timeout! \n"); disp_dump_reg(DISP_MODULE_OVL); }
                if(reg&(1<<6))  { printf(" WDMA0 update reg timeout! \n"); disp_dump_reg(DISP_MODULE_WDMA0); }
                if(reg&(1<<7))  { printf(" COLOR update reg timeout! \n"); disp_dump_reg(DISP_MODULE_COLOR); }
                if(reg&(1<<9))  { printf(" BLS update reg timeout! \n"); disp_dump_reg(DISP_MODULE_BLS); }
                if(reg&(1<<10))  { printf(" RDMA1 update reg timeout! \n"); disp_dump_reg(DISP_MODULE_RDMA0); }
          }  

          disp_dump_reg(DISP_MODULE_MUTEX);
          disp_dump_reg(DISP_MODULE_CONFIG);
          return - 1;
      }
   }
#endif

   return 0;
}



int disp_path_config_layer(OVL_CONFIG_STRUCT* pOvlConfig)
{
   //    unsigned int reg_addr;

   DISP_LOG("[DDP]disp_path_config_layer(), layer=%d, source=%d, fmt=%d, addr=0x%x, x=%d, y=%d \n\
             w=%d, h=%d, pitch=%d, keyEn=%d, key=%d, aen=%d, alpha=%d \n ", 
             pOvlConfig->layer,   // layer
             pOvlConfig->source,   // data source (0=memory)
             pOvlConfig->fmt, 
             pOvlConfig->addr, // addr 
             pOvlConfig->x,  // x
             pOvlConfig->y,  // y
             pOvlConfig->w, // width
             pOvlConfig->h, // height
             pOvlConfig->pitch, //pitch, pixel number
             pOvlConfig->keyEn,  //color key
             pOvlConfig->key,  //color key
             pOvlConfig->aen, // alpha enable
             pOvlConfig->alpha);	

   // config overlay
   OVLLayerSwitch(pOvlConfig->layer, pOvlConfig->layer_en);

   if(pOvlConfig->layer_en!=0)
   {
      OVLLayerConfig(pOvlConfig->layer,   // layer
                                 pOvlConfig->source,   // data source (0=memory)
                                 pOvlConfig->fmt, 
                                 pOvlConfig->addr, // addr 
                                 pOvlConfig->x,  // x
                                 pOvlConfig->y,  // y
                                 pOvlConfig->w, // width
                                 pOvlConfig->h, // height
                                 pOvlConfig->pitch, //pitch, pixel number
                                 pOvlConfig->keyEn,  //color key
                                 pOvlConfig->key,  //color key
                                 pOvlConfig->aen, // alpha enable
                                 pOvlConfig->alpha); // alpha
   }    

   //    printf("[DDP]disp_path_config_layer() done, addr=0x%x \n", pOvlConfig->addr);
   
   return 0;
}

int disp_path_config_layer_addr(unsigned int layer, unsigned int addr)
{
   unsigned int reg_addr = 0;
   

   DISP_LOG("[DDP]disp_path_config_layer_addr(), layer=%d, addr=0x%x\n ", layer, addr);
   
   switch(layer)
   {
      case 0:
         DISP_REG_SET(DISP_REG_OVL_L0_ADDR, addr);
         reg_addr = DISP_REG_OVL_L0_ADDR;
         break;
      case 1:
         DISP_REG_SET(DISP_REG_OVL_L1_ADDR, addr);
         reg_addr = DISP_REG_OVL_L1_ADDR;
         break;
      case 2:
         DISP_REG_SET(DISP_REG_OVL_L2_ADDR, addr);
         reg_addr = DISP_REG_OVL_L2_ADDR;
         break;
      case 3:
         DISP_REG_SET(DISP_REG_OVL_L3_ADDR, addr);
         reg_addr = DISP_REG_OVL_L3_ADDR;
         break;
      default:
         printf("[DDP] error! error: unknow layer=%d \n", layer);
         ASSERT(0);
   }
   DISP_LOG("[DDP]disp_path_config_layer_addr() done, addr=0x%x \n", DISP_REG_GET(reg_addr));
   
   return 0;
}

int disp_path_ddp_clock_on()
{    
    DISP_REG_SET(DISP_REG_CONFIG_CG_CLR0, 0x002081F0);
    DISP_REG_SET(0x14010064, 0xFFFFF);

    DISP_LOG("[DISP] - disp_path_ddp_clock_on 0. 0x%8x\n", INREG32(DISP_REG_CONFIG_CG_CON0));
    return 0;	
}

int disp_path_ddp_clock_off()
{    
    DISP_REG_SET(DISP_REG_CONFIG_CG_SET0, 0x002081F0);

    DISP_LOG("[DISP] - disp_path_ddp_clock_off 0. 0x%8x\n", INREG32(DISP_REG_CONFIG_CG_CON0));
    return 0;	
}


int disp_path_config(struct disp_path_config_struct* pConfig)
{
   ///> get mutex and set mout/sel
   //        unsigned int gMutexID = 0;
   unsigned int mutex_mode  = 0;
   

   DISP_LOG("[DDP]disp_path_config(), srcModule=%d, addr=0x%x, inFormat=%d, \n\
             pitch=%d, bgROI(%d,%d,%d,%d), bgColor=%d, outFormat=%d, dstModule=%d, dstAddr=0x%x,  \n",
             pConfig->srcModule,            
             pConfig->addr,  
             pConfig->inFormat,  
             pConfig->pitch, 
             pConfig->bgROI.x, 
             pConfig->bgROI.y, 
             pConfig->bgROI.width, 
             pConfig->bgROI.height, 
             pConfig->bgColor, 
             pConfig->outFormat,  
             pConfig->dstModule, 
             pConfig->dstAddr);

   if (pConfig->srcModule==DISP_MODULE_RDMA0 && pConfig->dstModule==DISP_MODULE_WDMA0)
   {
      printf("[DDP] error! rdma0 wdma1 can not enable together! \n");
      return -1;
   }

   
   switch(pConfig->dstModule)
   {
      case DISP_MODULE_DSI_VDO:
         mutex_mode = 1;
         break;
      
      case DISP_MODULE_DPI0:
         mutex_mode = 2;
         break;
      
      case DISP_MODULE_DBI:
      case DISP_MODULE_DSI_CMD:
      case DISP_MODULE_WDMA0:
         mutex_mode = 0;
         break;
      
      default:
         printf("[DDP] error! unknown dstModule=%d \n", pConfig->dstModule); 
         ASSERT(0);
   }
   
   
   DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gMutexID), 1);
   DISP_REG_SET(DISP_REG_CONFIG_MUTEX_RST(gMutexID), 0);

   if(pConfig->srcModule==DISP_MODULE_RDMA0)
   {
      DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), 0x680);  // [0]: MDP_WROT
                                                                                                                   // [1]: MDP_PRZ0
                                                                                                                   // [2]: MDP_PRZ1
                                                                                                                   // [3]: DISP_OVL
                                                                                                                   // [4]: MDP_WDMA
                                                                                                                   // [5]: MDP_RDMA
                                                                                                                   // [6]: DISP_WDMA
                                                                                                                   // [7]: DISP_COLOR
                                                                                                                   // [8]: MDP_TDSHP
                                                                                                                   // [9]: DISP_BLS
                                                                                                                   // [10]: DISP_RDMA
                                                                                                                   // [11]: ISP_MOUT
   }
   else
   {
      if (pConfig->dstModule==DISP_MODULE_WDMA0)
      {

         DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), 0x48);  // [0]: MDP_WROT
                                                                                                                   // [1]: MDP_PRZ0
                                                                                                                   // [2]: MDP_PRZ1
                                                                                                                   // [3]: DISP_OVL
                                                                                                                   // [4]: MDP_WDMA
                                                                                                                   // [5]: MDP_RDMA
                                                                                                                   // [6]: DISP_WDMA
                                                                                                                   // [7]: DISP_COLOR
                                                                                                                   // [8]: MDP_TDSHP
                                                                                                                   // [9]: DISP_BLS
                                                                                                                   // [10]: DISP_RDMA
                                                                                                                   // [11]: ISP_MOUT

      }
      else
      {
         // Elsa: de-couple BLS from OVL stream
         //DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), 0x284); //ovl=2, bls=9, rdma0=7

         DISP_REG_SET(DISP_REG_CONFIG_MUTEX_MOD(gMutexID), 0x688);  // [0]: MDP_WROT
                                                                                                                   // [1]: MDP_PRZ0
                                                                                                                   // [2]: MDP_PRZ1
                                                                                                                   // [3]: DISP_OVL
                                                                                                                   // [4]: MDP_WDMA
                                                                                                                   // [5]: MDP_RDMA
                                                                                                                   // [6]: DISP_WDMA
                                                                                                                   // [7]: DISP_COLOR
                                                                                                                   // [8]: MDP_TDSHP
                                                                                                                   // [9]: DISP_BLS
                                                                                                                   // [10]: DISP_RDMA
                                                                                                                   // [11]: ISP_MOUT
      }
   }		
   DISP_REG_SET(DISP_REG_CONFIG_MUTEX_SOF(gMutexID), mutex_mode);
   DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTSTA, (1 << gMutexID));
   DISP_REG_SET(DISP_REG_CONFIG_MUTEX_INTEN, (1 << gMutexID));        
   DISP_REG_SET(DISP_REG_CONFIG_MUTEX_EN(gMutexID), 1);        
   
   ///> config config reg
   switch(pConfig->dstModule)
   {
      case DISP_MODULE_DSI:
      case DISP_MODULE_DSI_VDO:
      case DISP_MODULE_DSI_CMD:
         DISP_REG_SET(DISP_REG_CONFIG_OVL_MOUT_EN, 0x1);  // OVL output, [0]: DISP_RDMA, [1]: DISP_WDMA, [2]: DISP_COLOR
         DISP_REG_SET(DISP_REG_CONFIG_DISP_OUT_SEL, 0x0);  // display output, 0: DSI, 1: DPI, 2: DBI
         DISP_REG_SET(DISP_REG_CONFIG_RDMA0_OUT_SEL, 0x0);  // RDMA output, 0: DISP_COLOR, 1: DSI
         DISP_REG_SET(DISP_REG_CONFIG_COLOR_SEL, 0x0);  // color input, 0: DISP_RDMA, 1: DISP_OVL
         DISP_REG_SET(DISP_REG_CONFIG_DSI_SEL, 0x0);  // DSI input, 0: DISP_BLS, 1: DISP_RDMA
         break;
      
      case DISP_MODULE_DPI0:
         DISP_LOG("DISI_MODULE_DPI0\n");
         DISP_REG_SET(DISP_REG_CONFIG_OVL_MOUT_EN, 0x1);  // OVL output, [0]: DISP_RDMA, [1]: DISP_WDMA, [2]: DISP_COLOR
         DISP_REG_SET(DISP_REG_CONFIG_DISP_OUT_SEL, 0x1);  // display output, 0: DSI, 1: DPI, 2: DBI
         DISP_REG_SET(DISP_REG_CONFIG_RDMA0_OUT_SEL, 0x0);  // RDMA output, 0: DISP_COLOR, 1: DSI
         DISP_REG_SET(DISP_REG_CONFIG_DPI0_SEL, 0);  // DPI input, 0: DISP_BLS, 1: DISP_RDMA
         DISP_REG_SET(DISP_REG_CONFIG_COLOR_SEL, 0x0);  // color input, 0: DISP_RDMA, 1: DISP_OVL
         break;
      
      case DISP_MODULE_DBI:
         DISP_REG_SET(DISP_REG_CONFIG_OVL_MOUT_EN, 0x1);  // OVL output, [0]: DISP_RDMA, [1]: DISP_WDMA, [2]: DISP_COLOR
         DISP_REG_SET(DISP_REG_CONFIG_DISP_OUT_SEL, 0x2);  // display output, 0: DSI, 1: DPI, 2: DBI
         DISP_REG_SET(DISP_REG_CONFIG_RDMA0_OUT_SEL, 0x0);  // RDMA output, 0: DISP_COLOR, 1: DSI
         DISP_REG_SET(DISP_REG_CONFIG_COLOR_SEL, 0x0);  // color input, 0: DISP_RDMA, 1: DISP_OVL
         break;

      case DISP_MODULE_WDMA0:
         DISP_REG_SET(DISP_REG_CONFIG_OVL_MOUT_EN, 0x1);  // OVL output, 0: DISP_RDMA, 1: DISP_WDMA, 2: DISP_COLOR
         break;
      
      default:
         printf("[DDP] error! unknown dstModule=%d \n", pConfig->dstModule); 
   }    
   
   ///> config engines
   if(pConfig->srcModule!=DISP_MODULE_RDMA0)
   {            // config OVL
      OVLStop();
      OVLROI(pConfig->bgROI.width, // width
                   pConfig->bgROI.height, // height
                   pConfig->bgColor);// background B
      
      OVLLayerSwitch(pConfig->ovl_config.layer, pConfig->ovl_config.layer_en);
      if(pConfig->ovl_config.layer_en!=0)
      {
         OVLLayerConfig(pConfig->ovl_config.layer,   // layer
                                    pConfig->ovl_config.source,   // data source (0=memory)
                                    pConfig->ovl_config.fmt, 
                                    pConfig->ovl_config.addr, // addr 
                                    pConfig->ovl_config.x,  // x
                                    pConfig->ovl_config.y,  // y
                                    pConfig->ovl_config.w, // width
                                    pConfig->ovl_config.h, // height
                                    pConfig->ovl_config.pitch,
                                    pConfig->ovl_config.keyEn,  //color key
                                    pConfig->ovl_config.key,  //color key
                                    pConfig->ovl_config.aen, // alpha enable
                                    pConfig->ovl_config.alpha); // alpha
      }
      
      OVLStart();

      if (pConfig->dstModule==DISP_MODULE_WDMA0)
      {
         WDMAReset(1);
         WDMAConfig(1, 
                              WDMA_INPUT_FORMAT_ARGB, 
                              pConfig->srcROI.width, 
                              pConfig->srcROI.height, 
                              0, 
                              0, 
                              pConfig->srcROI.width, 
                              pConfig->srcROI.height, 
                              pConfig->outFormat, 
                              pConfig->dstAddr, 
                              pConfig->srcROI.width, 
                              1, 
                              0);      
         WDMAStart(1);
      }
      else    //2. ovl->bls->rdma0->lcd
      {
                
                
                disp_bls_init(pConfig->srcROI.width, pConfig->srcROI.height);

                // color setting
                DISP_REG_SET(DISP_REG_COLOR_INTERNAL_IP_WIDTH, pConfig->srcROI.width);
                DISP_REG_SET(DISP_REG_COLOR_INTERNAL_IP_HEIGHT, pConfig->srcROI.height);
                DISP_REG_SET(DISP_REG_COLOR_CFG_MAIN, 0x200000BF);
                DISP_REG_SET(DISP_REG_COLOR_START, 1);

         
         ///config RDMA
         RDMAStop(0);
         RDMAReset(0);
         RDMAConfig(0, 
                             RDMA_MODE_DIRECT_LINK,       ///direct link mode
                             RDMA_INPUT_FORMAT_RGB888,    // inputFormat
                             (unsigned int)NULL,                        // address
                             pConfig->outFormat,          // output format
                             pConfig->pitch,              // pitch
                             pConfig->srcROI.width,       // width
                             pConfig->srcROI.height,      // height
                             0,                           //byte swap
                             0);                          // is RGB swap        
         
         RDMAStart(0);
      }
   }
   else  //3. mem->rdma->lcd
   {
      ///config RDMA
      RDMAStop(0);
      RDMAReset(0);
      RDMAConfig(0, 
                          RDMA_MODE_MEMORY,       ///direct link mode
                          pConfig->inFormat,      // inputFormat
                          pConfig->addr,          // address
                          pConfig->outFormat,     // output format
                          pConfig->pitch,          //                                         
                          pConfig->srcROI.width,
                          pConfig->srcROI.height,
                          0,                       //byte swap    
                          0);                      // is RGB swap          
      RDMAStart(0);
   }

   return 0;
}

unsigned int dbg_log = 1;
unsigned int irq_log = 0;
#define DISP_WRN(string, args...) if(dbg_log) printf("[DSS]"string,##args)
#define DISP_MSG(string, args...) printf("[DSS]"string,##args)
#define DISP_ERR(string, args...) printf("[DSS]error:"string,##args)
#define DISP_IRQ(string, args...) if(irq_log) printf("[DSS]"string,##args)

int disp_dump_reg(DISP_MODULE_ENUM module)           
{
   unsigned int index;    

   switch(module)
   {        
      case DISP_MODULE_CONFIG: 
         DISP_MSG("===== DISP CFG Reg Dump: ============\n");           
         DISP_MSG("(020)DISP_REG_CONFIG_RDMA_MOUT_EN                    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_RDMA_MOUT_EN                     ));
         DISP_MSG("(024)DISP_REG_CONFIG_RSZ0_MOUT_EN                  =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_RSZ0_MOUT_EN                 ));
         DISP_MSG("(028)DISP_REG_CONFIG_RSZ1_MOUT_EN                 =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_RSZ1_MOUT_EN                 ));
         DISP_MSG("(02C)DISP_REG_CONFIG_TDSHP_MOUT_EN                     =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_TDSHP_MOUT_EN                     ));
         DISP_MSG("(030)DISP_REG_CONFIG_OVL_MOUT_EN                   =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_OVL_MOUT_EN                  ));
         DISP_MSG("(034)DISP_REG_CONFIG_MOUT_RST                    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MOUT_RST                     ));
         DISP_MSG("(038)DISP_REG_CONFIG_RSZ0_SEL               =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_RSZ0_SEL              ));
         DISP_MSG("(03C)DISP_REG_CONFIG_RSZ1_SEL           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_RSZ1_SEL          ));
         DISP_MSG("(040)DISP_REG_CONFIG_TDSHP_SEL              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_TDSHP_SEL             ));
         DISP_MSG("(044)DISP_REG_CONFIG_WROT_SEL                =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_WROT_SEL                 ));
         DISP_MSG("(048)DISP_REG_CONFIG_WDMA0_SEL                 =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_WDMA0_SEL                 ));
         DISP_MSG("(04C)DISP_REG_CONFIG_DISP_OUT_SEL                    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DISP_OUT_SEL                     ));
         DISP_MSG("(050)DISP_REG_CONFIG_RDMA0_OUT_SEL                  =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_RDMA0_OUT_SEL                 ));
         DISP_MSG("(054)DISP_REG_CONFIG_COLOR_SEL                 =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_COLOR_SEL                 ));
         DISP_MSG("(058)DISP_REG_CONFIG_DSI_SEL                     =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DSI_SEL                     ));
         DISP_MSG("(05C)DISP_REG_CONFIG_DPI0_SEL                   =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DPI0_SEL                  ));
         DISP_MSG("(100)DISP_REG_CONFIG_CG_CON0                    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_CG_CON0                     ));
         DISP_MSG("(104)DISP_REG_CONFIG_CG_SET0               =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_CG_SET0              ));
         DISP_MSG("(108)DISP_REG_CONFIG_CG_CLR0           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_CG_CLR0          ));
         DISP_MSG("(110)DISP_REG_CONFIG_CG_CON1              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_CG_CON1             ));
         DISP_MSG("(114)DISP_REG_CONFIG_CG_SET1                =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_CG_SET1                 ));
         DISP_MSG("(118)DISP_REG_CONFIG_CG_CLR1                 =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_CG_CLR1                 ));
         DISP_MSG("(120)DISP_REG_CONFIG_HW_DCM_EN0                    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN0                     ));
         DISP_MSG("(124)DISP_REG_CONFIG_HW_DCM_EN_SET0                  =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN_SET0                 ));
         DISP_MSG("(128)DISP_REG_CONFIG_HW_DCM_EN_CLR0                 =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN_CLR0                 ));
         DISP_MSG("(130)DISP_REG_CONFIG_HW_DCM_EN1                     =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN1                     ));
         DISP_MSG("(134)DISP_REG_CONFIG_HW_DCM_EN_SET1                   =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN_SET1                  ));
         DISP_MSG("(138)DISP_REG_CONFIG_HW_DCM_EN_CLR1                    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_HW_DCM_EN_CLR1                     ));
         DISP_MSG("(800)DISP_REG_CONFIG_MBIST_DONE0               =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_DONE0              ));
         DISP_MSG("(804)DISP_REG_CONFIG_MBIST_FAIL0           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_FAIL0          ));
         DISP_MSG("(808)DISP_REG_CONFIG_MBIST_FAIL1              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_FAIL1             ));
         DISP_MSG("(80C)DISP_REG_CONFIG_MBIST_HOLDB0                =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_HOLDB0                 ));
         DISP_MSG("(810)DISP_REG_CONFIG_MBIST_MODE0                 =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_MODE0                 ));
         DISP_MSG("(814)DISP_REG_CONFIG_MBIST_BSEL0                    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_BSEL0                     ));
         DISP_MSG("(818)DISP_REG_CONFIG_MBIST_BSEL1                  =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_BSEL1                 ));
         DISP_MSG("(81C)DISP_REG_CONFIG_MBIST_BSEL2                 =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_BSEL2                 ));
         DISP_MSG("(820)DISP_REG_CONFIG_MBIST_CON                     =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MBIST_CON                     ));
         DISP_MSG("(824)DISP_REG_CONFIG_MEM_DELSEL0                   =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MEM_DELSEL0                  ));
         DISP_MSG("(828)DISP_REG_CONFIG_MEM_DELSEL1                    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MEM_DELSEL1                     ));
         DISP_MSG("(82C)DISP_REG_CONFIG_MEM_DELSEL2               =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MEM_DELSEL2              ));
         DISP_MSG("(830)DISP_REG_CONFIG_MEM_DELSEL3           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MEM_DELSEL3          ));
         DISP_MSG("(834)DISP_REG_CONFIG_MEM_DELSEL4              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MEM_DELSEL4             ));
         DISP_MSG("(838)DISP_REG_CONFIG_MEM_DELSEL5                =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MEM_DELSEL5                 ));
         DISP_MSG("(83C)DISP_REG_CONFIG_MEM_DELSEL6                 =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MEM_DELSEL6                 ));
         DISP_MSG("(840)DISP_REG_CONFIG_DEBUG_OUT_SEL                    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DEBUG_OUT_SEL                     ));
         DISP_MSG("(844)DISP_REG_CONFIG_DUMMY                  =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DUMMY                 ));
         DISP_MSG("(850)DISP_REG_CONFIG_MROT_MBISR_RESET                 =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MROT_MBISR_RESET                 ));
         DISP_MSG("(854)DISP_REG_CONFIG_MROT_MBISR_FAIL                     =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MROT_MBISR_FAIL                     ));
         DISP_MSG("(858)DISP_REG_CONFIG_MROT_MBISR_OK                   =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MROT_MBISR_OK                  ));
         DISP_MSG("(860)DISP_REG_CONFIG_DL_VALID0                    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DL_VALID0                     ));
         DISP_MSG("(864)DISP_REG_CONFIG_DL_VALID1               =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DL_VALID1              ));
         DISP_MSG("(868)DISP_REG_CONFIG_DL_READY0           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DL_READY0          ));
         DISP_MSG("(86C)DISP_REG_CONFIG_DL_READY1              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_DL_READY1             ));
         break;

      case DISP_MODULE_OVL: 
         DISP_MSG("===== DISP OVL Reg Dump: ============\n");           
         DISP_MSG("(000)OVL_STA                    =0x%x \n", DISP_REG_GET(DISP_REG_OVL_STA                     ));
         DISP_MSG("(004)OVL_INTEN                  =0x%x \n", DISP_REG_GET(DISP_REG_OVL_INTEN                 ));
         DISP_MSG("(008)OVL_INTSTA                 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_INTSTA                 ));
         DISP_MSG("(00C)OVL_EN                     =0x%x \n", DISP_REG_GET(DISP_REG_OVL_EN                     ));
         DISP_MSG("(010)OVL_TRIG                   =0x%x \n", DISP_REG_GET(DISP_REG_OVL_TRIG                  ));
         DISP_MSG("(014)OVL_RST                    =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RST                     ));
         DISP_MSG("(020)OVL_ROI_SIZE               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_ROI_SIZE              ));
         DISP_MSG("(024)OVL_DATAPATH_CON           =0x%x \n", DISP_REG_GET(DISP_REG_OVL_DATAPATH_CON          ));
         DISP_MSG("(028)OVL_ROI_BGCLR              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_ROI_BGCLR             ));
         DISP_MSG("(02C)OVL_SRC_CON                =0x%x \n", DISP_REG_GET(DISP_REG_OVL_SRC_CON                 ));
         DISP_MSG("(030)OVL_L0_CON                 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_CON                 ));
         DISP_MSG("(034)OVL_L0_SRCKEY              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_SRCKEY             ));
         DISP_MSG("(038)OVL_L0_SRC_SIZE            =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_SRC_SIZE             ));
         DISP_MSG("(03C)OVL_L0_OFFSET              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_OFFSET             ));
         DISP_MSG("(040)OVL_L0_ADDR                =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_ADDR                 ));
         DISP_MSG("(044)OVL_L0_PITCH               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L0_PITCH              ));
         DISP_MSG("(0C0)OVL_RDMA0_CTRL             =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_CTRL             ));
         DISP_MSG("(0C4)OVL_RDMA0_MEM_START_TRIG   =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_START_TRIG  ));
         DISP_MSG("(0C8)OVL_RDMA0_MEM_GMC_SETTING  =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_GMC_SETTING ));
         DISP_MSG("(0CC)OVL_RDMA0_MEM_SLOW_CON     =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_SLOW_CON     ));
         DISP_MSG("(0D0)OVL_RDMA0_FIFO_CTRL        =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_FIFO_CTRL         ));
         DISP_MSG("(050)OVL_L1_CON                 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_CON                 ));
         DISP_MSG("(054)OVL_L1_SRCKEY              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_SRCKEY             ));
         DISP_MSG("(058)OVL_L1_SRC_SIZE            =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_SRC_SIZE             ));
         DISP_MSG("(05C)OVL_L1_OFFSET              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_OFFSET             ));
         DISP_MSG("(060)OVL_L1_ADDR                =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_ADDR                 ));
         DISP_MSG("(064)OVL_L1_PITCH               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L1_PITCH              ));
         DISP_MSG("(0E0)OVL_RDMA1_CTRL             =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_CTRL             ));
         DISP_MSG("(0E4)OVL_RDMA1_MEM_START_TRIG   =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_START_TRIG  ));
         DISP_MSG("(0E8)OVL_RDMA1_MEM_GMC_SETTING  =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_GMC_SETTING ));
         DISP_MSG("(0EC)OVL_RDMA1_MEM_SLOW_CON     =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_SLOW_CON     ));
         DISP_MSG("(0F0)OVL_RDMA1_FIFO_CTRL        =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_FIFO_CTRL         ));
         DISP_MSG("(070)OVL_L2_CON                 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_CON                 ));
         DISP_MSG("(074)OVL_L2_SRCKEY              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_SRCKEY             ));
         DISP_MSG("(078)OVL_L2_SRC_SIZE            =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_SRC_SIZE             ));
         DISP_MSG("(07C)OVL_L2_OFFSET              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_OFFSET             ));
         DISP_MSG("(080)OVL_L2_ADDR                =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_ADDR                 ));
         DISP_MSG("(084)OVL_L2_PITCH               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L2_PITCH              ));
         DISP_MSG("(100)OVL_RDMA2_CTRL             =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_CTRL             ));
         DISP_MSG("(104)OVL_RDMA2_MEM_START_TRIG   =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_START_TRIG  ));
         DISP_MSG("(108)OVL_RDMA2_MEM_GMC_SETTING  =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_GMC_SETTING ));
         DISP_MSG("(10C)OVL_RDMA2_MEM_SLOW_CON     =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_SLOW_CON     ));
         DISP_MSG("(110)OVL_RDMA2_FIFO_CTRL        =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_FIFO_CTRL         ));
         DISP_MSG("(090)OVL_L3_CON                 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_CON                 ));
         DISP_MSG("(094)OVL_L3_SRCKEY              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_SRCKEY             ));
         DISP_MSG("(098)OVL_L3_SRC_SIZE            =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_SRC_SIZE             ));
         DISP_MSG("(09C)OVL_L3_OFFSET              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_OFFSET             ));
         DISP_MSG("(0A0)OVL_L3_ADDR                =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_ADDR                 ));
         DISP_MSG("(0A4)OVL_L3_PITCH               =0x%x \n", DISP_REG_GET(DISP_REG_OVL_L3_PITCH              ));
         DISP_MSG("(120)OVL_RDMA3_CTRL             =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_CTRL             ));
         DISP_MSG("(124)OVL_RDMA3_MEM_START_TRIG   =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_START_TRIG  ));
         DISP_MSG("(128)OVL_RDMA3_MEM_GMC_SETTING  =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_GMC_SETTING ));
         DISP_MSG("(12C)OVL_RDMA3_MEM_SLOW_CON     =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_SLOW_CON     ));
         DISP_MSG("(130)OVL_RDMA3_FIFO_CTRL        =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_FIFO_CTRL         ));
         DISP_MSG("(1C4)OVL_DEBUG_MON_SEL          =0x%x \n", DISP_REG_GET(DISP_REG_OVL_DEBUG_MON_SEL         ));
         DISP_MSG("(1C4)OVL_RDMA0_MEM_GMC_SETTING2 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_MEM_GMC_SETTING2));
         DISP_MSG("(1C8)OVL_RDMA1_MEM_GMC_SETTING2 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_MEM_GMC_SETTING2));
         DISP_MSG("(1CC)OVL_RDMA2_MEM_GMC_SETTING2 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_MEM_GMC_SETTING2));
         DISP_MSG("(1D0)OVL_RDMA3_MEM_GMC_SETTING2 =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_MEM_GMC_SETTING2));
         DISP_MSG("(240)OVL_FLOW_CTRL_DBG          =0x%x \n", DISP_REG_GET(DISP_REG_OVL_FLOW_CTRL_DBG         ));
         DISP_MSG("(244)OVL_ADDCON_DBG             =0x%x \n", DISP_REG_GET(DISP_REG_OVL_ADDCON_DBG             ));
         DISP_MSG("(248)OVL_OUTMUX_DBG             =0x%x \n", DISP_REG_GET(DISP_REG_OVL_OUTMUX_DBG             ));
         DISP_MSG("(24C)OVL_RDMA0_DBG              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA0_DBG             ));
         DISP_MSG("(250)OVL_RDMA1_DBG              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA1_DBG             ));
         DISP_MSG("(254)OVL_RDMA2_DBG              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA2_DBG             ));
         DISP_MSG("(258)OVL_RDMA3_DBG              =0x%x \n", DISP_REG_GET(DISP_REG_OVL_RDMA3_DBG             ));
         break;
      
      case DISP_MODULE_COLOR:  
         DISP_MSG("===== DISP COLOR Reg Dump: ============\n");           
         DISP_MSG("(400)DISP_REG_COLOR_CFG_MAIN                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CFG_MAIN                     ));
         DISP_MSG("(404)DISP_REG_COLOR_PXL_CNT_MAIN                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_PXL_CNT_MAIN                 ));
         DISP_MSG("(408)DISP_REG_COLOR_LINE_CNT_MAIN                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LINE_CNT_MAIN                 ));
         DISP_MSG("(40C)DISP_REG_COLOR_WIN_X_MAIN                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_WIN_X_MAIN                     ));
         DISP_MSG("(410)DISP_REG_COLOR_WIN_Y_MAIN                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_WIN_Y_MAIN                  ));
         DISP_MSG("(418)DISP_REG_COLOR_TIMING_DETECTION_0                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_TIMING_DETECTION_0                     ));
         DISP_MSG("(41C)DISP_REG_COLOR_TIMING_DETECTION_1                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_TIMING_DETECTION_1                 ));
         DISP_MSG("(420)DISP_REG_COLOR_DBG_CFG_MAIN                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_DBG_CFG_MAIN                     ));
         DISP_MSG("(428)DISP_REG_COLOR_C_BOOST_MAIN                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_C_BOOST_MAIN                 ));
         DISP_MSG("(42C)DISP_REG_COLOR_C_BOOST_MAIN_2                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_C_BOOST_MAIN_2                 ));
         DISP_MSG("(430)DISP_REG_COLOR_LUMA_ADJ                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_ADJ                     ));
         DISP_MSG("(434)DISP_REG_COLOR_G_PIC_ADJ_MAIN_1                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_G_PIC_ADJ_MAIN_1                  ));
         DISP_MSG("(438)DISP_REG_COLOR_G_PIC_ADJ_MAIN_2                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_G_PIC_ADJ_MAIN_2                     ));
         DISP_MSG("(440)DISP_REG_COLOR_Y_FTN_1_0_MAIN                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_1_0_MAIN                 ));
         DISP_MSG("(444)DISP_REG_COLOR_Y_FTN_3_2_MAIN                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_3_2_MAIN                     ));
         DISP_MSG("(448)DISP_REG_COLOR_Y_FTN_5_4_MAIN                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_5_4_MAIN                 ));
         DISP_MSG("(44C)DISP_REG_COLOR_Y_FTN_7_6_MAIN                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_7_6_MAIN                 ));
         DISP_MSG("(450)DISP_REG_COLOR_Y_FTN_9_8_MAIN                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_9_8_MAIN                     ));
         DISP_MSG("(454)DISP_REG_COLOR_Y_FTN_11_10_MAIN                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_11_10_MAIN                  ));
         DISP_MSG("(458)DISP_REG_COLOR_Y_FTN_13_12_MAIN                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_13_12_MAIN                     ));
         DISP_MSG("(45C)DISP_REG_COLOR_Y_FTN_15_14_MAIN                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_15_14_MAIN                 ));
         DISP_MSG("(460)DISP_REG_COLOR_Y_FTN_17_16_MAIN                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_Y_FTN_17_16_MAIN                     ));
         DISP_MSG("(484)DISP_REG_COLOR_POS_MAIN                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_POS_MAIN                 ));
         DISP_MSG("(488)DISP_REG_COLOR_INK_DATA_MAIN                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_INK_DATA_MAIN                 ));
         DISP_MSG("(48C)DISP_REG_COLOR_INK_DATA_MAIN_CR                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_INK_DATA_MAIN_CR                     ));
         DISP_MSG("(490)DISP_REG_COLOR_CAP_IN_DATA_MAIN                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CAP_IN_DATA_MAIN                  ));
         DISP_MSG("(494)DISP_REG_COLOR_CAP_IN_DATA_MAIN_CR                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CAP_IN_DATA_MAIN_CR                     ));
         DISP_MSG("(498)DISP_REG_COLOR_LUMA_HIST_00                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_00                 ));
         DISP_MSG("(49C)DISP_REG_COLOR_CAP_OUT_DATA_MAIN_CR                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CAP_OUT_DATA_MAIN_CR                     ));
         DISP_MSG("(520)DISP_REG_COLOR_LUMA_HIST_00                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_00                 ));
         DISP_MSG("(524)DISP_REG_COLOR_LUMA_HIST_01                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_01                 ));
         DISP_MSG("(528)DISP_REG_COLOR_LUMA_HIST_02                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_02                     ));
         DISP_MSG("(52C)DISP_REG_COLOR_LUMA_HIST_03                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_03                  ));
         DISP_MSG("(530)DISP_REG_COLOR_LUMA_HIST_04                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_04                     ));
         DISP_MSG("(534)DISP_REG_COLOR_LUMA_HIST_05                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_05                 ));
         DISP_MSG("(538)DISP_REG_COLOR_LUMA_HIST_06                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_06                     ));
         DISP_MSG("(53C)DISP_REG_COLOR_LUMA_HIST_07                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_07                 ));
         DISP_MSG("(540)DISP_REG_COLOR_LUMA_HIST_08                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_08                 ));
         DISP_MSG("(544)DISP_REG_COLOR_LUMA_HIST_09                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_09                     ));
         DISP_MSG("(548)DISP_REG_COLOR_LUMA_HIST_10                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_10                  ));
         DISP_MSG("(54C)DISP_REG_COLOR_LUMA_HIST_11                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_11                     ));
         DISP_MSG("(550)DISP_REG_COLOR_LUMA_HIST_12                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_12                 ));
         DISP_MSG("(554)DISP_REG_COLOR_LUMA_HIST_13                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_13                     ));
         DISP_MSG("(558)DISP_REG_COLOR_LUMA_HIST_14                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_14                 ));
         DISP_MSG("(55C)DISP_REG_COLOR_LUMA_HIST_15                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_15                 ));
         DISP_MSG("(560)DISP_REG_COLOR_LUMA_HIST_16                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_HIST_16                     ));
         DISP_MSG("(5A4)DISP_REG_COLOR_LUMA_SUM                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_SUM                  ));
         DISP_MSG("(5A8)DISP_REG_COLOR_LUMA_MIN_MAX                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LUMA_MIN_MAX                     ));
         DISP_MSG("(620)DISP_REG_COLOR_LOCAL_HUE_CD_0                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LOCAL_HUE_CD_0                 ));
         DISP_MSG("(624)DISP_REG_COLOR_LOCAL_HUE_CD_1                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LOCAL_HUE_CD_1                     ));
         DISP_MSG("(628)DISP_REG_COLOR_LOCAL_HUE_CD_2                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LOCAL_HUE_CD_2                 ));
         DISP_MSG("(62C)DISP_REG_COLOR_LOCAL_HUE_CD_3                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LOCAL_HUE_CD_3                 ));
         DISP_MSG("(630)DISP_REG_COLOR_LOCAL_HUE_CD_4                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_LOCAL_HUE_CD_4                     ));
         DISP_MSG("(740)DISP_REG_COLOR_TWO_D_WINDOW_1                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_TWO_D_WINDOW_1                  ));
         DISP_MSG("(74C)DISP_REG_COLOR_TWO_D_W1_RESULT                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_TWO_D_W1_RESULT                     ));
         DISP_MSG("(768)DISP_REG_COLOR_SAT_HIST_X_CFG_MAIN                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_SAT_HIST_X_CFG_MAIN                 ));
         DISP_MSG("(76C)DISP_REG_COLOR_SAT_HIST_Y_CFG_MAIN                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_SAT_HIST_Y_CFG_MAIN                     ));
         DISP_MSG("(7E0)DISP_REG_COLOR_CRC                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CRC(0)                 ));
         DISP_MSG("(7FC)DISP_REG_COLOR_PARTIAL_SAT_GAIN1                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_PARTIAL_SAT_GAIN1(0)                 ));
         DISP_MSG("(810)DISP_REG_COLOR_PARTIAL_SAT_GAIN2                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_PARTIAL_SAT_GAIN2(0)                     ));
         DISP_MSG("(824)DISP_REG_COLOR_PARTIAL_SAT_GAIN3                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_PARTIAL_SAT_GAIN3(0)                  ));
         DISP_MSG("(838)DISP_REG_COLOR_PARTIAL_SAT_POINT1                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_PARTIAL_SAT_POINT1(0)                     ));
         DISP_MSG("(84C)DISP_REG_COLOR_PARTIAL_SAT_POINT2                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_PARTIAL_SAT_POINT2(0)                 ));
         DISP_MSG("(F00)DISP_REG_COLOR_START                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_START                     ));
         DISP_MSG("(F04)DISP_REG_COLOR_INTEN                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_INTEN                 ));
         DISP_MSG("(F08)DISP_REG_COLOR_INTSTA                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_INTSTA                 ));
         DISP_MSG("(F0C)DISP_REG_COLOR_OUT_SEL                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_OUT_SEL                     ));
         DISP_MSG("(F10)DISP_REG_COLOR_FRAME_DONE_DEL                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_FRAME_DONE_DEL                  ));
         DISP_MSG("(76C)DISP_REG_COLOR_CRC_EN                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CRC_EN                     ));
         DISP_MSG("(7E0)DISP_REG_COLOR_SW_SCRATCH                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_SW_SCRATCH                 ));
         DISP_MSG("(7FC)DISP_REG_COLOR_RDY_SEL                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_RDY_SEL                 ));
         DISP_MSG("(810)DISP_REG_COLOR_RDY_SEL_EN                     =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_RDY_SEL_EN                     ));
         DISP_MSG("(824)DISP_REG_COLOR_CK_ON                   =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CK_ON                  ));
         DISP_MSG("(F50)DISP_REG_COLOR_INTERNAL_IP_WIDTH                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_INTERNAL_IP_WIDTH                     ));
         DISP_MSG("(F54)DISP_REG_COLOR_INTERNAL_IP_HEIGHT                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_INTERNAL_IP_HEIGHT                 ));
         DISP_MSG("(F60)DISP_REG_COLOR_CM1_EN                    =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CM1_EN                     ));
         DISP_MSG("(FA0)DISP_REG_COLOR_CM2_EN                  =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_CM2_EN                 ));
         DISP_MSG("(FF0)DISP_REG_COLOR_R0_CRC                 =0x%x \n", DISP_REG_GET(DISP_REG_COLOR_R0_CRC                 ));
         break;

      case DISP_MODULE_TDSHP:  
         break;

      case DISP_MODULE_BLS:      
         DISP_MSG("===== DISP BLS Reg Dump: ============\n");           
         DISP_MSG("(000)DISP_REG_BLS_EN                   =0x%x \n", DISP_REG_GET(DISP_REG_BLS_EN                  ));
         DISP_MSG("(004)DISP_REG_BLS_RST                    =0x%x \n", DISP_REG_GET(DISP_REG_BLS_RST                     ));
         DISP_MSG("(008)DISP_REG_BLS_INTEN                  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_INTEN                 ));
         DISP_MSG("(00C)DISP_REG_BLS_INTSTA                 =0x%x \n", DISP_REG_GET(DISP_REG_BLS_INTSTA                 ));
         DISP_MSG("(010)DISP_REG_BLS_SETTING                     =0x%x \n", DISP_REG_GET(DISP_REG_BLS_SETTING                     ));
         DISP_MSG("(014)DISP_REG_BLS_FANA_SETTING                   =0x%x \n", DISP_REG_GET(DISP_REG_BLS_FANA_SETTING                  ));
         DISP_MSG("(018)DISP_REG_BLS_SRC_SIZE                    =0x%x \n", DISP_REG_GET(DISP_REG_BLS_SRC_SIZE                     ));
         DISP_MSG("(020)DISP_REG_BLS_GAIN_SETTING                  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_GAIN_SETTING                 ));
         DISP_MSG("(024)DISP_REG_BLS_MANUAL_GAIN                    =0x%x \n", DISP_REG_GET(DISP_REG_BLS_MANUAL_GAIN                     ));
         DISP_MSG("(028)DISP_REG_BLS_MANUAL_MAXCLR                  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_MANUAL_MAXCLR                 ));
         DISP_MSG("(030)DISP_REG_BLS_GAMMA_SETTING                 =0x%x \n", DISP_REG_GET(DISP_REG_BLS_GAMMA_SETTING                 ));
         DISP_MSG("(034)DISP_REG_BLS_GAMMA_BOUND                   =0x%x \n", DISP_REG_GET(DISP_REG_BLS_GAMMA_BOUND                  ));
         DISP_MSG("(038)DISP_REG_BLS_LUT_UPDATE                    =0x%x \n", DISP_REG_GET(DISP_REG_BLS_LUT_UPDATE                     ));
         DISP_MSG("(060)DISP_REG_BLS_MAXCLR_THD                  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_MAXCLR_THD                 ));
         DISP_MSG("(064)DISP_REG_BLS_DISTPT_THD                 =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DISTPT_THD                 ));
         DISP_MSG("(068)DISP_REG_BLS_MAXCLR_LIMIT                     =0x%x \n", DISP_REG_GET(DISP_REG_BLS_MAXCLR_LIMIT                     ));
         DISP_MSG("(06C)DISP_REG_BLS_DISTPT_LIMIT                   =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DISTPT_LIMIT                  ));
         DISP_MSG("(070)DISP_REG_BLS_AVE_SETTING                    =0x%x \n", DISP_REG_GET(DISP_REG_BLS_AVE_SETTING                     ));
         DISP_MSG("(074)DISP_REG_BLS_AVE_LIMIT                  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_AVE_LIMIT                 ));
         DISP_MSG("(078)DISP_REG_BLS_DISTPT_SETTING                    =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DISTPT_SETTING                     ));
         DISP_MSG("(07C)DISP_REG_BLS_HIS_CLEAR                  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_HIS_CLEAR                 ));
         DISP_MSG("(080)DISP_REG_BLS_SC_DIFF_THD                 =0x%x \n", DISP_REG_GET(DISP_REG_BLS_SC_DIFF_THD                 ));
         DISP_MSG("(084)DISP_REG_BLS_SC_BIN_THD                   =0x%x \n", DISP_REG_GET(DISP_REG_BLS_SC_BIN_THD                  ));
         DISP_MSG("(088)DISP_REG_BLS_MAXCLR_GRADUAL                    =0x%x \n", DISP_REG_GET(DISP_REG_BLS_MAXCLR_GRADUAL                     ));
         DISP_MSG("(08C)DISP_REG_BLS_DISTPT_GRADUAL                  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DISTPT_GRADUAL                 ));
         DISP_MSG("(090)DISP_REG_BLS_FAST_IIR_XCOEFF                 =0x%x \n", DISP_REG_GET(DISP_REG_BLS_FAST_IIR_XCOEFF                 ));
         DISP_MSG("(094)DISP_REG_BLS_FAST_IIR_YCOEFF                     =0x%x \n", DISP_REG_GET(DISP_REG_BLS_FAST_IIR_YCOEFF                     ));
         DISP_MSG("(098)DISP_REG_BLS_SLOW_IIR_XCOEFF                   =0x%x \n", DISP_REG_GET(DISP_REG_BLS_SLOW_IIR_XCOEFF                  ));
         DISP_MSG("(09C)DISP_REG_BLS_SLOW_IIR_YCOEFF                    =0x%x \n", DISP_REG_GET(DISP_REG_BLS_SLOW_IIR_YCOEFF                     ));
         DISP_MSG("(0A0)DISP_REG_BLS_PWM_DUTY                  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_PWM_DUTY                 ));
         DISP_MSG("(0A4)DISP_REG_BLS_PWM_GRADUAL                    =0x%x \n", DISP_REG_GET(DISP_REG_BLS_PWM_GRADUAL                     ));
         DISP_MSG("(0A8)DISP_REG_BLS_PWM_CON                  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_PWM_CON                 ));
         DISP_MSG("(0AC)DISP_REG_BLS_PWM_MANUAL                 =0x%x \n", DISP_REG_GET(DISP_REG_BLS_PWM_MANUAL                 ));
         DISP_MSG("(0B0)DISP_REG_BLS_DEBUG                   =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DEBUG                  ));
         DISP_MSG("(0B4)DISP_REG_BLS_PATTERN                    =0x%x \n", DISP_REG_GET(DISP_REG_BLS_PATTERN                     ));
         DISP_MSG("(0B8)DISP_REG_BLS_CHKSUM                  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_CHKSUM                 ));
         DISP_MSG("(100)DISP_REG_BLS_HIS_BIN                 =0x%x \n", DISP_REG_GET(DISP_REG_BLS_HIS_BIN(0)                 ));
         DISP_MSG("(200)DISP_REG_BLS_PWM_DUTY_RD                     =0x%x \n", DISP_REG_GET(DISP_REG_BLS_PWM_DUTY_RD                     ));
         DISP_MSG("(204)DISP_REG_BLS_FRAME_AVE_RD                   =0x%x \n", DISP_REG_GET(DISP_REG_BLS_FRAME_AVE_RD                  ));
         DISP_MSG("(208)DISP_REG_BLS_MAXCLR_RD                    =0x%x \n", DISP_REG_GET(DISP_REG_BLS_MAXCLR_RD                     ));
         DISP_MSG("(20C)DISP_REG_BLS_DISTPT_RD                  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DISTPT_RD                 ));
         DISP_MSG("(210)DISP_REG_BLS_GAIN_RD                    =0x%x \n", DISP_REG_GET(DISP_REG_BLS_GAIN_RD                     ));
         DISP_MSG("(214)DISP_REG_BLS_SC_RD                  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_SC_RD                 ));
         DISP_MSG("(300)DISP_REG_BLS_LUMINANCE                 =0x%x \n", DISP_REG_GET(DISP_REG_BLS_LUMINANCE(0)                 ));
         DISP_MSG("(384)DISP_REG_BLS_LUMINANCE_255                   =0x%x \n", DISP_REG_GET(DISP_REG_BLS_LUMINANCE_255                  ));
         DISP_MSG("(400)DISP_REG_BLS_GAMMA_LUT                    =0x%x \n", DISP_REG_GET(DISP_REG_BLS_GAMMA_LUT(0)                     ));
         DISP_MSG("(E00)DISP_REG_BLS_DITHER_0                  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DITHER_0                 ));
         DISP_MSG("(E14)DISP_REG_BLS_DITHER_5                 =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DITHER_5                 ));
         DISP_MSG("(E18)DISP_REG_BLS_DITHER_6                     =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DITHER_6                     ));
         DISP_MSG("(E1C)DISP_REG_BLS_DITHER_7                   =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DITHER_7                  ));
         DISP_MSG("(E20)DISP_REG_BLS_DITHER_8                    =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DITHER_8                     ));
         DISP_MSG("(E24)DISP_REG_BLS_DITHER_9                  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DITHER_9                 ));
         DISP_MSG("(E28)DISP_REG_BLS_DITHER_10                    =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DITHER_10                     ));
         DISP_MSG("(E2C)DISP_REG_BLS_DITHER_11                  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DITHER_11                 ));
         DISP_MSG("(E30)DISP_REG_BLS_DITHER_12                 =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DITHER_12                 ));
         DISP_MSG("(E34)DISP_REG_BLS_DITHER_13                   =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DITHER_13                  ));
         DISP_MSG("(E38)DISP_REG_BLS_DITHER_14                    =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DITHER_14                     ));
         DISP_MSG("(E3C)DISP_REG_BLS_DITHER_15                  =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DITHER_15                 ));
         DISP_MSG("(E40)DISP_REG_BLS_DITHER_16                 =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DITHER_16                 ));
         DISP_MSG("(E44)DISP_REG_BLS_DITHER_17                     =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DITHER_17                     ));
         DISP_MSG("(F00)DISP_REG_BLS_DUMMY                   =0x%x \n", DISP_REG_GET(DISP_REG_BLS_DUMMY                  ));
         break;
      
      case DISP_MODULE_WDMA0:  
         index = 0;
         DISP_MSG("===== DISP WDMA%d Reg Dump: ============\n", index);      
         DISP_MSG("(000)WDMA_INTEN         =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_INTEN        +0x1000*index) );
         DISP_MSG("(004)WDMA_INTSTA        =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_INTSTA       +0x1000*index) );
         DISP_MSG("(008)WDMA_EN            =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_EN           +0x1000*index) );
         DISP_MSG("(00C)WDMA_RST           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_RST          +0x1000*index) );
         DISP_MSG("(010)WDMA_SMI_CON       =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_SMI_CON      +0x1000*index) );
         DISP_MSG("(014)WDMA_CFG           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_CFG          +0x1000*index) );
         DISP_MSG("(018)WDMA_SRC_SIZE      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_SRC_SIZE     +0x1000*index) );
         DISP_MSG("(01C)WDMA_CLIP_SIZE     =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_CLIP_SIZE    +0x1000*index) );
         DISP_MSG("(020)WDMA_CLIP_COORD    =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_CLIP_COORD   +0x1000*index) );
         DISP_MSG("(024)WDMA_DST_ADDR      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DST_ADDR     +0x1000*index) );
         DISP_MSG("(028)WDMA_DST_W_IN_BYTE =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DST_W_IN_BYTE+0x1000*index) );
         DISP_MSG("(02C)WDMA_ALPHA         =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_ALPHA        +0x1000*index) );
         DISP_MSG("(030)WDMA_BUF_ADDR      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_BUF_ADDR     +0x1000*index) );
         DISP_MSG("(034)WDMA_STA           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_STA          +0x1000*index) );
         DISP_MSG("(038)WDMA_BUF_CON1      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_BUF_CON1     +0x1000*index) );
         DISP_MSG("(03C)WDMA_BUF_CON2      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_BUF_CON2     +0x1000*index) );
         DISP_MSG("(040)WDMA_C00           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C00          +0x1000*index) );
         DISP_MSG("(044)WDMA_C02           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C02          +0x1000*index) );
         DISP_MSG("(048)WDMA_C10           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C10          +0x1000*index) );
         DISP_MSG("(04C)WDMA_C12           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C12          +0x1000*index) );
         DISP_MSG("(050)WDMA_C20           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C20          +0x1000*index) );
         DISP_MSG("(054)WDMA_C22           =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_C22          +0x1000*index) );
         DISP_MSG("(058)WDMA_PRE_ADD0      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_PRE_ADD0     +0x1000*index) );
         DISP_MSG("(05C)WDMA_PRE_ADD2      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_PRE_ADD2     +0x1000*index) );
         DISP_MSG("(060)WDMA_POST_ADD0     =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_POST_ADD0    +0x1000*index) );
         DISP_MSG("(064)WDMA_POST_ADD2     =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_POST_ADD2    +0x1000*index) );
         DISP_MSG("(070)WDMA_DST_U_ADDR    =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DST_U_ADDR   +0x1000*index) );
         DISP_MSG("(074)WDMA_DST_V_ADDR    =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DST_V_ADDR   +0x1000*index) );
         DISP_MSG("(078)WDMA_DST_UV_PITCH  =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DST_UV_PITCH +0x1000*index) );
         DISP_MSG("(090)WDMA_DITHER_CON    =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_DITHER_CON   +0x1000*index) );
         DISP_MSG("(0A0)WDMA_FLOW_CTRL_DBG =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_FLOW_CTRL_DBG+0x1000*index) );
         DISP_MSG("(0A4)WDMA_EXEC_DBG      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_EXEC_DBG     +0x1000*index) );
         DISP_MSG("(0A8)WDMA_CLIP_DBG      =0x%x \n", DISP_REG_GET(DISP_REG_WDMA_CLIP_DBG     +0x1000*index) );
         break;      
      
      case DISP_MODULE_RDMA0:  
         index = 0;
         DISP_MSG("===== DISP RDMA%d Reg Dump: ======== \n", index);
         DISP_MSG("(000)RDMA_INT_ENABLE        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_INT_ENABLE       +0x1000*index));
         DISP_MSG("(004)RDMA_INT_STATUS        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_INT_STATUS       +0x1000*index));
         DISP_MSG("(010)RDMA_GLOBAL_CON        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_GLOBAL_CON       +0x1000*index));
         DISP_MSG("(014)RDMA_SIZE_CON_0        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_0       +0x1000*index));
         DISP_MSG("(018)RDMA_SIZE_CON_1        =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_SIZE_CON_1       +0x1000*index));
         DISP_MSG("(024)RDMA_MEM_CON           =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_CON          +0x1000*index));
         DISP_MSG("(028)RDMA_MEM_START_ADDR    =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_START_ADDR   +0x1000*index));
         DISP_MSG("(02C)RDMA_MEM_SRC_PITCH     =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_SRC_PITCH    +0x1000*index));
         DISP_MSG("(030)RDMA_MEM_GMC_SETTING_0 =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_0+0x1000*index));
         DISP_MSG("(034)RDMA_MEM_SLOW_CON      =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_SLOW_CON     +0x1000*index));
         DISP_MSG("(030)RDMA_MEM_GMC_SETTING_1 =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_MEM_GMC_SETTING_1+0x1000*index));
         DISP_MSG("(040)RDMA_FIFO_CON          =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_FIFO_CON         +0x1000*index));
         DISP_MSG("(054)RDMA_CF_00             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_00            +0x1000*index));
         DISP_MSG("(058)RDMA_CF_01             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_01            +0x1000*index));
         DISP_MSG("(05C)RDMA_CF_02             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_02            +0x1000*index));
         DISP_MSG("(060)RDMA_CF_10             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_10            +0x1000*index));
         DISP_MSG("(064)RDMA_CF_11             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_11            +0x1000*index));
         DISP_MSG("(068)RDMA_CF_12             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_12            +0x1000*index));
         DISP_MSG("(06C)RDMA_CF_20             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_20            +0x1000*index));
         DISP_MSG("(070)RDMA_CF_21             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_21            +0x1000*index));
         DISP_MSG("(074)RDMA_CF_22             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_22            +0x1000*index));
         DISP_MSG("(078)RDMA_CF_PRE_ADD0       =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_PRE_ADD0      +0x1000*index));
         DISP_MSG("(07C)RDMA_CF_PRE_ADD1       =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_PRE_ADD1      +0x1000*index));
         DISP_MSG("(080)RDMA_CF_PRE_ADD2       =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_PRE_ADD2      +0x1000*index));
         DISP_MSG("(084)RDMA_CF_POST_ADD0      =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_POST_ADD0     +0x1000*index));
         DISP_MSG("(088)RDMA_CF_POST_ADD1      =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_POST_ADD1     +0x1000*index));
         DISP_MSG("(08C)RDMA_CF_POST_ADD2      =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_CF_POST_ADD2     +0x1000*index));      
         DISP_MSG("(090)RDMA_DUMMY             =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_DUMMY            +0x1000*index));
         DISP_MSG("(094)RDMA_DEBUG_OUT_SEL     =0x%x \n", DISP_REG_GET(DISP_REG_RDMA_DEBUG_OUT_SEL    +0x1000*index));
         break;
      
      case DISP_MODULE_CMDQ:
         DISP_MSG("===== DISP CMDQ Reg Dump: ============\n");
         DISP_MSG("(0x10)CMDQ_IRQ_FLAG          =0x%x \n", DISP_REG_GET(DISP_REG_CMDQ_IRQ_FLAG        )); 
         DISP_MSG("(0x20)CMDQ_LOADED_THR        =0x%x \n", DISP_REG_GET(DISP_REG_CMDQ_LOADED_THR      )); 
         DISP_MSG("(0x30)CMDQ_THR_SLOT_CYCLES   =0x%x \n", DISP_REG_GET(DISP_REG_CMDQ_THR_SLOT_CYCLES )); 
         DISP_MSG("(0x40)CMDQ_BUS_CTRL          =0x%x \n", DISP_REG_GET(DISP_REG_CMDQ_BUS_CTRL        ));                                               
         DISP_MSG("(0x50)CMDQ_ABORT             =0x%x \n", DISP_REG_GET(DISP_REG_CMDQ_ABORT           )); 
   
         for(index=0;index<CMDQ_THREAD_NUM;index++)
         {
            DISP_MSG("(0x%x)CMDQ_THRx_RESET%d                =0x%x \n", (0x100 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_RESET(index)               )); 
            DISP_MSG("(0x%x)CMDQ_THRx_EN%d                   =0x%x \n", (0x104 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_EN(index)                  )); 
            DISP_MSG("(0x%x)CMDQ_THRx_SUSPEND%d              =0x%x \n", (0x108 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_SUSPEND(index)             )); 
            DISP_MSG("(0x%x)CMDQ_THRx_STATUS%d               =0x%x \n", (0x10c + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_STATUS(index)              )); 
            DISP_MSG("(0x%x)CMDQ_THRx_IRQ_FLAG%d             =0x%x \n", (0x110 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_IRQ_FLAG(index)            )); 
            DISP_MSG("(0x%x)CMDQ_THRx_IRQ_FLAG_EN%d          =0x%x \n", (0x114 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_IRQ_FLAG_EN(index)         )); 
            DISP_MSG("(0x%x)CMDQ_THRx_SECURITY%d             =0x%x \n", (0x118 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_SECURITY(index)            )); 
            DISP_MSG("(0x%x)CMDQ_THRx_PC%d                   =0x%x \n", (0x120 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(index)                  )); 
            DISP_MSG("(0x%x)CMDQ_THRx_END_ADDR%d             =0x%x \n", (0x124 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_END_ADDR(index)            )); 
            DISP_MSG("(0x%x)CMDQ_THRx_EXEC_CMDS_CNT%d        =0x%x \n", (0x128 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_EXEC_CMDS_CNT(index)       )); 
            DISP_MSG("(0x%x)CMDQ_THRx_WAIT_EVENTS0%d         =0x%x \n", (0x130 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_WAIT_EVENTS0(index)        )); 
            DISP_MSG("(0x%x)CMDQ_THRx_WAIT_EVENTS1%d         =0x%x \n", (0x134 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_WAIT_EVENTS1(index)        )); 
            DISP_MSG("(0x%x)CMDQ_THRx_OBSERVED_EVENTS0%d     =0x%x \n", (0x140 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_OBSERVED_EVENTS0(index)    )); 
            DISP_MSG("(0x%x)CMDQ_THRx_OBSERVED_EVENTS1%d     =0x%x \n", (0x144 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_OBSERVED_EVENTS1(index)    )); 
            DISP_MSG("(0x%x)CMDQ_THRx_OBSERVED_EVENTS0_CLR%d =0x%x \n", (0x148 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_OBSERVED_EVENTS0_CLR(index))); 
            DISP_MSG("(0x%x)CMDQ_THRx_OBSERVED_EVENTS1_CLR%d =0x%x \n", (0x14c + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_OBSERVED_EVENTS1_CLR(index))); 
            DISP_MSG("(0x%x)CMDQ_THRx_INSTN_TIMEOUT_CYCLES%d =0x%x \n", (0x150 + 0x80*index), index, DISP_REG_GET(DISP_REG_CMDQ_THRx_INSTN_TIMEOUT_CYCLES(index))); 
         }
         break;
      
      case DISP_MODULE_GAMMA:  
      case DISP_MODULE_DBI:      
      case DISP_MODULE_DSI:
      case DISP_MODULE_DSI_VDO:
      case DISP_MODULE_DSI_CMD:
      case DISP_MODULE_DPI1:  
         break;
   
      case DISP_MODULE_DPI0:   
         DISP_MSG("===== DISP DPI0 Reg Dump: ============\n");
         DISP_MSG("(0x00)DPI0_EN_REG               = 0x%x \n", DISP_REG_GET(DISP_REG_DPI_EN)               ); 
         DISP_MSG("(0x04)DPI0_RST_REG              = 0x%x \n", DISP_REG_GET(DISP_REG_DPI_RST)              );
         DISP_MSG("(0x08)DPI0_INTEN_REG            = 0x%x \n", DISP_REG_GET(DISP_REG_DPI_INTEN)            ); 
         DISP_MSG("(0x0C)DPI0_INTSTA_REG           = 0x%x \n", DISP_REG_GET(DISP_REG_DPI_INTSTA)           );    
         DISP_MSG("(0x10)DPI0_CON_REG              = 0x%x \n", DISP_REG_GET(DISP_REG_DPI_CON)              ); 
         DISP_MSG("(0x14)DPI0_OUTPUT_SETTING_REG           = 0x%x \n", DISP_REG_GET(DISP_REG_DPI_OUTPUT_SETTING)           ); 
         DISP_MSG("(0x18)DPI0_SIZE_REG             = 0x%x \n", DISP_REG_GET(DISP_REG_DPI_SIZE)             ); 
         DISP_MSG("(0x1C)DPI0_TGEN_HWIDTH_REG      = 0x%x \n", DISP_REG_GET(DISP_REG_DPI_TGEN_HWIDTH)      ); 
         DISP_MSG("(0x20)DPI0_TGEN_HPORCH_REG      = 0x%x \n", DISP_REG_GET(DISP_REG_DPI_TGEN_HPROCH)      );    
         DISP_MSG("(0x24)DPI0_TGEN_VWIDTH_REG = 0x%x \n", DISP_REG_GET(DISP_REG_DPI_TGEN_VWIDTH) );    
         DISP_MSG("(0x28)DPI0_TGEN_VPORCH_REG = 0x%x \n", DISP_REG_GET(DISP_REG_DPI_TGEN_VPROCH) );    
         DISP_MSG("(0x50)DPI0_BG_HCNTL_REG         = 0x%x \n", DISP_REG_GET(DISP_REG_DPI_BG_HCNTL)         ); 
         DISP_MSG("(0x54)DPI0_BG_VCNTL_REG         = 0x%x \n", DISP_REG_GET(DISP_REG_DPI_BG_VCNTL)         );    
         DISP_MSG("(0x60)DPI0_BG_COLOR_REG           = 0x%x \n", DISP_REG_GET(DISP_REG_DPI_BG_COLOR)           );
         DISP_MSG("(0x60)DPI0_BG_FIFO_CTL_REG           = 0x%x \n", DISP_REG_GET(DISP_REG_DPI_FIFO_CTL)           );
         DISP_MSG("(0x60)DPI0_STATUS_REG           = 0x%x \n", DISP_REG_GET(DISP_REG_DPI_STATUS)           );
         DISP_MSG("(0x64)DPI0_TMODE_REG= 0x%x \n", DISP_REG_GET(DISP_REG_DPI_TMODE)); 
         DISP_MSG("(0x68)DPI0_CHKSUM_REG= 0x%x \n", DISP_REG_GET(DISP_REG_DPI_CHKSUM)); 
         DISP_MSG("(0x6C)DPI0_PATTERN_REG= 0x%x \n", DISP_REG_GET(DISP_REG_DPI_PATTERN)); 
         break;
      
      case DISP_MODULE_MUTEX:
         DISP_MSG("===== DISP DISP_REG_MUTEX_CONFIG Reg Dump: ============\n");
         DISP_MSG("(0x0  )DISP_MUTEX_INTEN        =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTEN         ));
         DISP_MSG("(0x4  )CONFIG_MUTEX_INTSTA       =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX_INTSTA        ));
         DISP_MSG("(0x8  )CONFIG_REG_UPD_TIMEOUT    =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_REG_UPD_TIMEOUT     ));
         DISP_MSG("(0xC  )CONFIG_REG_COMMIT         =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_REG_COMMIT          ));
         DISP_MSG("(0x20)CONFIG_MUTEX0_EN           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_EN           ));
         DISP_MSG("(0x24)CONFIG_MUTEX0              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0              ));
         DISP_MSG("(0x28)CONFIG_MUTEX0_RST          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_RST          ));
         DISP_MSG("(0x2C)CONFIG_MUTEX0_MOD          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_MOD          ));
         DISP_MSG("(0x30)CONFIG_MUTEX0_SOF          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX0_SOF          ));
         DISP_MSG("(0x40)CONFIG_MUTEX1_EN           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_EN           ));
         DISP_MSG("(0x44)CONFIG_MUTEX1              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1              ));
         DISP_MSG("(0x48)CONFIG_MUTEX1_RST          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_RST          ));
         DISP_MSG("(0x4C)CONFIG_MUTEX1_MOD          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_MOD          ));
         DISP_MSG("(0x50)CONFIG_MUTEX1_SOF          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX1_SOF          ));
         DISP_MSG("(0x60)CONFIG_MUTEX2_EN           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_EN           ));
         DISP_MSG("(0x64)CONFIG_MUTEX2              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2              ));
         DISP_MSG("(0x68)CONFIG_MUTEX2_RST          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_RST          ));
         DISP_MSG("(0x6C)CONFIG_MUTEX2_MOD          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_MOD          ));
         DISP_MSG("(0x70)CONFIG_MUTEX2_SOF          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX2_SOF          ));
         DISP_MSG("(0x80)CONFIG_MUTEX3_EN           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_EN           ));
         DISP_MSG("(0x84)CONFIG_MUTEX3              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3              ));
         DISP_MSG("(0x88)CONFIG_MUTEX3_RST          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_RST          ));
         DISP_MSG("(0x8C)CONFIG_MUTEX3_MOD          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_MOD          ));
         DISP_MSG("(0x90)CONFIG_MUTEX3_SOF          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX3_SOF          ));
         DISP_MSG("(0xA0)CONFIG_MUTEX4_EN           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_EN           ));
         DISP_MSG("(0xA4)CONFIG_MUTEX4              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4              ));
         DISP_MSG("(0xA8)CONFIG_MUTEX4_RST          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_RST          ));
         DISP_MSG("(0xAC)CONFIG_MUTEX4_MOD          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_MOD          ));
         DISP_MSG("(0xB0)CONFIG_MUTEX4_SOF          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX4_SOF          ));
         DISP_MSG("(0xC0)CONFIG_MUTEX5_EN           =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_EN           ));
         DISP_MSG("(0xC4)CONFIG_MUTEX5              =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5              ));
         DISP_MSG("(0xC8)CONFIG_MUTEX5_RST          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_RST          ));
         DISP_MSG("(0xCC)CONFIG_MUTEX5_MOD          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_MOD          ));
         DISP_MSG("(0xD0)CONFIG_MUTEX5_SOF          =0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX5_SOF          ));
         DISP_MSG("(0x100)CONFIG_MUTEX_DEBUG_OUT_SEL=0x%x \n", DISP_REG_GET(DISP_REG_CONFIG_MUTEX_DEBUG_OUT_SEL ));
      break;
     
      default:
         DISP_MSG("disp_dump_reg() invalid module id=%d \n", module);
   }
   
   return 0;
}

