#ifndef __TPIPE_CONFIG_H__
#define __TPIPE_CONFIG_H__

#define ISP_ERROR_MESSAGE_DATA(n, CMD) \
    CMD(n, ISP_TPIPE_MESSAGE_OK)\
    CMD(n, ISP_TPIPE_MESSAGE_FAIL)\
    /* final count, can not be changed */\
    CMD(n, ISP_TPIPE_MESSAGE_MAX_NO)

#define ISP_TPIPE_ENUM_DECLARE(a, b) b,
#define ISP_TPIPE_ENUM_STRING(n, a) (a == n)?#a:

#define GET_ISP_ERROR_NAME(n) \
    (0 == n)?"ISP_TPIPE_MESSAGE_UNKNOWN":\
    ISP_ERROR_MESSAGE_DATA(n, ISP_TPIPE_ENUM_STRING)\
    ""

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

typedef struct ISP_TPIPE_CONFIG_TOP_STRUCT
{
    int scenario;
    int mode;
    int debug_sel;
    int pixel_id;
    int cam_in_fmt;
    int imgi_en;
    int imgci_en;
    int vipi_en;
    int vip2i_en;
    int flki_en;
    int lce_en;
    int lcei_en;
    int lsci_en;
    int unp_en;
    int bnr_en;
    int lsc_en;
    int mfb_en;
    int c02_en;
    int c24_en;
    int cfa_en;
    int c42_en;
    int nbc_en;
    int seee_en;
    int imgo_en;
    int img2o_en;   
    int esfko_en;
    int aao_en;
    int lcso_en;
    int cdrz_en;
    int curz_en;
    int fe_sel;
    int fe_en;
    int prz_en;
    int disp_vid_sel;
    int g2g2_en;
    int vido_en;
    int dispo_en;
    int nr3d_en;
    /* released yet */
    //int prz_opt_sel;
}ISP_TPIPE_CONFIG_TOP_STRUCT;

typedef struct ISP_TPIPE_CONFIG_SW_STRUCT
{
    int log_en;
    int src_width;
    int src_height;
    int tpipe_irq_mode;
    int tpipe_width;
    int tpipe_height;
    int ring_buffer_mcu_no;
    int ring_buffer_mcu_y_size;
    /* released yet */
    //int vr_3dnr_x_offset;
    //int vr_3dnr_y_offset;
    //int zsd_3dnr_rdma_x_offset;
    //int zsd_3dnr_rdma_y_offset;
    //int zsd_3dnr_wdma_x_offset;
    //int zsd_3dnr_wdma_y_offset;
}ISP_TPIPE_CONFIG_SW_STRUCT;

typedef struct ISP_TPIPE_CONFIG_IMGI_STRUCT
{
    int imgi_stride;
    int imgi_ring_en;
    int imgi_ring_size;/* 256 base */
}ISP_TPIPE_CONFIG_IMGI_STRUCT;

typedef struct ISP_TPIPE_CONFIG_IMGCI_STRUCT
{
    int imgci_stride;
}ISP_TPIPE_CONFIG_IMGCI_STRUCT;

typedef struct ISP_TPIPE_CONFIG_VIPI_STRUCT
{
    int vipi_stride;
    int vipi_ring_en;
    int vipi_ring_size;/* 256 base */
}ISP_TPIPE_CONFIG_VIPI_STRUCT;

typedef struct ISP_TPIPE_CONFIG_VIP2I_STRUCT
{
    int vip2i_stride;
    int vip2i_ring_en;
    int vip2i_ring_size;/* 256 base */
}ISP_TPIPE_CONFIG_VIP2I_STRUCT;

typedef struct ISP_TPIPE_CONFIG_FLKI_STRUCT
{
    int flki_stride;
}ISP_TPIPE_CONFIG_FLKI_STRUCT;

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
    int bpc_tbl_en;
    int bpc_tbl_size;/* bad pixel table width */
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

typedef struct ISP_TPIPE_CONFIG_MFB_STRUCT
{
    int bld_mode;
    int bld_deblock_en;
}ISP_TPIPE_CONFIG_MFB_STRUCT;

typedef struct ISP_TPIPE_CONFIG_CFA_STRUCT
{
    int bayer_bypass;
}ISP_TPIPE_CONFIG_CFA_STRUCT;

typedef struct ISP_TPIPE_CONFIG_LCE_STRUCT
{
    int lce_bc_mag_kubnx;
    int lce_offset_x;
    int lce_bias_x;
    int lce_slm_width;
    int lce_bc_mag_kubny;
    int lce_offset_y;
    int lce_bias_y;
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
    int usm_over_shrink_en;
}ISP_TPIPE_CONFIG_SEEE_STRUCT;

typedef struct ISP_TPIPE_CONFIG_IMGO_STRUCT
{
    int imgo_stride;
    int imgo_crop_en;
}ISP_TPIPE_CONFIG_IMGO_STRUCT;

typedef struct ISP_TPIPE_CONFIG_ESFKO_STRUCT
{
    int esfko_stride;
}ISP_TPIPE_CONFIG_ESFKO_STRUCT;

typedef struct ISP_TPIPE_CONFIG_AAO_STRUCT
{
    int aao_stride;
}ISP_TPIPE_CONFIG_AAO_STRUCT;

typedef struct ISP_TPIPE_CONFIG_LCSO_STRUCT
{
    int lcso_stride;
    int lcso_crop_en;   
}ISP_TPIPE_CONFIG_LCSO_STRUCT;

typedef struct ISP_TPIPE_CONFIG_FE_STRUCT
{
    int fem_harris_tpipe_mode;
}ISP_TPIPE_CONFIG_FE_STRUCT;

typedef struct ISP_TPIPE_CONFIG_IMG2O_STRUCT
{
    int img2o_stride;
    int img2o_crop_en;
}ISP_TPIPE_CONFIG_IMG2O_STRUCT;

typedef struct ISP_TPIPE_CONFIG_CDRZ_STRUCT
{
    int cdrz_input_crop_width;
    int cdrz_input_crop_height;
    int cdrz_output_width;
    int cdrz_output_height;
    int cdrz_horizontal_integer_offset;/* pixel base */
    int cdrz_horizontal_subpixel_offset;/* 20 bits base */
    int cdrz_vertical_integer_offset;/* pixel base */
    int cdrz_vertical_subpixel_offset;/* 20 bits base */
    int cdrz_horizontal_luma_algorithm;
    int cdrz_vertical_luma_algorithm;
    int cdrz_horizontal_coeff_step;
    int cdrz_vertical_coeff_step;
}ISP_TPIPE_CONFIG_CDRZ_STRUCT;

typedef struct ISP_TPIPE_CONFIG_CURZ_STRUCT
{
    int curz_input_crop_width;
    int curz_input_crop_height;
    int curz_output_width;
    int curz_output_height;
    int curz_horizontal_integer_offset;/* pixel base */
    int curz_horizontal_subpixel_offset;/* 20 bits base */
    int curz_vertical_integer_offset;/* pixel base */
    int curz_vertical_subpixel_offset;/* 20 bits base */
    int curz_horizontal_coeff_step;
    int curz_vertical_coeff_step;
}ISP_TPIPE_CONFIG_CURZ_STRUCT;

typedef struct ISP_TPIPE_CONFIG_PRZ_STRUCT
{
    /* released yet */
    //int prz_input_crop_width;
    //int prz_input_crop_height;
    int prz_output_width;
    int prz_output_height;
    int prz_horizontal_integer_offset;/* pixel base */
    int prz_horizontal_subpixel_offset;/* 20 bits base */
    int prz_vertical_integer_offset;/* pixel base */
    int prz_vertical_subpixel_offset;/* 20 bits base */
    int prz_horizontal_luma_algorithm;
    int prz_vertical_luma_algorithm;
    int prz_horizontal_coeff_step;
    int prz_vertical_coeff_step;
}ISP_TPIPE_CONFIG_PRZ_STRUCT;

typedef struct ISP_TPIPE_CONFIG_VIDO_STRUCT
{
    int vido_rotation;
    int vido_flip;
    int vido_format_1;
    int vido_format_3;
    int vido_stride;
    int vido_stride_c;
    int vido_stride_v;
    int vido_crop_en;
}ISP_TPIPE_CONFIG_VIDO_STRUCT;

typedef struct ISP_TPIPE_CONFIG_DISPO_STRUCT
{
    int dispo_rotation;
    int dispo_flip;
    int dispo_format_1;
    int dispo_format_3;
    int dispo_stride;
    int dispo_stride_c;
    int dispo_stride_v;
    int dispo_crop_en;
}ISP_TPIPE_CONFIG_DISPO_STRUCT;

typedef struct ISP_TPIPE_CONFIG_STRUCT
{
    ISP_TPIPE_CONFIG_TOP_STRUCT top;
    ISP_TPIPE_CONFIG_SW_STRUCT sw;
    ISP_TPIPE_CONFIG_IMGI_STRUCT imgi;
    ISP_TPIPE_CONFIG_IMGCI_STRUCT imgci;
    ISP_TPIPE_CONFIG_VIPI_STRUCT vipi;
    ISP_TPIPE_CONFIG_VIP2I_STRUCT vip2i;
    ISP_TPIPE_CONFIG_LCEI_STRUCT lcei;
    ISP_TPIPE_CONFIG_LSCI_STRUCT lsci;
    ISP_TPIPE_CONFIG_BNR_STRUCT bnr;
    ISP_TPIPE_CONFIG_LSC_STRUCT lsc;
    ISP_TPIPE_CONFIG_LCE_STRUCT lce;
    ISP_TPIPE_CONFIG_NBC_STRUCT nbc;
    ISP_TPIPE_CONFIG_SEEE_STRUCT seee;
    ISP_TPIPE_CONFIG_IMGO_STRUCT imgo;
    ISP_TPIPE_CONFIG_ESFKO_STRUCT esfko;
    ISP_TPIPE_CONFIG_AAO_STRUCT aao;
    ISP_TPIPE_CONFIG_LCSO_STRUCT lcso;
    ISP_TPIPE_CONFIG_CDRZ_STRUCT cdrz;
    ISP_TPIPE_CONFIG_CURZ_STRUCT curz;
    ISP_TPIPE_CONFIG_FE_STRUCT fe;
    ISP_TPIPE_CONFIG_IMG2O_STRUCT img2o;
    ISP_TPIPE_CONFIG_PRZ_STRUCT prz;
    ISP_TPIPE_CONFIG_VIDO_STRUCT vido;
    ISP_TPIPE_CONFIG_DISPO_STRUCT dispo;
    ISP_TPIPE_CONFIG_MFB_STRUCT mfb;
    ISP_TPIPE_CONFIG_FLKI_STRUCT flki;
    ISP_TPIPE_CONFIG_CFA_STRUCT cfa;
}ISP_TPIPE_CONFIG_STRUCT;

typedef struct ISP_TPIPE_INFORMATION_STRUCT
{
    unsigned int mcu_buffer_start_no;/* mcu row start no */
    unsigned int mcu_buffer_end_no;/* mcu row end no */
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
#endif /* __TPIPE_CONFIG_H__ */
