#include "ddp_reg.h"
#include "ddp_cmdq.h"
#include "ddp_cmdq_debug.h"

uint32_t* cmdq_core_get_pc(struct TaskStruct *pTask, uint32_t thread, uint32_t insts[4])
{
    int32_t      currPC = 0;
    uint8_t      *pInst = NULL;

    insts[0] = 0;
    insts[1] = 0;
    insts[2] = 0;
    insts[3] = 0;

    currPC = DISP_REG_GET(DISP_REG_CMDQ_THRx_PC(thread));
    pInst  = (uint8_t*)pTask->pVABase + (currPC - pTask->MVABase);

    if(((uint8_t*)pTask->pVABase <= pInst) && (pInst <= (uint8_t*)pTask->pCMDEnd))
    {
        if (pInst != (uint8_t*)pTask->pCMDEnd)
        {
            // If PC points to start of pCMD,
            // - 8 causes access violation
            insts[2] = DISP_REG_GET(pInst + 0);
            insts[3] = DISP_REG_GET(pInst + 4);
        }
        else
        {            
            insts[2] = DISP_REG_GET(pInst - 8);
            insts[3] = DISP_REG_GET(pInst - 4);
        }
    }
    else
    {
        // invalid PC address
        return NULL;
    }

    return (uint32_t*)pInst;
}

const char* cmdq_core_get_event_name(uint32_t event)
{
    switch(event)
    {
    case 0: return "MUTEX0";
    case 1: return "MUTEX1";
    case 2: return "MUTEX2";
    case 3: return "MUTEX3";
    case 4: return "MUTEX4";
    case 5: return "MUTEX5";
    case 6: return "MUTEX6";
    case 7: return "MUTEX7";
    case 8 ... 11: return "Reserved";
    
    case 12: return "EOF_DISP_WDMA";
    case 13: return "EOF_DISP_RDMA";
    case 14: return "EOF_DISP_BLS";
    case 15: return "EOF_DISP_COLOR";
    case 16: return "EOF_DISP_OVL";
    case 17: return "EOF_MDP_TDSHP";
    case 18: return "EOF_MDP_RSZ1";
    case 19: return "EOF_MDP_RSZ0";
    case 20: return "EOF_MDP_RDMA";
    case 21: return "EOF_MDP_WDMA";
    case 22: return "EOF_MDP_WROT";
    case 23: return "EOF_ISP1";
    case 24: return "EOF_ISP2";
    case 25: return "SOF_MDP_WROT";
    case 26: return "SOF_MDP_RSZ0";
    case 27: return "SOF_MDP_RSZ1";
    case 28: return "SOF_DISP_OVL";
    case 29: return "SOF_MDP_WDMA";
    case 30: return "SOF_MDP_RDMA";
    case 31: return "SOF_DISP_WDMA";
    case 32: return "SOF_DISP_COLOR";
    case 33: return "SOF_MDP_TDSHP";
    case 34: return "SOF_DISP_BLS";
    case 35: return "SOF_DISP_RDMA";
    case 36: return "SOF_CAM_MDP";
    default: return "UNKOWN";
    }

    return "UNKOWN";
}


const char* cmdq_core_get_module_from_event_id(uint32_t event)
{
    const char *module = "UNKNOWN";

    switch(event)
    {
    case 0 ... 7:   // mutex 
    case 8 ... 11:  // reserved
        break; 
    case 12:        // "EOF_DISP_WDMA";
    case 13:        // "EOF_DISP_RDMA";
    case 14:        // "EOF_DISP_BLS";
    case 15:        // "EOF_DISP_COLOR";
    case 16:        // "EOF_DISP_OVL";        
        module = "DISP"; 
        break; 
    case 17:        // "EOF_MDP_TDSHP";
    case 18:        // "EOF_MDP_RSZ1";
    case 19:        // "EOF_MDP_RSZ0";
    case 20:        // "EOF_MDP_RDMA";
    case 21:        // "EOF_MDP_WDMA";
    case 22:        // "EOF_MDP_WROT";
        module = "MDP"; 
        break; 
    case 23:        // "EOF_ISP1";
    case 24:        // "EOF_ISP2";
        module = "ISP";
        break; 
    case 25:        // "SOF_MDP_WROT";
    case 26:        // "SOF_MDP_RSZ0";
    case 27:        // "SOF_MDP_RSZ1";
    case 29:        // "SOF_MDP_WDMA";
    case 30:        // "SOF_MDP_RDMA";
    case 33:        //"SOF_MDP_TDSHP"; 
        module = "MDP"; 
        break; 
    case 28:        // "SOF_DISP_OVL";
    case 31:        // "SOF_DISP_WDMA";
    case 32:        // "SOF_DISP_COLOR";
    case 34:        //"SOF_DISP_BLS";
    case 35:        // "SOF_DISP_RDMA";
        module = "DISP"; 
        break; 
    case 36:        // "SOF_CAM_MDP";
        module = "MDP";
        break; 
    default: 
        break;
    }

    return module;
}


const char* cmdq_core_parse_subsys(uint32_t argA, uint32_t argB)
{
    int32_t subsys_id = (argA & 0x00c00000) >> 22;
    const char *module = "CMDQ";
    
    switch (subsys_id)
    {
    case 0:
        module = "MM";
        break;
    case 1:
        module = "IMG";
        break;
    default:
        module = "UNKNOWN";
        break;
    }
    
    return module;
}


const char* cmdq_core_parse_op(uint32_t opCode)
{
    switch(opCode)
    {
    case CMDQ_CODE_POLL:
        return "POLL";
    case CMDQ_CODE_WRITE:
        return "WRITE";
    case CMDQ_CODE_WFE:
        return "WFE(SYNC)";
    case CMDQ_CODE_READ:
        return "READ";
    case CMDQ_CODE_MOVE:
        return "MOVE";
    case CMDQ_CODE_JUMP:
        return "JUMP";
    case CMDQ_CODE_EOC:
        return "EOC(MARKER)";
    }
    return NULL;
}


void cmdq_core_parse_error(struct TaskStruct *pTask, uint32_t thread, 
                           const char **moduleName, uint32_t *instA, uint32_t *instB)
{
    uint32_t     op, argA, argB;
    uint32_t     insts[4] = {0};

    const char   *module = NULL;
    
    if(cmdq_core_get_pc(pTask, thread, insts))
    {
        op = (insts[3] & 0xFF000000) >> 24;
        argA = insts[3] & (~0xFF000000);
        argB = insts[2];

        switch(op)
        {
        case CMDQ_CODE_POLL:
        case CMDQ_CODE_WRITE:
            module = cmdq_core_parse_subsys(argA, argB);
            break;
        case CMDQ_CODE_WFE:
            // argA is the event ID
            module = cmdq_core_get_module_from_event_id(argA); 
            break;
        case CMDQ_CODE_READ:
        case CMDQ_CODE_MOVE:
        case CMDQ_CODE_JUMP:
        case CMDQ_CODE_EOC:
        default:
            module = "CMDQ";
            break;
        }        
    }
    else
    {
        module = "CMDQ";
    }

    // fill output parameter
    *moduleName = module;
    *instA = insts[3];
    *instB = insts[2];

}


uint32_t* cmdq_core_dump_pc(TaskStruct *pTask, int thread, const char *tag)
{
    uint32_t *hwPC = NULL;
    uint32_t  insts[4] = {0};
    uint32_t  value = 0;
    
    hwPC = cmdq_core_get_pc(pTask, thread, insts); 
    if(hwPC)
    {
        uint32_t op, argA, argB;
        op = (insts[3] & 0xFF000000) >> 24;
        argA = insts[3] & (~0xFF000000);
        argB = insts[2];
    
        switch(op)
        {
        case CMDQ_CODE_WFE:
            // dump SYNC TOKEN value by writing to ID
            // then read value from VAL
            DISP_REG_SET(DISP_REG_CMDQ_SYNC_TOKEN_ID, 0x3FF & argA);
            value = DISP_REG_GET(DISP_REG_CMDQ_SYNC_TOKEN_VALUE);
            printk(KERN_DEBUG "[CMDQ][%s] Curr PC(VA): 0x%p, Inst: 0x%08x:0x%08x, WAIT for %s, token_value=0x%08x\n", 
                     tag, 
                     hwPC, 
                     insts[2], 
                     insts[3], 
                     cmdq_core_get_event_name(argA),
                     value);
            break;
        case CMDQ_CODE_POLL:
            printk(KERN_DEBUG "[CMDQ][%s] Curr PC(VA): 0x%p, Inst: 0x%08x:0x%08x, POLL for %s\n", 
                     tag, 
                     hwPC, 
                     insts[2], 
                     insts[3], 
                     cmdq_core_parse_subsys(argA, argB));
            break;
        default:
            printk(KERN_DEBUG "[CMDQ][%s] Curr PC(VA): 0x%p, Inst: 0x%08x:0x%08x, OP: %s\n", tag, hwPC, insts[2], insts[3], cmdq_core_parse_op(op));
            break;
        }

    }
    else
    {
         printk(KERN_DEBUG "[CMDQ][%s]Curr PC(VA): N/A, no valid address found\n", tag);
    }

    return hwPC;
}


typedef struct MODULE_BASE
{
    int engine;
    uint32_t base;
    const char *name;
} MODULE_BASE;

#define DEFINE_MODULE(eng, base) {eng, base, #eng}

void cmdq_core_dump_mdp_status(uint32_t engineFlag, int32_t logLevel)
{
    uint32_t     value[40] = {0};
    
    if (engineFlag & (0x01 << tRDMA0))
    {
        value[0] = DISP_REG_GET(0xF4001030);
        value[1] = DISP_REG_GET(0xF4001040);
        value[2] = DISP_REG_GET(0xF4001060);
        value[3] = DISP_REG_GET(0xF4001070);
        value[4] = DISP_REG_GET(0xF4001078);
        value[5] = DISP_REG_GET(0xF4001080);
        value[6] = DISP_REG_GET(0xF4001100);
        value[7] = DISP_REG_GET(0xF4001118);
        value[8] = DISP_REG_GET(0xF4001130);
        value[9] = DISP_REG_GET(0xF4001400);
        value[10] = DISP_REG_GET(0xF4001408);
        value[11] = DISP_REG_GET(0xF4001410);
        value[12] = DISP_REG_GET(0xF4001420);
        value[13] = DISP_REG_GET(0xF4001430);
        value[14] = DISP_REG_GET(0xF40014D0);

        CMDQ_ERR( "=============== [CMDQ] RDMA Status ====================================\n");
        CMDQ_ERR( "[CMDQ] RDMA_SRC_CON: 0x%08x, RDMA_SRC_BASE_0: 0x%08x, RDMA_MF_BKGD_SIZE_IN_BYTE: 0x%08x\n", 
            value[0], value[1], value[2]);
        CMDQ_ERR( "[CMDQ] RDMA_MF_SRC_SIZE: 0x%08x, RDMA_MF_CLIP_SIZE: 0x%08x, RDMA_MF_OFFSET_1: 0x%08x\n", 
            value[3], value[4], value[5]);
        CMDQ_ERR( "[CMDQ] RDMA_SRC_END_0: 0x%08x, RDMA_SRC_OFFSET_0: 0x%08x, RDMA_SRC_OFFSET_W_0: 0x%08x\n", 
            value[6], value[7], value[8]);
        CMDQ_ERR( "[CMDQ] RDMA_MON_STA_0: 0x%08x, RDMA_MON_STA_1: 0x%08x, RDMA_MON_STA_2: 0x%08x\n", 
            value[9], value[10], value[11]);
        CMDQ_ERR( "[CMDQ] RDMA_MON_STA_4: 0x%08x, RDMA_MON_STA_6: 0x%08x, RDMA_MON_STA_26: 0x%08x\n", 
            value[12], value[13], value[14]);
    }

    if (engineFlag & (0x01 << tSCL0))
    {
        value[0] = DISP_REG_GET(0xF4002004);
        value[1] = DISP_REG_GET(0xF400200C);
        value[2] = DISP_REG_GET(0xF4002010);
        value[3] = DISP_REG_GET(0xF4002014);
        value[4] = DISP_REG_GET(0xF4002018);
        DISP_REG_SET(0xF4002040, 0x00000001);
        value[5] = DISP_REG_GET(0xF4002044);
        DISP_REG_SET(0xF4002040, 0x00000002);
        value[6] = DISP_REG_GET(0xF4002044);
        DISP_REG_SET(0xF4002040, 0x00000003);
        value[7] = DISP_REG_GET(0xF4002044);

        CMDQ_ERR( "=============== [CMDQ] RSZ0 Status ====================================\n");
        CMDQ_ERR( "[CMDQ] RSZ0_CONTROL: 0x%08x, RSZ0_INPUT_IMAGE: 0x%08x RSZ0_OUTPUT_IMAGE: 0x%08x\n", 
            value[0], value[1], value[2]);
        CMDQ_ERR( "[CMDQ] RSZ0_HORIZONTAL_COEFF_STEP: 0x%08x, RSZ0_VERTICAL_COEFF_STEP: 0x%08x\n", 
            value[3], value[4]);
        CMDQ_ERR( "[CMDQ] RSZ0_DEBUG_1: 0x%08x, RSZ0_DEBUG_2: 0x%08x, RSZ0_DEBUG_3: 0x%08x\n", 
            value[5], value[6], value[7]);
    }

    if (engineFlag & (0x01 << tSCL1))
    {
        value[0] = DISP_REG_GET(0xF4003004);
        value[1] = DISP_REG_GET(0xF400300C);
        value[2] = DISP_REG_GET(0xF4003010);
        value[3] = DISP_REG_GET(0xF4003014);
        value[4] = DISP_REG_GET(0xF4003018);
        DISP_REG_SET(0xF4003040, 0x00000001);
        value[5] = DISP_REG_GET(0xF4003044);
        DISP_REG_SET(0xF4003040, 0x00000002);
        value[6] = DISP_REG_GET(0xF4003044);
        DISP_REG_SET(0xF4003040, 0x00000003);
        value[7] = DISP_REG_GET(0xF4003044);

        CMDQ_ERR( "=============== [CMDQ] RSZ1 Status ====================================\n");
        CMDQ_ERR( "[CMDQ] RSZ1_CONTROL: 0x%08x, RSZ1_INPUT_IMAGE: 0x%08x RSZ1_OUTPUT_IMAGE: 0x%08x\n", 
            value[0], value[1], value[2]);
        CMDQ_ERR( "[CMDQ] RSZ1_HORIZONTAL_COEFF_STEP: 0x%08x, RSZ1_VERTICAL_COEFF_STEP: 0x%08x\n", 
            value[3], value[4]);
        CMDQ_ERR( "[CMDQ] RSZ1_DEBUG_1: 0x%08x, RSZ1_DEBUG_2: 0x%08x, RSZ1_DEBUG_3: 0x%08x\n", 
            value[5], value[6], value[7]);
    }

    if (engineFlag & (0x01 << tTDSHP))
    {
        value[0] = DISP_REG_GET((0xF4006000 + 0x10C));
        value[1] = DISP_REG_GET((0xF4006000 + 0x114));
        value[2] = DISP_REG_GET((0xF4006000 + 0x11C));

        CMDQ_ERR( "=============== [CMDQ] tTDSHP Status ====================================\n");
        CMDQ_ERR( "[CMDQ] status: 0x%08x, INPUT_COUNT: 0x%08x, OUTPUT_COUNT: 0x%08x\n", value[0], value[1], value[2]);
    }

    if (engineFlag & (0x01 << tWROT))
    {
        value[0] = DISP_REG_GET(0xF4005000);
        value[1] = DISP_REG_GET(0xF4005008);
        value[2] = DISP_REG_GET(0xF400500C);
        value[3] = DISP_REG_GET(0xF4005024);
        value[4] = DISP_REG_GET(0xF4005028);
        value[5] = DISP_REG_GET(0xF400502C);
        value[6] = DISP_REG_GET(0xF4005004);
        value[7] = DISP_REG_GET(0xF4005030);
        value[8] = DISP_REG_GET(0xF4005078);
        value[9] = DISP_REG_GET(0xF4005070);
        DISP_REG_SET(0xF4005018, 0x00000100);
        value[10] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000200);
        value[11] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000300);
        value[12] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000400);
        value[13] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000500);
        value[14] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000600);
        value[15] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000700);
        value[16] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000800);
        value[17] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000900);
        value[18] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000A00);
        value[19] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000B00);
        value[20] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000C00);
        value[21] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000D00);
        value[22] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000E00);
        value[23] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00000F00);
        value[24] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00001000);
        value[25] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00001100);
        value[26] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00001200);
        value[27] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00001300);
        value[28] = DISP_REG_GET(0xF40050D0);
        DISP_REG_SET(0xF4005018, 0x00001400);
        value[29] = DISP_REG_GET(0xF40050D0);
        
        value[30] = DISP_REG_GET(0xF4005010);
        value[31] = DISP_REG_GET(0xF4005014);
        value[32] = DISP_REG_GET(0xF400501c);
        value[33] = DISP_REG_GET(0xF4005020);
        
        CMDQ_ERR( "=============== [CMDQ] ROT Status ====================================\n");
        CMDQ_ERR( "[CMDQ] ROT_CTRL: 0x%08x, ROT_MAIN_BUF_SIZE: 0x%08x, ROT_SUB_BUF_SIZE: 0x%08x\n", 
            value[0], value[1], value[2]);
        CMDQ_ERR( "[CMDQ] ROT_TAR_SIZE: 0x%08x, ROT_BASE_ADDR: 0x%08x, ROT_OFST_ADDR: 0x%08x\n", 
            value[3], value[4], value[5]);
        CMDQ_ERR( "[CMDQ] ROT_DMA_PERF: 0x%08x, ROT_STRIDE: 0x%08x, ROT_IN_SIZE: 0x%08x\n", 
            value[6], value[7], value[8]);
        CMDQ_ERR( "[CMDQ] ROT_EOL: 0x%08x, ROT_DBUGG_1: 0x%08x, ROT_DEBUBG_2: 0x%08x\n", 
            value[9], value[10], value[11]);
        CMDQ_ERR( "[CMDQ] ROT_DBUGG_3: 0x%08x, ROT_DBUGG_4: 0x%08x, ROT_DEBUBG_5: 0x%08x\n", 
            value[12], value[13], value[14]);
        CMDQ_ERR( "[CMDQ] ROT_DBUGG_6: 0x%08x, ROT_DBUGG_7: 0x%08x, ROT_DEBUBG_8: 0x%08x\n", 
            value[15], value[16], value[17]);
        CMDQ_ERR( "[CMDQ] ROT_DBUGG_9: 0x%08x, ROT_DBUGG_A: 0x%08x, ROT_DEBUBG_B: 0x%08x\n", 
            value[18], value[19], value[20]);
        CMDQ_ERR( "[CMDQ] ROT_DBUGG_C: 0x%08x, ROT_DBUGG_D: 0x%08x, ROT_DEBUBG_E: 0x%08x\n", 
            value[21], value[22], value[23]);
        CMDQ_ERR( "[CMDQ] ROT_DBUGG_F: 0x%08x, ROT_DBUGG_10: 0x%08x, ROT_DEBUBG_11: 0x%08x\n", 
            value[24], value[25], value[26]);
        CMDQ_ERR( "[CMDQ] ROT_DEBUG_12: 0x%08x, ROT_DBUGG_13: 0x%08x, ROT_DBUGG_14: 0x%08x\n", 
            value[27], value[28], value[29]);
        
        CMDQ_ERR( "[CMDQ] ROT_RST: 0x%08x, ROT_RST_STAT: 0x%08x, ROT_INIT: 0x%08x, [CMDQ] ROT_CROP_OFST: 0x%08x\n", 
            value[30], value[31], value[32], value[33]);
            
        value[0] = DISP_REG_GET(0xF4005034);
        value[1] = DISP_REG_GET(0xF4005038);
        value[2] = DISP_REG_GET(0xF400503c);
        value[3] = DISP_REG_GET(0xF4005054);
        value[4] = DISP_REG_GET(0xF4005064);                
        value[5] = DISP_REG_GET(0xF4005068);
        value[6] = DISP_REG_GET(0xF400506C);
        value[7] = DISP_REG_GET(0xF4005074);
        value[8] = DISP_REG_GET(0xF400507C);
        value[9] = DISP_REG_GET(0xF4005080);
        
        value[10] = DISP_REG_GET(0xF4005084);
        value[11] = DISP_REG_GET(0xF4005088);
        value[12] = DISP_REG_GET(0xF400508C);            
        value[13] = DISP_REG_GET(0xF4005090);
        value[14] = DISP_REG_GET(0xF4005094);  
        value[15] = DISP_REG_GET(0xF4005098);
        value[16] = DISP_REG_GET(0xF400509C);
        value[17] = DISP_REG_GET(0xF40050A0);
        value[18] = DISP_REG_GET(0xF40050A4);
        value[19] = DISP_REG_GET(0xF40050AC);
        value[20] = DISP_REG_GET(0xF40050B0);
        value[21] = DISP_REG_GET(0xF40050B4);
        value[22] = DISP_REG_GET(0xF40050B8);            
        value[23] = DISP_REG_GET(0xF40050BC);
        value[24] = DISP_REG_GET(0xF40050C0);
        value[25] = DISP_REG_GET(0xF40050C4);
        value[26] = DISP_REG_GET(0xF40050C8);
        value[27] = DISP_REG_GET(0xF40050CC);
        
        CMDQ_ERR( "[CMDQ] ROT_BASE_ADDR_C: 0x%08x, ROT_OFST_ADDR_C: 0x%08x, ROT_STRIDE_C: 0x%08x, ROT_DITHER: 0x%08x\n", 
            value[0], value[1], value[2], value[3]);            
        CMDQ_ERR( "[CMDQ] ROT_BASE_ADDR_V: 0x%08x, ROT_OFST_ADDR_V: 0x%08x, ROT_STRIDE_V: 0x%08x", 
            value[4], value[5], value[6]);
        CMDQ_ERR( "[CMDQ] DMA_PREULTRA: 0x%08x, ROT_ROT_EN: 0x%08x, ROT_FIFO_TEST: 0x%08x\n", 
            value[7], value[8], value[9]);
            
        CMDQ_ERR( "[CMDQ] ROT_MAT_CTRL: 0x%08x, ROT_MAT_RMY: 0x%08x, ROT_MAT_RMV: 0x%08x\n", 
            value[10], value[11], value[12]);
        CMDQ_ERR( "[CMDQ] ROT_GMY: 0x%08x, ROT_BMY: 0x%08x, ROT_BMV: 0x%08x\n", 
            value[13], value[14], value[15]);
        CMDQ_ERR( "[CMDQ] ROT_MAT_PREADD: 0x%08x, ROT_MAT_POSTADD: 0x%08x, DITHER_00: 0x%08x\n", 
            value[16], value[17], value[18]);
        CMDQ_ERR( "[CMDQ] ROT_DITHER_02: 0x%08x, ROT_DITHER_03: 0x%08x, ROT_DITHER_04: 0x%08x\n", 
            value[19], value[20], value[21]);
        CMDQ_ERR( "[CMDQ] ROT_DITHER_05: 0x%08x, ROT_DITHER_06: 0x%08x, ROT_DITHER_07: 0x%08x\n", 
            value[22], value[23], value[24]);
        CMDQ_ERR( "[CMDQ] ROT_DITHER_08: 0x%08x, ROT_DITHER_09: 0x%08x, ROT_DITHER_10: 0x%08x\n", 
            value[25], value[26], value[27]);
        
    }

    if (engineFlag & (0x01 << tWDMA1))
    {
        value[0] = DISP_REG_GET(0xF4004014);
        value[1] = DISP_REG_GET(0xF4004018);
        value[2] = DISP_REG_GET(0xF4004028);
        value[3] = DISP_REG_GET(0xF4004024);
        value[4] = DISP_REG_GET(0xF4004078);
        value[5] = DISP_REG_GET(0xF4004080);
        value[6] = DISP_REG_GET(0xF40040A0);
        value[7] = DISP_REG_GET(0xF40040A8);

        DISP_REG_SET(0xF4004014, (value[0] & (0x0FFFFFFF)));
        value[8] = DISP_REG_GET(0xF4004014);
        value[9] = DISP_REG_GET(0xF40040AC);
        DISP_REG_SET(0xF4004014, 0x10000000  | (value[0] & (0x0FFFFFFF)));
        value[10] = DISP_REG_GET(0xF4004014);
        value[11] = DISP_REG_GET(0xF40040AC);
        DISP_REG_SET(0xF4004014, 0x20000000  | (value[0] & (0x0FFFFFFF)));
        value[12] = DISP_REG_GET(0xF4004014);
        value[13] = DISP_REG_GET(0xF40040AC);
        DISP_REG_SET(0xF4004014, 0x30000000  | (value[0] & (0x0FFFFFFF)));
        value[14] = DISP_REG_GET(0xF4004014);
        value[15] = DISP_REG_GET(0xF40040AC);
        DISP_REG_SET(0xF4004014, 0x40000000  | (value[0] & (0x0FFFFFFF)));
        value[16] = DISP_REG_GET(0xF4004014);
        value[17] = DISP_REG_GET(0xF40040AC);
        DISP_REG_SET(0xF4004014, 0x50000000  | (value[0] & (0x0FFFFFFF)));
        value[18] = DISP_REG_GET(0xF4004014);
        value[19] = DISP_REG_GET(0xF40040AC);
        DISP_REG_SET(0xF4004014, 0x60000000  | (value[0] & (0x0FFFFFFF)));
        value[20] = DISP_REG_GET(0xF4004014);
        value[21] = DISP_REG_GET(0xF40040AC);
        DISP_REG_SET(0xF4004014, 0x70000000  | (value[0] & (0x0FFFFFFF)));
        value[22] = DISP_REG_GET(0xF4004014);
        value[23] = DISP_REG_GET(0xF40040AC);
        DISP_REG_SET(0xF4004014, 0x80000000  | (value[0] & (0x0FFFFFFF)));
        value[24] = DISP_REG_GET(0xF4004014);
        value[25] = DISP_REG_GET(0xF40040AC);
        DISP_REG_SET(0xF4004014, 0x90000000  | (value[0] & (0x0FFFFFFF)));
        value[26] = DISP_REG_GET(0xF4004014);
        value[27] = DISP_REG_GET(0xF40040AC);
        DISP_REG_SET(0xF4004014, 0xA0000000  | (value[0] & (0x0FFFFFFF)));
        value[28] = DISP_REG_GET(0xF4004014);
        value[29] = DISP_REG_GET(0xF40040AC);
        DISP_REG_SET(0xF4004014, 0xB0000000  | (value[0] & (0x0FFFFFFF)));
        value[30] = DISP_REG_GET(0xF4004014);
        value[31] = DISP_REG_GET(0xF40040AC);
        DISP_REG_SET(0xF4004014, 0xC0000000  | (value[0] & (0x0FFFFFFF)));
        value[32] = DISP_REG_GET(0xF4004014);
        value[33] = DISP_REG_GET(0xF40040AC);
        DISP_REG_SET(0xF4004014, 0xD0000000  | (value[0] & (0x0FFFFFFF)));
        value[34] = DISP_REG_GET(0xF4004014);
        value[35] = DISP_REG_GET(0xF40040AC);
        DISP_REG_SET(0xF4004014, 0xE0000000  | (value[0] & (0x0FFFFFFF)));
        value[36] = DISP_REG_GET(0xF4004014);
        value[37] = DISP_REG_GET(0xF40040AC);
        DISP_REG_SET(0xF4004014, 0xF0000000  | (value[0] & (0x0FFFFFFF)));
        value[38] = DISP_REG_GET(0xF4004014);
        value[39] = DISP_REG_GET(0xF40040AC);

        CMDQ_ERR( "=============== [CMDQ] WDMA Status ====================================\n");
        CMDQ_ERR( "[CMDQ]WDMA_CFG: 0x%08x, WDMA_SRC_SIZE: 0x%08x, WDMA_DST_W_IN_BYTE = 0x%08x\n", 
            value[0], value[1], value[2]);
        CMDQ_ERR( "[CMDQ]WDMA_DST_ADDR0: 0x%08x, WDMA_DST_UV_PITCH: 0x%08x, WDMA_DST_ADDR_OFFSET0 = 0x%08x\n", 
            value[3], value[4], value[5]);
        CMDQ_ERR( "[CMDQ]WDMA_STATUS: 0x%08x, WDMA_INPUT_CNT: 0x%08x\n", 
            value[6], value[7]);

        //Dump Addtional WDMA debug info
        CMDQ_ERR( "WDMA_DEBUG_0 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[8], value[9]);
        CMDQ_ERR( "WDMA_DEBUG_1 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[10], value[11]);
        CMDQ_ERR( "WDMA_DEBUG_2 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[12], value[13]);
        CMDQ_ERR( "WDMA_DEBUG_3 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[14], value[15]);
        CMDQ_ERR( "WDMA_DEBUG_4 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[16], value[17]);
        CMDQ_ERR( "WDMA_DEBUG_5 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[18], value[19]);
        CMDQ_ERR( "WDMA_DEBUG_6 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[20], value[21]);
        CMDQ_ERR( "WDMA_DEBUG_7 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[22], value[23]);
        CMDQ_ERR( "WDMA_DEBUG_8 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[24], value[25]);
        CMDQ_ERR( "WDMA_DEBUG_9 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[26], value[27]);
        CMDQ_ERR( "WDMA_DEBUG_A 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[28], value[29]);
        CMDQ_ERR( "WDMA_DEBUG_B 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[30], value[31]);
        CMDQ_ERR( "WDMA_DEBUG_C 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[32], value[33]);
        CMDQ_ERR( "WDMA_DEBUG_D 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[34], value[35]);
        CMDQ_ERR( "WDMA_DEBUG_E 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[36], value[37]);
        CMDQ_ERR( "WDMA_DEBUG_F 0xF4004014: 0x%08x , 0xF40040ac: 0x%08x \n", value[38], value[39]);
    }


    // verbose case, dump entire 1KB HW register region
    // for each enabled HW module.
    if (logLevel >= 1)
    {
        int inner = 0;
        static const MODULE_BASE bases[] = {
                DEFINE_MODULE(tRDMA0, 0xF4001000),
                DEFINE_MODULE(tSCL0,  0xF4002000),
                DEFINE_MODULE(tSCL1,  0xF4003000),
                DEFINE_MODULE(tTDSHP, 0xF4006000),
                DEFINE_MODULE(tWROT,  0xF4005000),
                DEFINE_MODULE(tWDMA1, 0xF4004000),
            };
        
        for (inner = 0; inner < (sizeof(bases) / sizeof(bases[0])); ++inner)
        {
            if (engineFlag & (0x01 << bases[inner].engine))
            {
                CMDQ_ERR( "========= [CMDQ] %s dump base 0x%08x ========\n", bases[inner].name, bases[inner].base);
                print_hex_dump(KERN_ERR, "", DUMP_PREFIX_ADDRESS, 32, 4,
                       bases[inner].base, 1024, false);
            }
        }
    }
}
