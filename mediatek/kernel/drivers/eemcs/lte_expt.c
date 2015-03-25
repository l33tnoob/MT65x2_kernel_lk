#include <linux/list.h>

#include "lte_df_main.h"
#include "lte_hif_sdio.h"
#include "eemcs_kal.h"


static struct mtlte_expt_priv lte_expt_priv ;


int mtlte_expt_register_callback(EEMCS_CCCI_EX_IND cb)
{
    lte_expt_priv.cb_ccci_expt_int = cb;
    return KAL_SUCCESS;
}


void mtlte_expt_q_num_init(KAL_UINT32 dhldl_q, KAL_UINT32 except_q)
{
    int i;

    for(i=0; i<RXQ_NUM; i++)
    {
        if( dhldl_q & (0x1 << i) )
        {
            lte_expt_priv.non_stop_dlq[i] = 1;
        }

        if(i >= 16) break;
    }


    for(i=0; i<RXQ_NUM; i++)
    {
        if( except_q & (0x1 << i) )
        {
            lte_expt_priv.except_mode_dlq[i] = 1;
        }

        if(i >= 16) break;
    }


    for(i=0; i<TXQ_NUM; i++)
    {
        if( except_q & (0x1 << (i+16)) )
        {
            lte_expt_priv.except_mode_ulq[i] = 1;
        }
    }
    
}


int mtlte_check_excetion_int(unsigned int swint_status)
{
    int i;
    
    if(swint_status & D2H_INT_except_init)
    {
        KAL_DBGPRINT(KAL, DBG_ERROR,("[exception] MT6290m modem assertion!!  Start assertion dump flow... \r\n")) ;
        
        if(lte_expt_priv.cb_ccci_expt_int){
            lte_expt_priv.cb_ccci_expt_int(EX_INIT);
        }else{
            KAL_DBGPRINT(KAL, DBG_ERROR,("[exception] there is no ccci callback function !!\r\n")) ;
            KAL_ASSERT(0) ;
        }

        if (mtlte_hif_expt_mode_init() != KAL_SUCCESS){
			return KAL_FAIL ; 
		}	

        // eemcs_ccci_ex_ind (DHL_DL_RDY)
        lte_expt_priv.cb_ccci_expt_int(EX_DHL_DL_RDY);
        
        // [SDIO] Active DHL_DL queue & Enable DHL_DL related DLQ interrupt
        for(i=0; i<RXQ_NUM; i++)
        {
            if( 1 == lte_expt_priv.non_stop_dlq[i] )
            {
                mtlte_hif_expt_restart_que(1, i);   
            }
        }

        mtlte_hif_expt_unmask_swint();
        mtlte_hif_expt_enable_interrupt();

        KAL_DBGPRINT(KAL, DBG_ERROR,("[exception] Start to transfer remain DHL DL pkt... \r\n")) ;
        
    }
    else if(swint_status & D2H_INT_except_clearQ_done)
    {
        KAL_DBGPRINT(KAL, DBG_ERROR,("[exception] DHL DL pkt transfer Done, Start reset to exception que ... \r\n")) ;
        mtlte_hif_expt_set_reset_allQ_bit();
    }

    return KAL_SUCCESS;
}



int mtlte_expt_reset_inform_hif(void)
{
    
    if(lte_expt_priv.cb_ccci_expt_int){
            lte_expt_priv.cb_ccci_expt_int(EX_INIT_DONE);
    }else{
            KAL_DBGPRINT(KAL, DBG_ERROR,("[exception] there is no ccci callback function !!\r\n")) ;
            KAL_ASSERT(0) ;
            return KAL_FAIL;
    }

    return KAL_SUCCESS;
}

int mtlte_expt_check_expt_q_num(KAL_UINT32 is_DL, KAL_UINT32 q_num)
{
    if(is_DL){
        return lte_expt_priv.except_mode_dlq[q_num];
    }else{
        return lte_expt_priv.except_mode_ulq[q_num]; 
    }
}


int mtlte_expt_init(void)
{
    int i;
    
    lte_expt_priv.cb_ccci_expt_int = NULL;

    for(i=0; i<TXQ_NUM; i++)
    {
        lte_expt_priv.except_mode_ulq[i] = 0;
    }
    
    for(i=0; i<RXQ_NUM; i++)
    {
        lte_expt_priv.except_mode_dlq[i] = 0;
        lte_expt_priv.non_stop_dlq[i] = 0;
    }
    
    return KAL_SUCCESS;
}


int mtlte_expt_probe(void)
{
    return KAL_SUCCESS;
}

int mtlte_expt_remove(void)
{
    return KAL_SUCCESS;
}

int mtlte_expt_deinit(void)
{
    int i;
    
    lte_expt_priv.cb_ccci_expt_int = NULL;

    for(i=0; i<TXQ_NUM; i++)
    {
        lte_expt_priv.except_mode_ulq[i] = 0;
    }
    
    for(i=0; i<RXQ_NUM; i++)
    {
        lte_expt_priv.except_mode_dlq[i] = 0;
        lte_expt_priv.non_stop_dlq[i] = 0;
    }

    return KAL_SUCCESS;
}


