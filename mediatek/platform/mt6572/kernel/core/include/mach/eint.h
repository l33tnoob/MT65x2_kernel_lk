#ifndef __EINT_H__
#define __EINT_H__

#include "mt_reg_base.h"

/*
 * Define hardware registers.
 */
#define EINT_STA_BASE           ((EINT_BASE + 0x000))
#define EINT_INTACK_BASE        ((EINT_BASE + 0x040))

#define EINT_MASK_BASE          ((EINT_BASE + 0x080))
#define EINT_MASK_SET_BASE      ((EINT_BASE + 0x0c0))
#define EINT_MASK_CLR_BASE      ((EINT_BASE + 0x100))

#define EINT_SENS_BASE          ((EINT_BASE + 0x140))
#define EINT_SENS_SET_BASE      ((EINT_BASE + 0x180))
#define EINT_SENS_CLR_BASE      ((EINT_BASE + 0x1c0))

#define EINT_SOFT_BASE          ((EINT_BASE + 0x200))
#define EINT_SOFT_SET_BASE      ((EINT_BASE + 0x240))
#define EINT_SOFT_CLR_BASE      ((EINT_BASE + 0x280))

#define EINT_POL_BASE           ((EINT_BASE + 0x300))
#define EINT_POL_SET_BASE       ((EINT_BASE + 0x340))
#define EINT_POL_CLR_BASE       ((EINT_BASE + 0x380))

#define EINT_D0_EN_BASE         ((EINT_BASE + 0x400))

#define EINT_DBNC_BASE          ((EINT_BASE + 0x500))
#define EINT_DBNC_SET_BASE      ((EINT_BASE + 0x600))
#define EINT_DBNC_CLR_BASE      ((EINT_BASE + 0x700))

#define EINT_EEVT_BASE          ((EINT_BASE + 0x900))

#define EINT_EMUL_BASE          ((EINT_BASE + 0xF00))


/*
 * Define control bits.
 */
#define EINT_DBNC_SET_DBNC_BITS     (4)
#define EINT_DBNC_CLR_DBNC_BITS     (4)
#define EINT_DBNC_SET_EN_BITS       (0)
#define EINT_DBNC_CLR_EN_BITS       (0)
#define EINT_DBNC_SET_RST_BITS      (1)

#define EINT_DBNC_SET_EN            (0x1)
#define EINT_DBNC_CLR_EN            (0x1)

#define EINT_DBNC_EN_BIT            (0x1)
#define EINT_DBNC_RST_BIT           (0x1)

#define EINT_DBNC_0_MS              (0x7)
#define EINT_DBNC                   (0x7)


#if 0
/*
 * Define the default value.
 */
#define EINT_STA_DEFAULT        0x00000000
#define EINT_INTACK_DEFAULT	    0x00000000
#define EINT_MASK_DEFAULT       0x03FFFFFF
#define EINT_MASK_SET_DEFAULT   0x00000000
#define EINT_MASK_CLR_DEFAULT   0x00000000
#define EINT_SENS_DEFAULT       0x03FFFFFF
#define EINT_SENS_SET_DEFAULT   0x00000000
#define EINT_SENS_CLR_DEFAULT   0x00000000
#define EINT_SOFT_DEFAULT       0x00000000
#define EINT_SOFT_SET_DEFAULT   0x00000000
#define EINT_SOFT_CLR_DEFAULT   0x00000000
#define EINT_POL_DEFAULT        0x00000000
#define EINT_POL_SET_DEFAULT    0x00000000
#define EINT_POL_CLR_DEFAULT    0x00000000
#define EINT_D0EN_DEFAULT       0x00000000
#define EINT_DBNC_DEFAULT(n)    0x00000000
#define EINT_EEVT_DEFAULT       0x00000001
#endif


/*
 * Define constants.
 */
#define EINT_AP_MAXNUMBER       25
#define EINT_MAX_CHANNEL        26
#define MAX_HW_DEBOUNCE_CNT     26

#define EINT_CTRL_REG_NUMBER    ((EINT_MAX_CHANNEL + 31) / 32)

#define MT_EINT_POL_NEG (0)
#define MT_EINT_POL_POS (1)
#define MT65XX_EINT_POL_NEG     (MT_EINT_POL_NEG)
#define MT65XX_EINT_POL_POS     (MT_EINT_POL_POS)

#define EINTF_TRIGGER_RISING     0x00000001
#define EINTF_TRIGGER_FALLING    0x00000002
#define EINTF_TRIGGER_HIGH       0x00000004
#define EINTF_TRIGGER_LOW        0x00000008


/*
 * Define function prototypes.
 */
extern void mt_eint_mask(unsigned int eint_num);
extern void mt_eint_unmask(unsigned int eint_num);
extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol);
extern unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
extern void mt_eint_print_status(void);
extern unsigned int mt_eint_get_status(unsigned int eint_num);
extern int get_eint_attribute(char *name, unsigned int name_len, unsigned int type, char *result, unsigned int *len);

#if 1
extern void mt65xx_eint_mask(unsigned int eint_num);
extern void mt65xx_eint_unmask(unsigned int eint_num);
extern void mt65xx_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern void mt65xx_eint_set_polarity(unsigned int eint_num, unsigned int pol);
extern unsigned int mt65xx_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt65xx_eint_registration(unsigned int eint_num, unsigned int is_deb_en, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);
#endif

#endif  /*!__EINT_H__ */
