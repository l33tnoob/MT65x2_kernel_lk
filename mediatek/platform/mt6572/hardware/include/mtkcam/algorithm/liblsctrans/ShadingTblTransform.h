#ifndef SHADINGTBLTRANSFORM_H
#define SHADINGTBLTRANSFORM_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define LSCTRANS_SUPPORT_CH_NUM          (4)
#define LSCTRANS_MAX_FRM_GRID_NUM                (16)
#define LSCTRANS_MAX_TIL_GRID_NUM                (32)

// temporary size *in byte*, added by Maggie, will need to be cleaned up later after the packing method is determined
#define MAX_FRM_GAIN_SIZE		(LSCTRANS_MAX_FRM_GRID_NUM*LSCTRANS_MAX_FRM_GRID_NUM*LSCTRANS_SUPPORT_CH_NUM*2) // 2 bytes/pixel/channel for each gain
#define MAX_TIL_GAIN_SIZE		(LSCTRANS_MAX_TIL_GRID_NUM*LSCTRANS_MAX_TIL_GRID_NUM*LSCTRANS_SUPPORT_CH_NUM*2) // 2 bytes/pixel/channel for each gain
#define MAX_FRM_COEFF_SIZE		((LSCTRANS_MAX_FRM_GRID_NUM-1)*(LSCTRANS_MAX_FRM_GRID_NUM-1)*LSCTRANS_SUPPORT_CH_NUM*12/3*4) // pack every 3 coef into 4 bytes 
#define MAX_TIL_COEFF_SIZE		((LSCTRANS_MAX_TIL_GRID_NUM-1)*(LSCTRANS_MAX_TIL_GRID_NUM-1)*LSCTRANS_SUPPORT_CH_NUM*12/3*4) // pack every 3 coef into 4 bytes
                                            // (each coefficient takes 10, 11 or 12 bits)

// Added By Janice, working buffer size of each function
//=================== Working Buffer Size Counting (in bytes) =====================//
#define SHADIND_FUNC_WORKING_BUFFER_SIZE                \
/*g_p_sdblk int  [(MAX_TIL_GRID_NUM-1)*(MAX_TIL_GRID_NUM-1)*12*4] (184512)*/			(((LSCTRANS_MAX_TIL_GRID_NUM-1)*(LSCTRANS_MAX_TIL_GRID_NUM-1)*12*4))*4\
/*g_coef_t_1d int[4*(MAX_TIL_GRID_NUM-1)*(MAX_TIL_GRID_NUM-1)*12] (184512) */			+(4*((LSCTRANS_MAX_TIL_GRID_NUM-1)*(LSCTRANS_MAX_TIL_GRID_NUM-1))*12)*4\
/*g_p_til_gain_1 int[MAX_SHADING_TIL_GAIN_SIZE/sizeof(UINT32)]	(8192) */				+MAX_TIL_GAIN_SIZE\
/*g_p_til_gain_2 int[MAX_SHADING_TIL_GAIN_SIZE/sizeof(UINT32)]	(8192) */				+MAX_TIL_GAIN_SIZE\
/*g_frm_coef int[MAX_SHADING_FRM_COEFF_SIZE/sizeof(UINT32)]		(14400)*/				+MAX_FRM_COEFF_SIZE\
/*g_zh2_1d float[MAX_TIL_GRID_NUM*MAX_TIL_GRID_NUM]				(4096)*/				+(LSCTRANS_MAX_TIL_GRID_NUM)*(LSCTRANS_MAX_TIL_GRID_NUM)*4\
/*g_zv2_1d float[MAX_TIL_GRID_NUM*MAX_TIL_GRID_NUM]				(4096)*/				+(LSCTRANS_MAX_TIL_GRID_NUM)*(LSCTRANS_MAX_TIL_GRID_NUM)*4\
/*g_zz_1d float[4*MAX_TIL_GRID_NUM*MAX_TIL_GRID_NUM]			(16384)*/				+(LSCTRANS_MAX_TIL_GRID_NUM)*(LSCTRANS_MAX_TIL_GRID_NUM)*4*4\


typedef enum
{
    BAYER_B,
    BAYER_GB,
    BAYER_GR,
    BAYER_R
} BAYER_ID_T;

typedef enum
{
  SHADING_TYPE_GAIN,  // gain table
  SHADING_TYPE_COEFF  // coeff table
} SHADING_TYPE;

// change the id order, so that 0~3 is native to the coordinate flipping
// low bit: flip x (horizontal) coordinate
// high bit: flip y (vertical) coordinate
typedef enum
{
  SHADING_AFN_R0D,    // clockwise rotation 0 degree,	rotation id: 0
  SHADING_AFN_MIRROR, // left-right,					rotation id: 1
  SHADING_AFN_FLIP,    // up-down,						rotation id: 2
  SHADING_AFN_R180D,  // clockwise rotation 180 degree,	rotation id: 3
  SHADING_AFN_R90D,   // clockwise rotation 90 degree,	not being supported
  SHADING_AFN_R270D,  // clockwise rotation 270 degree, not being supported
  SHADING_AFN_MAX
} SHADING_AFN_T;

typedef enum
{
  SHADING_GRGB_SAME_NO,
  SHADING_GRGB_SAME_YES
} SHADING_GRGB_SAME;	// if to lock the gr and gb to the same gain. Default is no.

typedef struct SHADING_TBL_SPEC
{
  unsigned int            img_width;   // orig/target img width
  unsigned int            img_height;  // orig/target img height
  unsigned int            offset_x;    // (0,0) at the upper left corner
  unsigned int            offset_y;    // (0,0) at the upper left corner
  unsigned int            crop_width;  // actual width of the input image
  unsigned int            crop_height; // actual height of the input image
  BAYER_ID_T        bayer;       // bayer id of the input image
  unsigned int            grid_x;      // input/output table x grid number
  unsigned int            grid_y;      // input/output table y grid number
  unsigned int            lwidth;      // input/output table last block width
  unsigned int            lheight;     // input/output table last block height
  unsigned int			ratio_idx;	 // index for the compensation strength (0~20)//(0~10)
  SHADING_GRGB_SAME grgb_same;	 // whether to use the same gains for Gr and Gb
  unsigned int            *table;      // input/output table
  SHADING_TYPE      data_type;   // input/output table type
} SHADING_TBL_SPEC;

typedef struct
{
  SHADING_TBL_SPEC  input;
  SHADING_TBL_SPEC  output;
  SHADING_AFN_T     afn;
  void				*working_buff_addr;
  unsigned int		working_buff_size;
} SHADIND_TRFM_CONF;

typedef struct SHADIND_ALIGN_CONF
{
  SHADING_TBL_SPEC  cali;   // per unit calibration data
  SHADING_TBL_SPEC  golden; // golden reference
  SHADING_TBL_SPEC  input;  // golden shading other than reference
  SHADING_TBL_SPEC  output; // per unit compensated golden shading
  void				*working_buff_addr;
  unsigned int		working_buff_size;
} SHADIND_ALIGN_CONF;

typedef enum
{
    S_LSC_CONVERT_OK,						// Success to do shading table converstion
	E_LSC_CONVERT_BITS_OVERFLOW,			// overflow
    E_LSC_CONVERT_WRONG_INPUT,				// input info incorrect
	E_LSC_CONVERT_OUT_OF_WORKING_MEM		// working buffer size is not enough
}LSC_RESULT;

LSC_RESULT shading_transform(SHADIND_TRFM_CONF shading_conf);
LSC_RESULT shading_align_golden(SHADIND_ALIGN_CONF trans);

#endif
