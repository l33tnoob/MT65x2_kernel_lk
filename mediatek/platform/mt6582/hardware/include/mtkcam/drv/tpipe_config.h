#ifndef __TPIPE_ISP_CONFIG_H__
#define __TPIPE_ISP_CONFIG_H__

/* following define can be changed with rebuilding tpipe_driver.c */
#define LOG_REMARK_MUST_FILL_STR "<must>" /* can be changed */
#define LOG_REMARK_NOT_FOUND_STR "<no config>?" /* can be changed */
#define LOG_REMARK_HW_NOT_FOUND_STR "<no hw>?" /* can be changed */
#define LOG_REMARK_HW_REG_STR "<hw>" /* can be changed */
#define LOG_REMARK_DEBUG_STR "<debug>" /* can be changed */
#define LOG_REMARK_HW_DIFF_STR "<hw diff> " /* can be changed */
#define LOG_REMARK_HW_LOG_STR "<hw log>" /* can be changed */
#define TPIPE_DEBUG_DUMP_HEADER "[ISP/MDP][tpipe_dump]"
#define TPIPE_DEBUG_DUMP_START "start MT6582"
#define TPIPE_DEBUG_DUMP_END "end MT6582"
#define TPIPE_LOG_PARSE_FILE_NAME "tpipe_log_parse_"
#define TPIPE_LOG_PARSE_FILE_EXT ".txt"
#define TPIPE_REG_DUMP_HEADER "0x"
#define TPIPE_REG_DUMP_SKIP_NO (4096)

#define ISP_ERROR_MESSAGE_DATA(n, CMD) \
    CMD(n, ISP_TPIPE_MESSAGE_OK)\
    CMD(n, ISP_TPIPE_MESSAGE_FAIL)\
    /* final count, can not be changed */\
    CMD(n, ISP_TPIPE_MESSAGE_MAX_NO)\

#define ISP_TPIPE_ENUM_DECLARE(a, b) b,
#define ISP_TPIPE_ENUM_STRING(n, a) if (a == n) return #a;

#define GET_ISP_ERROR_NAME(n) \
    if (0 == n) return "ISP_TPIPE_MESSAGE_UNKNOWN";\
    ISP_ERROR_MESSAGE_DATA(n, ISP_TPIPE_ENUM_STRING)\
    return "";

/* error enum */
typedef enum ISP_TPIPE_MESSAGE_ENUM
{
    ISP_TPIPE_MESSAGE_UNKNOWN=0,
    ISP_ERROR_MESSAGE_DATA(,ISP_TPIPE_ENUM_DECLARE)
}ISP_TPIPE_MESSAGE_ENUM;

/* tpipe_irq_mode */
typedef enum TPIPE_IRQ_MODE_ENUM
{
    TPIPE_IRQ_FRAME_STOP=0,
    TPIPE_IRQ_LINE_END,
    TPIPE_IRQ_PER_TPIPE,
    TPIPE_IRQ_MODE_MAX_NO
}TPIPE_IRQ_MODE_ENUM;

typedef enum TPIPE_Pass2CmdqNum_ENUM
{
	TPIPE_PASS2_CMDQ_NONE=0,
    TPIPE_PASS2_CMDQ_1,
    TPIPE_PASS2_CMDQ_2,
  	TPIPE_PASS2_CMDQ_NUM
}TPIPE_Pass2CmdqNum_ENUM;

typedef enum TPIPE_Pass2CmdqPrior_ENUM
{
	TPIPE_PASS2_CMDQ_PRIOR_LOW=1,
    TPIPE_PASS2_CMDQ_PRIOR_HIGH = 10
}TPIPE_Pass2CmdqPrior_ENUM;
typedef struct ISP_TPIPE_CONFIG_TOP_STRUCT
{
    int scenario;
    int mode;
    int debug_sel;
    int pixel_id;
    int cam_in_fmt;
    int tcm_load_en;
    int ctl_extension_en;
    int rsp_en;
    int mdp_crop_en;
    int imgi_en;
    int lce_en;
    int lcei_en;
    int lsci_en;
    int unp_en;
    int bnr_en;
    int lsc_en;
    int sl2_en;
    int c24_en;
    int cfa_en;
    int c42_en;
    int nbc_en;
    int seee_en;
    int imgo_en;
    int img2o_en;   
    int cdrz_en;
    int mdp_sel;
    int interlace_mode;
}ISP_TPIPE_CONFIG_TOP_STRUCT;

typedef struct ISP_TPIPE_CONFIG_SW_STRUCT
{
    int log_en;
    int src_width;
    int src_height;
    int tpipe_irq_mode;
    int tpipe_width;
    int tpipe_height;
}ISP_TPIPE_CONFIG_SW_STRUCT;

typedef struct ISP_TPIPE_CONFIG_IMGI_STRUCT
{
    int imgi_stride;
}ISP_TPIPE_CONFIG_IMGI_STRUCT;

typedef struct ISP_TPIPE_CONFIG_LCEI_STRUCT
{
    int lcei_stride;
}ISP_TPIPE_CONFIG_LCEI_STRUCT;
typedef struct ISP_TPIPE_CONFIG_LSCI_STRUCT
{
    int lsci_stride;
}ISP_TPIPE_CONFIG_LSCI_STRUCT;

typedef struct ISP_TPIPE_CONFIG_BNR_STRUCT
{
    int bpc_en;
}ISP_TPIPE_CONFIG_BNR_STRUCT;

typedef struct ISP_TPIPE_CONFIG_LSC_STRUCT
{
    int sdblk_width;
    int sdblk_xnum;
    int sdblk_last_width;
    int sdblk_height;
    int sdblk_ynum;
    int sdblk_last_height;
}ISP_TPIPE_CONFIG_LSC_STRUCT;

typedef struct ISP_TPIPE_CONFIG_CFA_STRUCT
{
    int bayer_bypass;
}ISP_TPIPE_CONFIG_CFA_STRUCT;
typedef struct ISP_TPIPE_CONFIG_LCE_STRUCT
{
    int lce_bc_mag_kubnx;
    int lce_slm_width;
    int lce_bc_mag_kubny;
    int lce_slm_height;
}ISP_TPIPE_CONFIG_LCE_STRUCT;

typedef struct ISP_TPIPE_CONFIG_NBC_STRUCT
{
    int anr_eny;
    int anr_enc;
    int anr_iir_mode;
    int anr_scale_mode;
}ISP_TPIPE_CONFIG_NBC_STRUCT;

typedef struct ISP_TPIPE_CONFIG_SEEE_STRUCT
{
    int se_edge;
}ISP_TPIPE_CONFIG_SEEE_STRUCT;

typedef struct ISP_TPIPE_CONFIG_IMGO_STRUCT
{
    int imgo_stride;
    int imgo_crop_en;
    int imgo_xoffset;
    int imgo_yoffset;
    int imgo_xsize;
    int imgo_ysize;
    int imgo_mux_en;
    int imgo_mux;
}ISP_TPIPE_CONFIG_IMGO_STRUCT;

typedef struct ISP_TPIPE_CONFIG_IMG2O_STRUCT
{
    int img2o_stride;
    int img2o_crop_en;
    int img2o_xoffset;
    int img2o_yoffset;
    int img2o_xsize;
    int img2o_ysize;
    int img2o_mux_en;
    int img2o_mux;
}ISP_TPIPE_CONFIG_IMG2O_STRUCT;

typedef struct ISP_TPIPE_CONFIG_CDRZ_STRUCT
{
    int cdrz_input_crop_width;
    int cdrz_input_crop_height;
    int cdrz_output_width;
    int cdrz_output_height;
    int cdrz_luma_horizontal_integer_offset;/* pixel base */
    int cdrz_luma_horizontal_subpixel_offset;/* 20 bits base */
    int cdrz_luma_vertical_integer_offset;/* pixel base */
    int cdrz_luma_vertical_subpixel_offset;/* 20 bits base */
    int cdrz_horizontal_luma_algorithm;/* 0~2 */
    int cdrz_vertical_luma_algorithm;/* can only select 0 */
    int cdrz_horizontal_coeff_step;
    int cdrz_vertical_coeff_step;
}ISP_TPIPE_CONFIG_CDRZ_STRUCT;

typedef struct ISP_TPIPE_CONFIG_SL2_STRUCT
{
    int sl2_hrz_comp;
}ISP_TPIPE_CONFIG_SL2_STRUCT;
typedef struct ISP_TPIPE_CONFIG_PASS2_STRUCT
{
    TPIPE_Pass2CmdqNum_ENUM Pass2CmdqNum;
	TPIPE_Pass2CmdqPrior_ENUM Pass2CmdqPriority;
}ISP_TPIPE_CONFIG_PASS2_STRUCT;

typedef struct ISP_TPIPE_CONFIG_STRUCT
{
    ISP_TPIPE_CONFIG_TOP_STRUCT top;
    ISP_TPIPE_CONFIG_SW_STRUCT sw;
    ISP_TPIPE_CONFIG_IMGI_STRUCT imgi;
    ISP_TPIPE_CONFIG_LCEI_STRUCT lcei;
    ISP_TPIPE_CONFIG_LSCI_STRUCT lsci;
    ISP_TPIPE_CONFIG_BNR_STRUCT bnr;
    ISP_TPIPE_CONFIG_LSC_STRUCT lsc;
    ISP_TPIPE_CONFIG_LCE_STRUCT lce;
    ISP_TPIPE_CONFIG_NBC_STRUCT nbc;
    ISP_TPIPE_CONFIG_SEEE_STRUCT seee;
    ISP_TPIPE_CONFIG_IMGO_STRUCT imgo;
    ISP_TPIPE_CONFIG_CDRZ_STRUCT cdrz;
    ISP_TPIPE_CONFIG_IMG2O_STRUCT img2o;
    ISP_TPIPE_CONFIG_CFA_STRUCT cfa;
    ISP_TPIPE_CONFIG_SL2_STRUCT sl2;
	ISP_TPIPE_CONFIG_PASS2_STRUCT pass2;
}ISP_TPIPE_CONFIG_STRUCT;

typedef struct ISP_TPIPE_INFORMATION_STRUCT
{
    unsigned int pos_xs;/* tpipe start */
    unsigned int pos_xe;/* tpipe end */
    unsigned int pos_ys;/* tpipe start */
    unsigned int pos_ye;/* tpipe end */
    unsigned int tpipe_stop_flag;/* stop flag */
    unsigned int dump_offset_no;/* word offset */
}ISP_TPIPE_INFORMATION_STRUCT;

typedef struct ISP_TPIPE_DESCRIPTOR_STRUCT
{
    unsigned int *tpipe_config;
    ISP_TPIPE_INFORMATION_STRUCT *tpipe_info;
    unsigned int used_word_no;
    unsigned int total_word_no;
    unsigned int config_no_per_tpipe;
    unsigned int used_tpipe_no;
    unsigned int total_tpipe_no;
    unsigned int horizontal_tpipe_no;
    unsigned int curr_horizontal_tpipe_no;
    unsigned int curr_vertical_tpipe_no;
}ISP_TPIPE_DESCRIPTOR_STRUCT;

extern int tpipe_main_query_platform_working_buffer_size(int tpipe_no);
extern ISP_TPIPE_MESSAGE_ENUM tpipe_main_platform(const ISP_TPIPE_CONFIG_STRUCT *ptr_tpipe_config,
                ISP_TPIPE_DESCRIPTOR_STRUCT *ptr_isp_tpipe_descriptor,
                char *ptr_working_buffer, int buffer_size);
#endif
