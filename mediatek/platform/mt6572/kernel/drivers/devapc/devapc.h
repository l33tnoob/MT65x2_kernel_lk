#ifndef _MTK_DEVICE_APC_SW_H
#define _MTK_DEVICE_APC_SW_H

/*
 * Define constants.
 */
#define DEVAPC_TAG              "DEVAPC"
#define DEVAPC_MAX_TIMEOUT      100

#define DEVAPC_ABORT_EMI        0x1


/*
 * Define enums.
 */
/* Domain index */
typedef enum
{
    E_DOM_AP=0,
    E_DOM_MD,
    E_DOM_CONN,
    E_MAX_DOM
} DEVAPC_DOM;

/* Access permission attribute */
typedef enum
{
    E_ATTR_L0=0,
    E_ATTR_L1,
    E_ATTR_L2,
    E_ATTR_L3,
    E_MAX_ATTR
} DEVAPC_ATTR;


/*
 * Define enums.
 */
typedef struct {
    int             device_num;
    bool            forbidden;
    DEVAPC_ATTR     d0_attr;
    DEVAPC_ATTR     d1_attr;
    DEVAPC_ATTR     d2_attr;
} DEVICE_INFO;


/*
 * Define all devices' attribute.
 */  
static DEVICE_INFO DEVAPC_Devices[] = {
#if 1
    {0,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {1,   TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {2,   TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {3,   TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {4,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {5,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {6,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {7,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {8,   TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {9,   TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},

    {10,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {11,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {12,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {13,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {14,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {15,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {16,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {17,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {18,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {19,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},

    {20,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {21,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {22,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {23,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {24,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {25,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {26,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {27,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {28,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {29,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},

    {30,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {31,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {32,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {33,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {34,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {35,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {36,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {37,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {38,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {39,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},

    {40,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {41,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {42,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {43,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {44,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {45,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {46,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {47,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {48,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {49,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},

    {50,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {51,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {52,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {53,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {54,  TRUE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {55,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {56,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {57,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {58,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {59,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},

    {60,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {61,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {62,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
    {63,  TRUE,  E_ATTR_L0, E_ATTR_L1, E_ATTR_L0},
#endif
    {-1,  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
};


/* Backup Design */
#if 0
/*
 * Define enums.
 */
typedef struct {
    const char      *device_name;
    bool            forbidden;
    DEVAPC_ATTR     d0_attr;
    DEVAPC_ATTR     d1_attr;
    DEVAPC_ATTR     d2_attr;
} DEVICE_INFO;


/*
 * Define all devices' attribute.
 */  
static DEVICE_INFO DEVAPC_Devices[] = {
    {"0",   FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"1",   FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"2",   FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"3",   FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"4",   FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"5",   FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"6",   FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"7",   FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"8",   FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"9",   FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},

    {"10",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"11",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"12",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"13",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"14",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"15",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"16",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"17",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"18",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"19",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},

    {"20",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"21",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"22",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"23",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"24",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"25",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"26",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"27",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"28",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"29",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},

    {"30",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"31",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"32",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"33",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"34",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"35",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"36",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"37",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"38",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"39",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},

    {"40",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"41",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"42",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"43",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"44",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"45",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"46",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"47",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"48",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"49",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},

    {"50",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"51",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"52",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"53",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"54",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"55",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"56",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"57",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"58",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"59",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},

    {"60",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"61",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"62",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
    {"63",  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},

    {NULL,  FALSE,  E_ATTR_L0, E_ATTR_L0, E_ATTR_L0},
};
#endif 

/* Original Design */
#if 0
/*
 * Define enums.
 */
typedef struct {
    const char      *device_name;
    bool            forbidden;
} DEVICE_INFO;


/*
 * Define all devices' attribute.
 */  
static DEVICE_INFO DEVAPC_Devices[] = {
    {"0",   FALSE},
    {"1",   FALSE},
    {"2",   FALSE},
    {"3",   FALSE},
    {"4",   FALSE},
    {"5",   FALSE},
    {"6",   FALSE},
    {"7",   FALSE},
    {"8",   FALSE},
    {"9",   FALSE},

    {"10",  FALSE},
    {"11",  FALSE},
    {"12",  FALSE},
    {"13",  FALSE},
    {"14",  FALSE},
    {"15",  FALSE},
    {"16",  FALSE},
    {"17",  FALSE},
    {"18",  FALSE},
    {"19",  FALSE},

    {"20",  FALSE},
    {"21",  FALSE},
    {"22",  FALSE},
    {"23",  FALSE},
    {"24",  FALSE},
    {"25",  FALSE},
    {"26",  FALSE},
    {"27",  FALSE},
    {"28",  FALSE},
    {"29",  FALSE},

    {"30",  FALSE},
    {"31",  FALSE},
    {"32",  FALSE},
    {"33",  FALSE},
    {"34",  FALSE},
    {"35",  FALSE},
    {"36",  FALSE},
    {"37",  FALSE},
    {"38",  FALSE},
    {"39",  FALSE},

    {"40",  FALSE},
    {"41",  FALSE},
    {"42",  FALSE},
    {"43",  FALSE},
    {"44",  FALSE},
    {"45",  FALSE},
    {"46",  FALSE},
    {"47",  FALSE},
    {"48",  FALSE},
    {"49",  FALSE},

    {"50",  FALSE},
    {"51",  FALSE},
    {"52",  FALSE},
    {"53",  FALSE},
    {"54",  FALSE},
    {"55",  FALSE},
    {"56",  FALSE},
    {"57",  FALSE},
    {"58",  FALSE},
    {"59",  FALSE},

    {"60",  FALSE},
    {"61",  FALSE},
    {"62",  FALSE},
    {"63",  FALSE},

    {NULL,  FALSE},
};
#endif 

/* From MT6589 */
#if 0
#define SET_SINGLE_MODULE(apcnum, domnum, index, module, permission_control)     \
 {                                                                               \
     mt65xx_reg_sync_writel(readl(DEVAPC##apcnum##_D##domnum##_APC_##index) & ~(0x3 << (2 * module)), DEVAPC##apcnum##_D##domnum##_APC_##index); \
     mt65xx_reg_sync_writel(readl(DEVAPC##apcnum##_D##domnum##_APC_##index) | (permission_control << (2 * module)),DEVAPC##apcnum##_D##domnum##_APC_##index); \
 }

#define UNMASK_SINGLE_MODULE_IRQ(apcnum, domnum, module_index)                  \
 {                                                                               \
     mt65xx_reg_sync_writel(readl(DEVAPC##apcnum##_D##domnum##_VIO_MASK) & ~(module_index),      \
         DEVAPC##apcnum##_D##domnum##_VIO_MASK);                                 \
 }

#define CLEAR_SINGLE_VIO_STA(apcnum, domnum, module_index)                     \
 {                                                                               \
     mt65xx_reg_sync_writel(readl(DEVAPC##apcnum##_D##domnum##_VIO_STA) | (module_index),        \
         DEVAPC##apcnum##_D##domnum##_VIO_STA);                                  \
 }
#endif

#endif
 
