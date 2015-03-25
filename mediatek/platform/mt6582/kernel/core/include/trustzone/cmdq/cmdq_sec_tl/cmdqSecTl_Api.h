#ifndef __CMDQSECTL_API_H__
#define __CMDQSECTL_API_H__

//#include "tci.h"
//#include "cmdq_sec_iwc_common.h"


/*
 * Command ID's for communication Trustlet Connector -> Trustlet.
 */
#define CMD_CMDQ_TL_SUBMIT_TASK      1
#define CMD_CMDQ_TL_DUMP_TASK        2
#define CMD_CMDQ_TL_TEST_TASK_IWC    3
#define CMD_CMDQ_TL_TEST_DUMMY       4
#define CMD_CMDQ_TL_TEST_SIG_WAIT    5
#define CMD_CMDQ_TL_DEBUG_SW_COPY    6


/*
 * Termination codes
 */
#define EXIT_ERROR                  ((uint32_t)(-1))

/*
 * TCI message data: see cmdq_sec_iwc_common.h
 */
 
/*
 * Trustlet UUID.
 */
#define TL_CMDQ_UUID { { 9, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

#endif // __CMDQSECTEST_API_H__
