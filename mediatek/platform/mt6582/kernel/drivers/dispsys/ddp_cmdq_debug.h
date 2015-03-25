#ifndef __DDP_CMDQ_DEBUG_H__
#define __DDP_CMDQ_DEBUG_H__

#include "ddp_cmdq.h"

uint32_t* cmdq_core_get_pc(struct TaskStruct *pTask, uint32_t thread, uint32_t insts[4]);
const char* cmdq_core_get_event_name(uint32_t event);
const char* cmdq_core_parse_subsys(uint32_t argA, uint32_t argB);
const char* cmdq_core_parse_op(uint32_t opCode);
void cmdq_core_parse_error(struct TaskStruct *pTask, uint32_t thread, 
                           const char **moduleName, uint32_t *instA, uint32_t *instB);

uint32_t* cmdq_core_dump_pc(TaskStruct *pTask, int thread, const char *tag);
void cmdq_core_dump_mdp_status(uint32_t engineFlag, int32_t logLevel);
    
#endif // __DDP_CMDQ_DEBUG_H__