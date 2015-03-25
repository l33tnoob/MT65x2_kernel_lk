/*******************************************************************************
 *
 * Filename:
 * ---------
 *   meta_dfo_para.h
 *
 * Project:
 * --------
 *   YUSU
 *
 * Description:
 * ------------
 *   header file of main function
 *
 * Author:
 * -------
 *   Brett Ting (mtk04357) 12/11/2012
 *
 *******************************************************************************/

#ifndef __META_DFO_PARA_H__
#define __META_DFO_PARA_H__

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum{
    DFO_OP_QUERY_STATUS = 0,
    DFO_OP_READ_COUNT,
    DFO_OP_READ,
    DFO_OP_WRITE,
    DFO_OP_COMBO_COUNT,
    DFO_OP_COMBO_READ,
    DFO_OP_COMBO_MODE,
    DFO_OP_COMBO_UPDATE,
    DFO_END
} DFO_OP;


typedef struct
{
    unsigned char reserved;
} Dfo_query_status_req;

typedef struct
{
    unsigned char support;
} Dfo_query_status_cnf;


typedef struct
{
    unsigned char reserved;
} Dfo_read_count_req;

typedef struct
{
    int     count;
} Dfo_read_count_cnf;

typedef struct
{
    int     index;          // [0, count)
} Dfo_read_req;

typedef struct
{
    char    name[32];       // read result, dfo name
    int     value;          // read result, dfo value
    int     partition;      // 0: NVRAM, 1: MISC
} Dfo_read_cnf;

typedef struct
{
    char    name[32];
    int     value;
    int     partition;      // 0: NVRAM, 1: MISC
    int     save;           // 0: don't save to MISC/NVRAM, 1: save to MISC/NVRAM
} Dfo_write_req;

typedef struct
{
    unsigned char status;
} Dfo_write_cnf;

typedef struct
{
    unsigned char reserved;
} Dfo_combo_count_req;

typedef struct
{
    int     count;
} Dfo_combo_count_cnf;

typedef struct
{
    int     index;          // [0, count) of combo
} Dfo_combo_read_req;

typedef struct
{
    char    name[32];       // read result, combo name
    int     modeCount;      // read result, the number of mode
} Dfo_combo_read_cnf;

typedef struct
{
    int     index;          // [0, count) of combo
    int     modeIndex;      // [0, modeCount) of the mode of a combo
} Dfo_combo_mode_req;

typedef struct
{
    char    name[32];       // read result, the mode name of a combo
} Dfo_combo_mode_cnf;

typedef struct
{
    int     index;          // [0, count) of combo
    int     modeIndex;      // [0, modeCount) of the mode of a combo
} Dfo_combo_update_req;

typedef struct
{
    int     status;
} Dfo_combo_update_cnf;

typedef union
{
    Dfo_query_status_req  query_status_req;
    Dfo_read_count_req    read_count_req;
    Dfo_read_req          read_req;
    Dfo_write_req         write_req;
    Dfo_combo_count_req   combo_count_req;
    Dfo_combo_read_req    combo_read_req;
    Dfo_combo_mode_req    combo_mode_req;
    Dfo_combo_update_req  combo_update_req;
} FT_DFO_CMD;

typedef union 
{
    Dfo_query_status_cnf query_status_cnf;
    Dfo_read_count_cnf   read_count_cnf;
    Dfo_read_cnf         read_cnf;
    Dfo_write_cnf        write_cnf;
    Dfo_combo_count_cnf  combo_count_cnf;
    Dfo_combo_read_cnf   combo_read_cnf;
    Dfo_combo_mode_cnf   combo_mode_cnf;
    Dfo_combo_update_cnf combo_update_cnf;
} FT_DFO_RESULT;

typedef struct {
    FT_H        header;  
    DFO_OP      op;
    FT_DFO_CMD  cmd;
} FT_DFO_REQ;

typedef struct {
    FT_H           header;
    DFO_OP         op;
    unsigned char  status;   // The data frame state, 0 means normal
    FT_DFO_RESULT  result;
} FT_DFO_CNF;

bool    META_Dfo_Init(void);
void    META_Dfo_Deinit(void);
void    META_Dfo_OP(FT_DFO_REQ* req);

#ifdef __cplusplus
}
#endif

#endif
