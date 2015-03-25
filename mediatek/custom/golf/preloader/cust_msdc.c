#include "cust_msdc.h"

#ifdef FPGA_PLATFORM
struct msdc_cust msdc_cap = {
    0, /* host clock source             */
    MSDC_SMPL_RISING,   /* command latch edge            */
    MSDC_SMPL_RISING,   /* data latch edge               */
    2,                  /* clock pad driving             */
    2,                  /* command pad driving           */
    2,                  /* data pad driving              */
    4,                  /* data pins                     */
    0,                  /* data address offset           */

    /* hardware capability flags     */
    MSDC_HIGHSPEED,//|MSDC_DDR,
};
#else
struct msdc_cust msdc_cap = {
    MSDC_CLKSRC_200MHZ, /* host clock source             */
    MSDC_SMPL_RISING,   /* command latch edge            */
    MSDC_SMPL_RISING,   /* data latch edge               */
    2,                  /* clock pad driving             */
    2,                  /* command pad driving           */
    2,                  /* data pad driving              */
    8,                  /* data pins                     */
    0,                  /* data address offset           */

    /* hardware capability flags     */
    MSDC_HIGHSPEED|MSDC_DDR,
};

#endif


