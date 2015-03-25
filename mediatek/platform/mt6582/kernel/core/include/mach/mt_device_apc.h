#ifndef _MTK_DEVICE_APC_H
#define _MTK_DEVICE_APC_H


#define DEVAPC0_AO_BASE         0xF000E000
#define DEVAPC0_PD_BASE         0xF0207000


/******************************************************************************
*
 * REGISTER ADDRESS DEFINATION
 
******************************************************************************/
#define DEVAPC0_D0_APC_0		    ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0000))
#define DEVAPC0_D0_APC_1            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0004))
#define DEVAPC0_D0_APC_2            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0008))
#define DEVAPC0_D0_APC_3            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x000C))
#define DEVAPC0_D0_APC_4            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0010))
#define DEVAPC0_D0_APC_5            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0014))
#define DEVAPC0_D0_APC_6            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0018))
#define DEVAPC0_D0_APC_7            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x001C))
#define DEVAPC0_D1_APC_0            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0100))
#define DEVAPC0_D1_APC_1            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0104))
#define DEVAPC0_D1_APC_2            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0108))
#define DEVAPC0_D1_APC_3            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x010C))
#define DEVAPC0_D1_APC_4            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0110))
#define DEVAPC0_D1_APC_5            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0114))
#define DEVAPC0_D1_APC_6            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0118))
#define DEVAPC0_D1_APC_7            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x011C))
#define DEVAPC0_D2_APC_0            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0200))
#define DEVAPC0_D2_APC_1            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0204))
#define DEVAPC0_D2_APC_2            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0208))
#define DEVAPC0_D2_APC_3            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x020C))
#define DEVAPC0_D2_APC_4            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0210))
#define DEVAPC0_D2_APC_5            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0214))
#define DEVAPC0_D2_APC_6            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0218))
#define DEVAPC0_D2_APC_7            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x021C))
#define DEVAPC0_D3_APC_0            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0300))
#define DEVAPC0_D3_APC_1            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0304))
#define DEVAPC0_D3_APC_2            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0308))
#define DEVAPC0_D3_APC_3            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x030C))
#define DEVAPC0_D3_APC_4            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0310))
#define DEVAPC0_D3_APC_5            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0314))
#define DEVAPC0_D3_APC_6            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0318))
#define DEVAPC0_D3_APC_7            ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x031C))
#define DEVAPC0_MAS_DOM_0           ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0400))
#define DEVAPC0_MAS_DOM_1           ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0404))
#define DEVAPC0_MAS_SEC             ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0500))
#define DEVAPC0_APC_CON             ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0F00))
#define DEVAPC0_APC_LOCK_0          ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0F04))
#define DEVAPC0_APC_LOCK_1          ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0F08))
#define DEVAPC0_APC_LOCK_2          ((volatile unsigned int*)(DEVAPC0_AO_BASE+0x0F0C))



#define DEVAPC0_PD_APC_CON          ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0F00))
#define DEVAPC0_D0_VIO_MASK_0       ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0000))
#define DEVAPC0_D0_VIO_MASK_1       ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0004))
#define DEVAPC0_D0_VIO_MASK_2       ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0008))
#define DEVAPC0_D0_VIO_MASK_3       ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x000C))// legacy code
#define DEVAPC0_D0_VIO_STA_0        ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0400))
#define DEVAPC0_D0_VIO_STA_1        ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0404))
#define DEVAPC0_D0_VIO_STA_2        ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0408))
#define DEVAPC0_D0_VIO_STA_3        ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x040C)) 
#define DEVAPC0_VIO_DBG0            ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0900))
#define DEVAPC0_VIO_DBG1            ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0904))
#define DEVAPC0_DEC_ERR_CON         ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0F80))
#define DEVAPC0_DEC_ERR_ADDR        ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0F84))
#define DEVAPC0_DEC_ERR_ID          ((volatile unsigned int*)(DEVAPC0_PD_BASE+0x0F88))


/* DOMAIN_SETUP */
#define DOMAIN_AP						0
#define DOMAIN_MD1	    				1
#define DOMAIN_MD2						2
#define DOMAIN_MM                       3


#endif
