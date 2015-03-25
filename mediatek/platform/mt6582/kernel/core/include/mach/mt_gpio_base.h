#ifndef _MT_GPIO_BASE_H_
#define _MT_GPIO_BASE_H_

#include <mach/sync_write.h>

#define GPIO_WR32(addr, data)   mt65xx_reg_sync_writel(data, addr)
#define GPIO_RD32(addr)         __raw_readl(addr)
//#define GPIO_SET_BITS(BIT,REG)   ((*(volatile unsigned long*)(REG)) = (unsigned long)(BIT))
//#define GPIO_CLR_BITS(BIT,REG)   ((*(volatile unsigned long*)(REG)) &= ~((unsigned long)(BIT)))
#define GPIO_SET_BITS(BIT,REG)   GPIO_WR32(REG, (unsigned long)(BIT))
#define GPIO_CLR_BITS(BIT,REG)   GPIO_WR32(REG,GPIO_RD32(REG) & ~((unsigned long)(BIT)))

#define MAX_GPIO_REG_BITS      16
#define MAX_GPIO_MODE_PER_REG  5
#define GPIO_MODE_BITS         3 

#define MT_GPIO_BASE_START 0
typedef enum GPIO_PIN
{    
    GPIO_UNSUPPORTED = -1,    
	GPIO0 = MT_GPIO_BASE_START,
    GPIO1  , GPIO2  , GPIO3  , GPIO4  , GPIO5  , GPIO6  , GPIO7  , 
    GPIO8  , GPIO9  , GPIO10 , GPIO11 , GPIO12 , GPIO13 , GPIO14 , GPIO15 , 
    GPIO16 , GPIO17 , GPIO18 , GPIO19 , GPIO20 , GPIO21 , GPIO22 , GPIO23 , 
    GPIO24 , GPIO25 , GPIO26 , GPIO27 , GPIO28 , GPIO29 , GPIO30 , GPIO31 , 
    GPIO32 , GPIO33 , GPIO34 , GPIO35 , GPIO36 , GPIO37 , GPIO38 , GPIO39 , 
    GPIO40 , GPIO41 , GPIO42 , GPIO43 , GPIO44 , GPIO45 , GPIO46 , GPIO47 , 
    GPIO48 , GPIO49 , GPIO50 , GPIO51 , GPIO52 , GPIO53 , GPIO54 , GPIO55 , 
    GPIO56 , GPIO57 , GPIO58 , GPIO59 , GPIO60 , GPIO61 , GPIO62 , GPIO63 , 
    GPIO64 , GPIO65 , GPIO66 , GPIO67 , GPIO68 , GPIO69 , GPIO70 , GPIO71 , 
    GPIO72 , GPIO73 , GPIO74 , GPIO75 , GPIO76 , GPIO77 , GPIO78 , GPIO79 , 
    GPIO80 , GPIO81 , GPIO82 , GPIO83 , GPIO84 , GPIO85 , GPIO86 , GPIO87 , 
    GPIO88 , GPIO89 , GPIO90 , GPIO91 , GPIO92 , GPIO93 , GPIO94 , GPIO95 , 
    GPIO96 , GPIO97 , GPIO98 , GPIO99 , GPIO100, GPIO101, GPIO102, GPIO103, 
    GPIO104, GPIO105, GPIO106, GPIO107, GPIO108, GPIO109, GPIO110, GPIO111, 
    GPIO112, GPIO113, GPIO114, GPIO115, GPIO116, GPIO117, GPIO118, GPIO119, 
    GPIO120, GPIO121, GPIO122, GPIO123, GPIO124, GPIO125, GPIO126, GPIO127, 
    GPIO128, GPIO129, GPIO130, GPIO131, GPIO132, GPIO133, GPIO134, GPIO135, 
    GPIO136, GPIO137, GPIO138, GPIO139, GPIO140, GPIO141, GPIO142, GPIO143, 
    GPIO144, GPIO145, GPIO146, GPIO147, GPIO148, GPIO149, GPIO150, GPIO151, 
    GPIO152, GPIO153, GPIO154, GPIO155, GPIO156, GPIO157, GPIO158, GPIO159, 
    GPIO160, GPIO161, GPIO162, GPIO163, GPIO164, GPIO165, GPIO166, GPIO167, 
    GPIO168, 
    MT_GPIO_BASE_MAX
}GPIO_PIN;

/*----------------------------------------------------------------------------*/
typedef struct {        /*FIXME: check GPIO spec*/
    unsigned short val;        
    unsigned short _align1;
    unsigned short set;
    unsigned short _align2;
    unsigned short rst;
    unsigned short _align3[3];
} VAL_REGS;
/*----------------------------------------------------------------------------*/
typedef struct {
    VAL_REGS    dir[11];            /*0x0000 ~ 0x00AF: 176 bytes*/
    unsigned char       rsv00[80];  /*0x00B0 ~ 0x00FF: 	80 bytes*/
    VAL_REGS    pullen[11];         /*0x0100 ~ 0x01AF: 176 bytes*/
    unsigned char       rsv01[80];  /*0x01B0 ~ 0x01FF: 	80 bytes*/
    VAL_REGS    pullsel[11];        /*0x0200 ~ 0x02AF: 176 bytes*/
    unsigned char       rsv02[80];  /*0x02B0 ~ 0x02FF: 	80 bytes*/
    unsigned char       rsv03[256]; /*0x0300 ~ 0x03FF: 256 bytes*/
    VAL_REGS    dout[11];           /*0x0400 ~ 0x04AF: 176 bytes*/
    unsigned char       rsv04[80];  /*0x04B0 ~ 0x04FF: 	80 bytes*/
    VAL_REGS    din[11];            /*0x0500 ~ 0x05AF: 176 bytes*/
    unsigned char       rsv05[80];	/*0x05B0 ~ 0x05FF: 	80 bytes*/
    VAL_REGS    mode[36];           /*0x0600 ~ 0x083F: 576 bytes*/  
	unsigned char		rsv06[192];	/*0x0840 ~ 0x08FF: 192 bytes*/
	VAL_REGS    ies[2];            	/*0x0900 ~ 0x091F: 	32 bytes*/
    VAL_REGS    smt[2];        		/*0x0920 ~ 0x093F: 	32 bytes*/ 
	unsigned char		rsv07[192];	/*0x0940 ~ 0x09FF: 192 bytes*/
	VAL_REGS    tdsel[8];        	/*0x0A00 ~ 0x0A7F: 128 bytes*/ 
	VAL_REGS    rdsel[8];        	/*0x0A80 ~ 0x0AFF: 128 bytes*/ 
	VAL_REGS    drv_mode[8];        /*0x0B00 ~ 0x0B7F: 128 bytes*/ 
	unsigned char		rsv08[128];	/*0x0B80 ~ 0x0BFF: 128 bytes*/
	VAL_REGS    msdc0_ctrl[4];      /*0x0C00 ~ 0x0C3F: 	64 bytes*/ 
	VAL_REGS    msdc1_ctrl[4];      /*0x0C40 ~ 0x0C7F: 	64 bytes*/ 
	VAL_REGS    msdc2_ctrl[4];      /*0x0C80 ~ 0x0CBF: 	64 bytes*/
	VAL_REGS    exmd_ctrl[1];       /*0x0CC0 ~ 0x0CCF: 	16 bytes*/
	VAL_REGS    bpi_ctrl[1];       	/*0x0CD0 ~ 0x0CDF: 	16 bytes*/
	VAL_REGS    kpad_ctrl[2];       /*0x0CE0 ~ 0x0CFF: 	32 bytes*/
} GPIO_REGS;

#endif //_MT_GPIO_BASE_H_
