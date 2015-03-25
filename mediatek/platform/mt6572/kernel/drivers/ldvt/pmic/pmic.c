#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/cnt32_to_63.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>

//#include <mach/timer.h>
#include <mach/irqs.h>
#include <asm/uaccess.h>

#include <mach/pmic_mt6323_sw.h>
#include <mach/upmu_common.h>
#include <mach/upmu_hw.h>
#include <mach/mt_pm_ldo.h>
#include <linux/time.h> 


#define pmicname	"uvvp_pmic"

/*
	This is a Kernel driver used by user space. 
	This driver will using the interface of the PMIC production driver.  
	We implement some IOCTLs: 
*/
#define	UVVP_PMIC_GET_VERSION				_IOW('k', 0, int)
#define	UVVP_PMIC_GET_PCHR_CHRDET			_IOW('k', 1, int)

#define	UVVP_PMIC_VERIFY_DEFAULT_VALUE		_IOW('k', 2, int)
#define	UVVP_PMIC_TOP_WR_1                  _IOW('k', 3, int)
#define	UVVP_PMIC_TOP_WR_2                  _IOW('k', 4, int)

#define	UVVP_PMIC_AUXADC_611                _IOW('k', 5, int)
#define	UVVP_PMIC_AUXADC_612                _IOW('k', 6, int)
#define	UVVP_PMIC_AUXADC_621                _IOW('k', 7, int)
#define	UVVP_PMIC_AUXADC_622                _IOW('k', 8, int)

#define	UVVP_PMIC_INT_451                   _IOW('k', 9, int)
#define	UVVP_PMIC_INT_461                   _IOW('k', 10, int)
#define	UVVP_PMIC_INT_462                   _IOW('k', 11, int)
#define	UVVP_PMIC_INT_463                   _IOW('k', 12, int)
#define	UVVP_PMIC_INT_464                   _IOW('k', 13, int)
#define	UVVP_PMIC_INT_465                   _IOW('k', 14, int)
#define	UVVP_PMIC_INT_466                   _IOW('k', 15, int)
#define	UVVP_PMIC_INT_467                   _IOW('k', 16, int)
#define	UVVP_PMIC_INT_468                   _IOW('k', 17, int)

#define	UVVP_PMIC_LDO_ON_OFF_0              _IOW('k', 18, int)
#define	UVVP_PMIC_LDO_ON_OFF_1              _IOW('k', 19, int)
#define	UVVP_PMIC_LDO_ON_OFF_2              _IOW('k', 20, int)
#define	UVVP_PMIC_LDO_ON_OFF_3              _IOW('k', 21, int)
#define	UVVP_PMIC_LDO_ON_OFF_4              _IOW('k', 22, int)
#define	UVVP_PMIC_LDO_ON_OFF_5              _IOW('k', 23, int)
#define	UVVP_PMIC_LDO_ON_OFF_6              _IOW('k', 24, int)
#define	UVVP_PMIC_LDO_ON_OFF_7              _IOW('k', 25, int)
#define	UVVP_PMIC_LDO_ON_OFF_8              _IOW('k', 26, int)
#define	UVVP_PMIC_LDO_ON_OFF_9              _IOW('k', 27, int)
#define	UVVP_PMIC_LDO_ON_OFF_10             _IOW('k', 28, int)
#define	UVVP_PMIC_LDO_ON_OFF_11             _IOW('k', 29, int)
#define	UVVP_PMIC_LDO_ON_OFF_12             _IOW('k', 30, int)
#define	UVVP_PMIC_LDO_ON_OFF_13             _IOW('k', 31, int)
#define	UVVP_PMIC_LDO_ON_OFF_14             _IOW('k', 32, int)
#define	UVVP_PMIC_LDO_ON_OFF_15             _IOW('k', 33, int)
#define	UVVP_PMIC_LDO_ON_OFF_16             _IOW('k', 34, int)
#define	UVVP_PMIC_LDO_ON_OFF_17             _IOW('k', 35, int)
#define	UVVP_PMIC_LDO_ON_OFF_18             _IOW('k', 36, int)
#define	UVVP_PMIC_LDO_ON_OFF_19             _IOW('k', 37, int)
#define	UVVP_PMIC_LDO_ON_OFF_20             _IOW('k', 38, int)
#define	UVVP_PMIC_LDO_ON_OFF_21             _IOW('k', 39, int)
#define	UVVP_PMIC_LDO_ON_OFF_22             _IOW('k', 40, int)
#define	UVVP_PMIC_LDO_ON_OFF_23             _IOW('k', 41, int)

#define	UVVP_PMIC_LDO_VOSEL_0              _IOW('k', 42, int)
#define	UVVP_PMIC_LDO_VOSEL_1              _IOW('k', 43, int)
#define	UVVP_PMIC_LDO_VOSEL_2              _IOW('k', 44, int)
#define	UVVP_PMIC_LDO_VOSEL_3              _IOW('k', 45, int)
#define	UVVP_PMIC_LDO_VOSEL_4              _IOW('k', 46, int)
#define	UVVP_PMIC_LDO_VOSEL_5              _IOW('k', 47, int)
#define	UVVP_PMIC_LDO_VOSEL_6              _IOW('k', 48, int)
#define	UVVP_PMIC_LDO_VOSEL_7              _IOW('k', 49, int)
#define	UVVP_PMIC_LDO_VOSEL_8              _IOW('k', 50, int)
#define	UVVP_PMIC_LDO_VOSEL_9              _IOW('k', 51, int)
#define	UVVP_PMIC_LDO_VOSEL_10             _IOW('k', 52, int)
#define	UVVP_PMIC_LDO_VOSEL_11             _IOW('k', 53, int)
#define	UVVP_PMIC_LDO_VOSEL_12             _IOW('k', 54, int)
#define	UVVP_PMIC_LDO_VOSEL_13             _IOW('k', 55, int)
#define	UVVP_PMIC_LDO_VOSEL_14             _IOW('k', 56, int)
#define	UVVP_PMIC_LDO_VOSEL_15             _IOW('k', 57, int)
#define	UVVP_PMIC_LDO_VOSEL_16             _IOW('k', 58, int)
#define	UVVP_PMIC_LDO_VOSEL_17             _IOW('k', 59, int)
#define	UVVP_PMIC_LDO_VOSEL_18             _IOW('k', 60, int)
#define	UVVP_PMIC_LDO_VOSEL_19             _IOW('k', 61, int)
#define	UVVP_PMIC_LDO_VOSEL_20             _IOW('k', 62, int)
#define	UVVP_PMIC_LDO_VOSEL_21             _IOW('k', 63, int)
#define	UVVP_PMIC_LDO_VOSEL_22             _IOW('k', 64, int)
#define	UVVP_PMIC_LDO_VOSEL_23             _IOW('k', 65, int)

#define	UVVP_PMIC_LDO_CAL_0              _IOW('k', 66, int)
#define	UVVP_PMIC_LDO_CAL_1              _IOW('k', 67, int)
#define	UVVP_PMIC_LDO_CAL_2              _IOW('k', 68, int)
#define	UVVP_PMIC_LDO_CAL_3              _IOW('k', 69, int)
#define	UVVP_PMIC_LDO_CAL_4              _IOW('k', 70, int)
#define	UVVP_PMIC_LDO_CAL_5              _IOW('k', 71, int)
#define	UVVP_PMIC_LDO_CAL_6              _IOW('k', 72, int)
#define	UVVP_PMIC_LDO_CAL_7              _IOW('k', 73, int)
#define	UVVP_PMIC_LDO_CAL_8              _IOW('k', 74, int)
#define	UVVP_PMIC_LDO_CAL_9              _IOW('k', 75, int)
#define	UVVP_PMIC_LDO_CAL_10             _IOW('k', 76, int)
#define	UVVP_PMIC_LDO_CAL_11             _IOW('k', 77, int)
#define	UVVP_PMIC_LDO_CAL_12             _IOW('k', 78, int)
#define	UVVP_PMIC_LDO_CAL_13             _IOW('k', 79, int)
#define	UVVP_PMIC_LDO_CAL_14             _IOW('k', 80, int)
#define	UVVP_PMIC_LDO_CAL_15             _IOW('k', 81, int)
#define	UVVP_PMIC_LDO_CAL_16             _IOW('k', 82, int)
#define	UVVP_PMIC_LDO_CAL_17             _IOW('k', 83, int)
#define	UVVP_PMIC_LDO_CAL_18             _IOW('k', 84, int)
#define	UVVP_PMIC_LDO_CAL_19             _IOW('k', 85, int)
#define	UVVP_PMIC_LDO_CAL_20             _IOW('k', 86, int)
#define	UVVP_PMIC_LDO_CAL_21             _IOW('k', 87, int)
#define	UVVP_PMIC_LDO_CAL_22             _IOW('k', 88, int)
#define	UVVP_PMIC_LDO_CAL_23             _IOW('k', 89, int)

#define	UVVP_PMIC_BUCK_ON_OFF_0              _IOW('k', 90, int)
#define	UVVP_PMIC_BUCK_ON_OFF_1              _IOW('k', 91, int)
#define	UVVP_PMIC_BUCK_ON_OFF_2              _IOW('k', 92, int)
#define	UVVP_PMIC_BUCK_ON_OFF_3              _IOW('k', 93, int)
#define	UVVP_PMIC_BUCK_ON_OFF_4              _IOW('k', 94, int)
#define	UVVP_PMIC_BUCK_ON_OFF_5              _IOW('k', 95, int)
#define	UVVP_PMIC_BUCK_ON_OFF_6              _IOW('k', 96, int)
#define	UVVP_PMIC_BUCK_ON_OFF_7              _IOW('k', 97, int)

#define	UVVP_PMIC_BUCK_VOSEL_0              _IOW('k', 98, int)
#define	UVVP_PMIC_BUCK_VOSEL_1              _IOW('k', 99, int)
#define	UVVP_PMIC_BUCK_VOSEL_2              _IOW('k', 100, int)
#define	UVVP_PMIC_BUCK_VOSEL_3              _IOW('k', 101, int)
#define	UVVP_PMIC_BUCK_VOSEL_4              _IOW('k', 102, int)
#define	UVVP_PMIC_BUCK_VOSEL_5              _IOW('k', 103, int)
#define	UVVP_PMIC_BUCK_VOSEL_6              _IOW('k', 104, int)
#define	UVVP_PMIC_BUCK_VOSEL_7              _IOW('k', 105, int)

#define	UVVP_PMIC_BUCK_DLC_0              _IOW('k', 106, int)
#define	UVVP_PMIC_BUCK_DLC_1              _IOW('k', 107, int)
#define	UVVP_PMIC_BUCK_DLC_2              _IOW('k', 108, int)
#define	UVVP_PMIC_BUCK_DLC_3              _IOW('k', 109, int)
#define	UVVP_PMIC_BUCK_DLC_4              _IOW('k', 110, int)
#define	UVVP_PMIC_BUCK_DLC_5              _IOW('k', 111, int)
#define	UVVP_PMIC_BUCK_DLC_6              _IOW('k', 112, int)
#define	UVVP_PMIC_BUCK_DLC_7              _IOW('k', 113, int)

#define	UVVP_PMIC_BUCK_BURST_0              _IOW('k', 114, int)
#define	UVVP_PMIC_BUCK_BURST_1              _IOW('k', 115, int)
#define	UVVP_PMIC_BUCK_BURST_2              _IOW('k', 116, int)
#define	UVVP_PMIC_BUCK_BURST_3              _IOW('k', 117, int)
#define	UVVP_PMIC_BUCK_BURST_4              _IOW('k', 118, int)
#define	UVVP_PMIC_BUCK_BURST_5              _IOW('k', 119, int)
#define	UVVP_PMIC_BUCK_BURST_6              _IOW('k', 120, int)
#define	UVVP_PMIC_BUCK_BURST_7              _IOW('k', 121, int)

#define	UVVP_PMIC_AUXADC_111              _IOW('k', 130, int)
#define	UVVP_PMIC_AUXADC_131              _IOW('k', 131, int)
#define	UVVP_PMIC_AUXADC_132              _IOW('k', 132, int)
#define	UVVP_PMIC_AUXADC_133              _IOW('k', 133, int)
#define	UVVP_PMIC_AUXADC_134              _IOW('k', 134, int)
#define	UVVP_PMIC_AUXADC_135              _IOW('k', 135, int)
#define	UVVP_PMIC_AUXADC_141              _IOW('k', 136, int)
#define	UVVP_PMIC_AUXADC_211              _IOW('k', 137, int)
#define	UVVP_PMIC_AUXADC_212              _IOW('k', 138, int)
#define	UVVP_PMIC_AUXADC_213              _IOW('k', 139, int)
#define	UVVP_PMIC_AUXADC_214              _IOW('k', 140, int)
#define	UVVP_PMIC_AUXADC_215              _IOW('k', 141, int)
#define	UVVP_PMIC_AUXADC_216              _IOW('k', 142, int)
#define	UVVP_PMIC_AUXADC_321              _IOW('k', 143, int)

#define	UVVP_PMIC_DUMP_MEMORY           _IOW('k', 150, int)




/*Define for test*/
#define PMIC6320_E1_CID_CODE    0x1020

/*Externs*/
extern int PMIC_IMM_GetOneChannelValue(int dwChannel, int deCount, int trimd);

static kal_uint32 aPMURegBeg_bank0[273][2]= {  /* Register , reset val*/
  {0x003C, 0x0000},
  {0x003E, 0x0000},
  {0x0040, 0x4001},
  {0x0042, 0x0000},
  {0x0044, 0x0000},
  {0x0046, 0x0000},
  {0x0048, 0x0000},
  {0x004A, 0x0000},
  {0x004C, 0x0000},
  {0x004E, 0x0000},
  {0x0050, 0x0000},
  {0x0052, 0x0000},
  {0x0054, 0x0000},
  {0x0056, 0x0014},
  {0x005E, 0x0000},
  {0x0060, 0x4531},
  {0x0062, 0x0000},
  {0x0064, 0x2000},
  {0x0066, 0x0000},
  {0x0068, 0x0000},
  {0x006A, 0x0000},
  {0x0160, 0x0F30},
  {0x0166, 0x0001},
  {0x016C, 0x0000},
  {0x0172, 0x0000},
  {0x0174, 0x0000},
  {0x0176, 0x0000},
  {0x0178, 0x0000},
  {0x017A, 0x0001},
  {0x017C, 0x0000},
  {0x017E, 0x0000},
  {0x0180, 0x0010},
  {0x0182, 0x0000},
  {0x0184, 0x0000},
  {0x0186, 0x0000},
  {0x0188, 0x0000},
  {0x0204, 0xB100},
  {0x0206, 0x110A},
  {0x0208, 0xC10B},
  {0x020A, 0x0000},
  {0x020C, 0x0000},
  {0x020E, 0x0383},
  {0x0210, 0x0200},
  {0x0212, 0x0003},
  {0x0214, 0x00F0},
  {0x0216, 0x0000},
  {0x021A, 0x3001},
  {0x021C, 0x0505},
  {0x021E, 0x0048},
  {0x0220, 0x0048},
  {0x0222, 0x0048},
  {0x0224, 0x0048},
  {0x0226, 0x1111},
  {0x0228, 0x3333},
  {0x022A, 0x3333},
  {0x0230, 0x0000},
  {0x0232, 0x0000},
  {0x0234, 0x0383},
  {0x0236, 0x0200},
  {0x0238, 0x0003},
  {0x023A, 0x00F0},
  {0x023C, 0x0000},
  {0x0240, 0x3001},
  {0x0242, 0x0505},
  {0x0244, 0x0040},
  {0x0246, 0x0040},
  {0x0248, 0x0040},
  {0x024A, 0x0040},
  {0x024C, 0x1111},
  {0x024E, 0x3333},
  {0x0250, 0x3333},
  {0x0256, 0x0001},
  {0x0300, 0x0000},
  {0x0302, 0x0000},
  {0x0304, 0x0200},
  {0x0306, 0x0000},
  {0x0308, 0x0000},
  {0x030A, 0x0000},
  {0x030E, 0x0000},
  {0x0310, 0x0101},
  {0x0312, 0x0000},
  {0x0314, 0x0000},
  {0x0316, 0x0000},
  {0x0318, 0x0000},
  {0x031C, 0x0000},
  {0x0320, 0x0000},
  {0x0322, 0x0000},
  {0x0324, 0x0071},
  {0x0326, 0x0E00},
  {0x0328, 0x2E14},
  {0x032A, 0x0000},
  {0x032E, 0x0000},
  {0x0330, 0x0000},
  {0x0332, 0x0000},
  {0x0334, 0x0000},
  {0x0336, 0x0000},
  {0x0338, 0x0000},
  {0x033A, 0x0000},
  {0x033C, 0x0000},
  {0x033E, 0x0000},
  {0x0340, 0x0000},
  {0x0342, 0x0000},
  {0x0344, 0x0000},
  {0x0346, 0x0000},
  {0x0348, 0x0000},
  {0x034A, 0x0000},
  {0x034C, 0x0000},
  {0x034E, 0x0000},
  {0x0350, 0x0000},
  {0x0352, 0x0000},
  {0x0354, 0x0000},
  {0x0356, 0x0000},
  {0x0400, 0x0000},
  {0x0402, 0x0400},
  {0x0404, 0x4000},
  {0x0406, 0x0000},
  {0x0408, 0x0000},
  {0x040A, 0x0000},
  {0x040C, 0x0000},
  {0x040E, 0x0010},
  {0x0410, 0x0004},
  {0x0412, 0x0064},
  {0x0414, 0x0000},
  {0x0416, 0x001C},
  {0x0418, 0x0000},
  {0x041A, 0x0044},
  {0x041C, 0x0000},
  {0x041E, 0x0000},
  {0x0420, 0x0000},
  {0x0500, 0x4000},
  {0x0502, 0x4000},
  {0x0504, 0x1000},
  {0x0506, 0x4000},
  {0x0508, 0x4000},
  {0x050A, 0x0000},
  {0x050C, 0x0000},
  {0x050E, 0x0000},
  {0x0510, 0x0001},
  {0x0512, 0x0000},
  {0x0514, 0x0000},
  {0x0516, 0x0000},
  {0x0518, 0x0000},
  {0x051A, 0x0100},
  {0x051C, 0x0000},
  {0x051E, 0x0000},
  {0x0520, 0x0000},
  {0x0522, 0x0000},
  {0x0524, 0x0000},
  {0x0526, 0x0010},
  {0x0528, 0x0010},
  {0x052A, 0x00D1},
  {0x052C, 0x01C1},
  {0x052E, 0x03C9},
  {0x0530, 0x00A1},
  {0x0532, 0x0081},
  {0x0534, 0x0001},
  {0x0536, 0x0000},
  {0x0538, 0x0081},
  {0x053A, 0x0000},
  {0x053C, 0x0001},
  {0x053E, 0x0001},
  {0x0540, 0x0000},
  {0x0542, 0x0000},
  {0x0544, 0x0101},
  {0x0546, 0x0000},
  {0x0548, 0x0000},
  {0x054A, 0x0000},
  {0x054C, 0x0000},
  {0x054E, 0x0000},
  {0x0550, 0x0001},
  {0x0552, 0x4000},
  {0x0556, 0x4000},
  {0x0558, 0x0001},
  {0x055A, 0x0000},
  {0x055C, 0x0001},
  {0x055E, 0x0000},
  {0x0560, 0x0001},
  {0x0600, 0x0000},
  {0x0602, 0x0000},
  {0x0604, 0x0001},
  {0x0606, 0x0000},
  {0x0608, 0x0000},
  {0x060A, 0x0000},
  {0x060C, 0x0000},
  {0x060E, 0x0000},
  {0x0610, 0x0000},
  {0x0612, 0x0000},
  {0x0614, 0x0000},
  {0x0616, 0x0000},
  {0x0618, 0x0000},
  {0x061A, 0x0000},
  {0x061C, 0x0000},
  {0x061E, 0x0000},
  {0x0620, 0x0000},
  {0x0622, 0x0000},
  {0x0624, 0x0000},
  {0x063E, 0x0000},
  {0x0640, 0x0000},
  {0x0642, 0x0000},
  {0x0644, 0x0788},
  {0x0646, 0x0600},
  {0x0700, 0x6010},
  {0x0702, 0x0100},
  {0x0704, 0x00C0},
  {0x0706, 0x0000},
  {0x0708, 0x0000},
  {0x070A, 0x1100},
  {0x070C, 0x1C32},
  {0x070E, 0x0000},
  {0x0710, 0x0200},
  {0x0712, 0x0018},
  {0x0714, 0x0000},
  {0x0716, 0x0000},
  {0x0718, 0x0000},
  {0x071A, 0x0000},
  {0x071C, 0x0000},
  {0x071E, 0x0000},
  {0x0720, 0x0000},
  {0x0722, 0x0000},
  {0x0724, 0x0000},
  {0x0726, 0x0000},
  {0x0728, 0x0000},
  {0x072A, 0x0000},
  {0x072C, 0x0000},
  {0x072E, 0x0000},
  {0x0730, 0x0000},
  {0x0732, 0x0000},
  {0x0734, 0x0000},
  {0x0736, 0x0000},
  {0x0738, 0x0000},
  {0x073A, 0x0000},
  {0x073C, 0x0000},
  {0x073E, 0x0000},
  {0x0740, 0x0000},
  {0x0742, 0x0000},
  {0x0744, 0x0000},
  {0x0746, 0x0000},
  {0x0748, 0x0000},
  {0x074A, 0x0000},
  {0x074C, 0x8000},
  {0x074E, 0x8000},
  {0x0750, 0x0000},
  {0x0752, 0x0000},
  {0x0754, 0x0020},
  {0x0756, 0x0042},
  {0x0758, 0x0000},
  {0x0766, 0x01A0},
  {0x0768, 0x0000},
  {0x076A, 0x0000},
  {0x076C, 0x0000},
  {0x076E, 0x0000},
  {0x0770, 0x0000},
  {0x0772, 0x0012},
  {0x0774, 0x0000},
  {0x0776, 0x0000},
  {0x0778, 0x0000},
  {0x077A, 0x0010},
  {0x077C, 0x0000},
  {0x077E, 0x0000},
  {0x0780, 0x0000},
  {0x0782, 0x0000},
  {0x0784, 0x0101},
  {0x0786, 0x0010},
  {0x0788, 0x0010},
  {0x078A, 0x0010},
  {0x078C, 0x0010},
  {0x078E, 0x0333},
  {0x0790, 0x0000},
  {0x0792, 0x0000},
  {0x0794, 0x00FF},
  {0x0796, 0x0004},
  {0x0798, 0x0000},
  {0x079A, 0x0000}
};

static kal_uint32 aPMURegBeg_mask[273] = { /* mask*/
  0x007F,
  0x001F,
  0x7037,
  0x000F,
  0x0003,
  0xBFFF,
  0x0FFF,
  0x0030,
  0x7F03,
  0x00F0,
  0x0003,
  0x330D,
  0x7F00,
  0x07FF,
  0x0F37,
  0x7FFF,
  0x7FFF,
  0xFFFF,
  0x0777,
  0x0077,
  0xAAAB,
  0x0FFF,
  0x00FF,
  0x009F,
  0x0000,
  0x0000,
  0x0003,
  0x0003,
  0x0003,
  0x001F,
  0x001F,
  0x001F,
  0x8007,
  0xFFFF,
  0x0000,
  0x0001,
  0xF1FF,
  0xF1FF,
  0xF1FF,
  0xFFFF,
  0x7FFF,
  0xC3F3,
  0x030F,
  0x0033,
  0x00FF,
  0x000F,
  0x0001,
  0xFFFF,
  0x007F,
  0x007F,
  0x007F,
  0x0000,
  0x0333,
  0x0333,
  0x0333,
  0x0D73,
  0x7FFF,
  0xC3F3,
  0x030F,
  0x0033,
  0x00FF,
  0x000F,
  0x0001,
  0xFFFF,
  0x007F,
  0x007F,
  0x007F,
  0x0000,
  0x0333,
  0x0333,
  0x0333,
  0x0D73,
  0x003F,
  0xFFF3,
  0xF300,
  0x0043,
  0xFFFF,
  0x000F,
  0x0001,
  0xFFFF,
  0x003F,
  0x003F,
  0x003F,
  0x0000,
  0x0777,
  0x0333,
  0x0333,
  0x0073,
  0x3F01,
  0x3F3F,
  0x00FF,
  0x0000,
  0xFFFC,
  0xFFFF,
  0x7007,
  0xFF0F,
  0xFFFC,
  0xFFFF,
  0x7007,
  0xFF0F,
  0xFFFC,
  0xFFFF,
  0x7007,
  0xFF0F,
  0xFFFC,
  0xFFFF,
  0x7007,
  0xFF0F,
  0xFFFF,
  0x0000,
  0x003F,
  0x00FF,
  0xFFFF,
  0x0F03,
  0x7303,
  0xFFFF,
  0xB000,
  0x0029,
  0xFFFF,
  0x0F50,
  0x0F54,
  0x0FFF,
  0xF00F,
  0x78FC,
  0x5300,
  0x0F56,
  0x5300,
  0x0003,
  0x5003,
  0x7003,
  0x7003,
  0x5307,
  0x7003,
  0x7003,
  0xB003,
  0xB003,
  0xB003,
  0x0F17,
  0x7003,
  0xFFFF,
  0xB003,
  0xB007,
  0x0101,
  0x8C48,
  0xC82D,
  0x0000,
  0x0000,
  0xFFFF,
  0x0F50,
  0x0F50,
  0x1FD7,
  0x3FD7,
  0x3FDF,
  0x0FF5,
  0x0FF5,
  0x0F75,
  0xB003,
  0x78FD,
  0xFFFF,
  0x0F35,
  0x0F35,
  0x0003,
  0xB007,
  0x3FF5,
  0x8FFF,
  0x0000,
  0x8C5C,
  0xE380,
  0xB003,
  0x0F17,
  0x7003,
  0x7003,
  0x0F17,
  0x7003,
  0x0F77,
  0x7003,
  0x0F17,
  0x001F,
  0x003F,
  0x0001,
  0xFFFF,
  0x0001,
  0x0015,
  0x0000,
  0xFFFF,
  0xFFFF,
  0xFFFF,
  0xFFFF,
  0xFFFF,
  0xFFFF,
  0xFFFF,
  0xFFFF,
  0xFFFF,
  0xFFFF,
  0xFFFF,
  0xFFFF,
  0x0003,
  0x00FF,
  0x0003,
  0x1FEB,
  0x13FF,
  0x7FFF,
  0x07F7,
  0x00FF,
  0xFF37,
  0x007F,
  0x771F,
  0xFFFF,
  0x3FF7,
  0xFFFF,
  0x000F,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0x0000,
  0xFFFF,
  0xFFFF,
  0x1FF5,
  0xF037,
  0xFFFF,
  0xFFFF,
  0x000F,
  0x3FFF,
  0x3FFF,
  0x0000,
  0x0000,
  0xFC38,
  0xCFFF,
  0x8017,
  0xC3FF,
  0xD0BF,
  0x8000,
  0x8000,
  0x81FF,
  0x00FF,
  0x00FE,
  0x00FF,
  0xFFFF,
  0xFFFF,
  0xFF73,
  0x0003,
  0x0077,
  0xFFFF,
  0xFFFF,
  0xFFFF,
  0xFFFF,
  0xFFFF,
  0xFFFF,
  0xFFFF,
  0x8333,
  0x0100,
  0xF3FF,
  0x0000,
  0x0000,
  0xFFFF,
  0xFFFF
};

static kal_uint32 aPMURegBeg_bank0_write[34][2]= {  /* Register* , write val*/
  {0x003C, 0xFFFF},
  {0x003E, 0xFFFF},
  {0x0040, 0xBFFE},
  {0x0042, 0xFFFF},
  {0x0044, 0xFFFF},
  {0x0048, 0xFFFF},
  {0x004A, 0xFFFF},
  {0x004C, 0xFFFF},
  {0x004E, 0xFFFF},
  {0x0050, 0xFFFF},
  {0x0052, 0xFFFF},
  {0x0054, 0xFFFF},
  {0x0056, 0xFFEB},
  {0x005E, 0xFFFF},
  {0x0060, 0xBACE},
  {0x0062, 0xFFFF},
  {0x0064, 0xDFFF},
  {0x0066, 0xFFFF},
  {0x0068, 0xFFFF},
  {0x006A, 0xFFFF},
  {0x0160, 0xF0CF},
  {0x0166, 0xFFFE},
  {0x016C, 0xFFFF},
  {0x0172, 0xFFFF},
  {0x0174, 0xFFFF},
  {0x0176, 0xFFFF},
  {0x0178, 0xFFFF},
  {0x017A, 0xFFFE},
  {0x017C, 0xFFFF},
  {0x017E, 0xFFFF},
  {0x0180, 0xFFEF},
  {0x0182, 0xFFFF},
  {0x0184, 0xFFFF},
  {0x0186, 0xFFFF}
};

static kal_uint32 aPMURegBeg_mask_write[34] = { /* mask*/
  0x007F,
  0x001F,
  0x7037,
  0x000F,
  0x0003,
  0x0FFF,
  0x0030,
  0x7F03,
  0x00F0,
  0x0003,
  0x330D,
  0x7F00,
  0x07FF,
  0x0F37,
  0x7FFF,
  0x7FFF,
  0xFFFF,
  0x0777,
  0x0077,
  0xAAAB,
  0x0FFF,
  0x00FF,
  0x009F,
  0x0000,
  0x0000,
  0x0003,
  0x0003,
  0x0003,
  0x001F,
  0x001F,
  0x001F,
  0x8007,
  0xFFFF,
  0x0000
};




//---------------------------------------------------------------------------
// Common Test API
//---------------------------------------------------------------------------
extern kal_uint32 upmu_get_reg_value(kal_uint32 reg);
extern void upmu_set_reg_value(kal_uint32 reg, kal_uint32 reg_val);

//---------------------------------------------------------------------------
// Test Case Implementation
//---------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////////
//
//  TOP TEST CASE
//
///////////////////////////////////////////////////////////////////////////////////
void pmic_get_chip_version_ldvt(void)
{
//    kal_uint32 eco_version = 0;
    kal_uint32 tmp32;

	tmp32 = upmu_get_cid();

	printk("[pmic_get_chip_version_ldvt] %x\n", tmp32);		
}
///////////////////////////////////////////////////////////////////////////////////
void pmic_get_PCHR_CHRDET_ldvt(void)
{
	kal_uint32 tmp32;
	
	tmp32=upmu_get_rgs_chrdet();
		
	if(tmp32 == 0)
	{
		printk("[pmic_get_PCHR_CHRDET_ldvt] No charger\n");
	}
	else
	{
		printk("[pmic_get_PCHR_CHRDET_ldvt] Charger exist\n");
	}	
}
///////////////////////////////////////////////////////////////////////////////////
void pmic_VERIFY_DEFAULT_VALUE_ldvt(void)
{
//	kal_uint32 tmp32=0;
	kal_uint32 u2PMUReg = 0;
    kal_uint32 u2Cnt = 0;
    kal_uint32 default_value_mask = 0;

	printk("RegNum,DefaultValue,GotValue,TestValue,Mask\n");

	for(u2Cnt = 0; u2Cnt < (sizeof(aPMURegBeg_bank0)/sizeof(*aPMURegBeg_bank0)); ++u2Cnt)
	{
	   u2PMUReg = upmu_get_reg_value(    (aPMURegBeg_bank0[u2Cnt][0])  );

	   //printk("[Before MASK] %x,%x,%x\r\n",(aPMURegBeg_bank0[u2Cnt][0]), u2PMUReg,(aPMURegBeg_bank0[u2Cnt][1]));	   
	   
       //only check value of mask
	   u2PMUReg &= aPMURegBeg_mask[u2Cnt];
	   
	   //printk("[After MASK]%x,%x,%x\r\n",(aPMURegBeg_bank0[u2Cnt][0]), u2PMUReg,(aPMURegBeg_bank0[u2Cnt][1]));

       default_value_mask = ((aPMURegBeg_bank0[u2Cnt][1]) & aPMURegBeg_mask[u2Cnt]);
       
	   //if(u2PMUReg != (aPMURegBeg_bank0[u2Cnt][1]))
	   if(u2PMUReg != default_value_mask)
	   {
	   	   printk("[error] %x,%x,%x,%x,%x\r\n",
            (aPMURegBeg_bank0[u2Cnt][0]), 
            (aPMURegBeg_bank0[u2Cnt][1]),
            upmu_get_reg_value(    (aPMURegBeg_bank0[u2Cnt][0])  ),              
            u2PMUReg, 
            aPMURegBeg_mask[u2Cnt]);
	   }
	}	
	
}
///////////////////////////////////////////////////////////////////////////////////
void pmic_UVVP_PMIC_TOP_WR(int test_value)
{
	//kal_uint32 tmp32=0;
	kal_uint32 u2PMUReg = 0;
    kal_uint32 u2Cnt = 0;
    kal_uint32 default_value_mask = 0;

	printk("RegNum,write_value(default_value_mask),GotValue,TestValue,Mask\n");

	for(u2Cnt = 0; u2Cnt < (sizeof(aPMURegBeg_bank0_write)/sizeof(*aPMURegBeg_bank0_write)); ++u2Cnt)
	{
       //write test value
       upmu_set_reg_value( (aPMURegBeg_bank0_write[u2Cnt][0]), test_value );
	
	   //read back value 
	   u2PMUReg = upmu_get_reg_value(    (aPMURegBeg_bank0_write[u2Cnt][0])  );

	   //printk("[Before MASK] %x,%x,%x\r\n",(aPMURegBeg_bank0_write[u2Cnt][0]), u2PMUReg,(aPMURegBeg_mask_write[u2Cnt]));	   
	   
       //only check value of mask
	   u2PMUReg &= aPMURegBeg_mask_write[u2Cnt];
	   
	   //printk("[After MASK]%x,%x,%x\r\n",(aPMURegBeg_bank0_write[u2Cnt][0]), u2PMUReg,(aPMURegBeg_mask_write[u2Cnt]));
	   
       default_value_mask = (test_value & aPMURegBeg_mask_write[u2Cnt]);

	   if(u2PMUReg != default_value_mask)
	   {	   	   
           printk("[error] %x,%x(%x),%x,%x,%x\r\n",
            (aPMURegBeg_bank0_write[u2Cnt][0]), 
            //(aPMURegBeg_bank0[u2Cnt][1]),
            test_value,
            default_value_mask,
            upmu_get_reg_value(    (aPMURegBeg_bank0_write[u2Cnt][0])  ),              
            u2PMUReg, 
            aPMURegBeg_mask_write[u2Cnt]);
	   }
	}

    #if 0 //debug check
    for(u2Cnt = 0; u2Cnt < (sizeof(aPMURegBeg_bank0_write)/sizeof(*aPMURegBeg_bank0_write)); ++u2Cnt)
    {
        printk("Reg[%x] %x\n", 
            (aPMURegBeg_bank0_write[u2Cnt][0]), 
            upmu_get_reg_value(    (aPMURegBeg_bank0_write[u2Cnt][0])  )
            );
    }
    #endif
}

///////////////////////////////////////////////////////////////////////////////////
//
// AUXADC TEST CASE
//
///////////////////////////////////////////////////////////////////////////////////
void pmic_UVVP_PMIC_AUXADC_611(void)
{
	kal_int32 i=0;
	kal_int32 ret_val=0;

	printk("\r\n[pmic_UVVP_PMIC_AUXADC_611]\r\n");

	for(i=0;i<=7;i++)
	{
		ret_val=PMIC_IMM_GetOneChannelValue(i,1,1);
		
        if(i==2)
            ret_val = ret_val / 100; // 2:v_charger
		
		printk("[pmic_UVVP_PMIC_AUXADC_611] ch_num=%d, val=%d\n", i, ret_val);
	}
}
///////////////////////////////////////////////////////////////////////////////////
void pmic_UVVP_PMIC_AUXADC_612(void)
{
	kal_int32 i=0;
	kal_int32 ret_val=0;

	printk("\r\n[pmic_UVVP_PMIC_AUXADC_612]\r\n");

	for(i=0;i<=7;i++)
	{
		
		
		ret_val=PMIC_IMM_GetOneChannelValue(0,1,1); //0:vbat
		
		printk("[pmic_UVVP_PMIC_AUXADC_612] avg_num%d, val=%d, Reg[0x540]=%x\n", i, ret_val, upmu_get_reg_value(0x540));
	}
}
///////////////////////////////////////////////////////////////////////////////////
kal_uint32 LBAT_VOLT_MAX=0x037b; // wait SA provide, 4.2V
kal_uint32 LBAT_VOLT_MIN=0x02d1; // wait SA provide, 3.4V

kal_uint32 LBAT_DET_PRD_19_16=0x0; 
kal_uint32 LBAT_DET_PRD_15_0=0x03E8;

kal_uint32 LBAT_DEBT_MAX_8_0=1;
kal_uint32 LBAT_DEBT_MIN_8_0=1;

void pmic_UVVP_PMIC_AUXADC_621(void)
{
	kal_uint32 lbat_debounce_count_max=0;
	kal_uint32 lbat_debounce_count_min=0;
	kal_uint32 adc_out_lbat=0;

	printk("\r\n[pmic_UVVP_PMIC_AUXADC_621]\r\n");

	printk("LOW BATTERY (AUXADC) interrupt setting .. start\r\n");

	upmu_set_rg_int_en_bat_h(0);
	upmu_set_rg_int_en_bat_l(1);

//2.1.1 test low battery voltage interrupt 
//1. setup max voltage treshold as VBAT = 4.2
//    SetReg(RG_LBAT_VOLT_MAX_7_0, LBAT_VOLT_MAX[7:0]);
//    SetReg(RG_LBAT_VOLT_MAX_9_8, LBAT_VOLT_MAX[9:8]);
	upmu_set_rg_lbat_volt_max(LBAT_VOLT_MAX);
//2. setup min voltage treshold as VBAT = 3.4
//    SetReg(RG_LBAT_VOLT_MIN_7_0, LBAT_VOLT_MIN[7:0]);
//    SetReg(RG_LBAT_VOLT_MIN_9_8, LBAT_VOLT_MIN[9:8]);
	upmu_set_rg_lbat_volt_min(LBAT_VOLT_MIN);
//3. setup detection period
//    SetReg(RG_LBAT_DET_PRD_19_16, LBAT_DET_PRD[19:16]);
//    SetReg(RG_LBAT_DET_PRD_15_8, LBAT_DET_PRD[15:8]);
	upmu_set_rg_lbat_det_prd_19_16(LBAT_DET_PRD_19_16);
	upmu_set_rg_lbat_det_prd_15_0(LBAT_DET_PRD_15_0);
//4. setup max./min. debounce time.
//    SetReg(RG_LBAT_DEBT_MAX, LBAT_DEBT_MAX[8:0]);
//    SetReg(RG_LBAT_DEBT_MIN, LBAT_DEBT_MIN[8:0]);
	upmu_set_rg_lbat_debt_max(LBAT_DEBT_MAX_8_0);
	upmu_set_rg_lbat_debt_min(LBAT_DEBT_MIN_8_0);
//5. turn on IRQ
//    SetReg(RG_LBAT_IRQ_EN_MAX, 1'b0); // ?? =>1
//    SetReg(RG_LBAT_IRQ_EN_MIN, 1'b0); // ?? =>1
	upmu_set_rg_lbat_irq_en_max(1);
	upmu_set_rg_lbat_irq_en_min(1);
//6. turn on LowBattery Detection
//    SetReg(RG_LBAT_EN_MAX, 1'b1); 
//    SetReg(RG_LBAT_EN_MIN, 1'b1);
	upmu_set_rg_lbat_en_max(1);
	upmu_set_rg_lbat_en_min(1);
//7. Monitor Debounce counts
//    ReadReg(RG_LBAT_DEBOUNCE_COUNT_MAX);
//    ReadReg(BRG_LBAT_DEBOUNCE_COUNT_MIN);
	lbat_debounce_count_max = upmu_get_rg_lbat_debounce_count_max();
	lbat_debounce_count_min = upmu_get_rg_lbat_debounce_count_min();
//8. Read LowBattery Detect Value
//    while(! ReadReg(BRW_RG_ADC_RDY_LBAT));
//    ReadReg(BRW_RG_ADC_OUT_LBAT_7_0);
//    ReadReg(BRW_RG_ADC_OUT_LBAT_9_8);
	//while( upmu_get_rg_adc_rdy_lbat() != 1 )
	//{
	//	printk("1");
	//}
	adc_out_lbat = upmu_get_rg_adc_out_lbat();

    lbat_debounce_count_max = upmu_get_rg_lbat_debounce_count_max();
	lbat_debounce_count_min = upmu_get_rg_lbat_debounce_count_min();
	
//9. Test on VBAT = 3.5 -> 3.4 -> 3.3 and receive interrupt at pmic driver	
//	wake_up_pmic();

	printk("LOWBATTERY (AUXADC) interrupt setting .. done (adc_out_lbat=%d, lbat_debounce_count_max=%d, lbat_debounce_count_min=%d) \n", 
	adc_out_lbat, lbat_debounce_count_max, lbat_debounce_count_min);

}
///////////////////////////////////////////////////////////////////////////////////
void pmic_UVVP_PMIC_AUXADC_622(void)
{
	kal_uint32 lbat_debounce_count_max=0;
	kal_uint32 lbat_debounce_count_min=0;
	kal_uint32 adc_out_lbat=0;

	printk("\r\n[pmic_UVVP_PMIC_AUXADC_622]\r\n");

	printk("HIGH BATTERY (AUXADC) interrupt setting .. start\r\n");

	upmu_set_rg_int_en_bat_h(1);
	upmu_set_rg_int_en_bat_l(0);

//2.1.2 test low battery voltage interrupt 
//1. setup max voltage treshold as VBAT = 4.2
//    SetReg(RG_LBAT_VOLT_MAX_7_0, LBAT_VOLT_MAX[7:0]);
//    SetReg(RG_LBAT_VOLT_MAX_9_8, LBAT_VOLT_MAX[9:8]);
	upmu_set_rg_lbat_volt_max(LBAT_VOLT_MAX);
//2. setup min voltage treshold as VBAT = 3.4
//    SetReg(RG_LBAT_VOLT_MIN_7_0, LBAT_VOLT_MIN[7:0]);
//    SetReg(RG_LBAT_VOLT_MIN_9_8, LBAT_VOLT_MIN[9:8]);
	upmu_set_rg_lbat_volt_min(LBAT_VOLT_MIN);
//3. setup detection period
//    SetReg(RG_LBAT_DET_PRD_19_16, LBAT_DET_PRD[19:16]);
//    SetReg(RG_LBAT_DET_PRD_15_8, LBAT_DET_PRD[15:8]);
	upmu_set_rg_lbat_det_prd_19_16(LBAT_DET_PRD_19_16);
	upmu_set_rg_lbat_det_prd_15_0(LBAT_DET_PRD_15_0);
//4. setup max./min. debounce time.
//    SetReg(RG_LBAT_DEBT_MAX, LBAT_DEBT_MAX[8:0]);
//    SetReg(RG_LBAT_DEBT_MIN, LBAT_DEBT_MIN[8:0]);
	upmu_set_rg_lbat_debt_max(LBAT_DEBT_MAX_8_0);
	upmu_set_rg_lbat_debt_min(LBAT_DEBT_MIN_8_0);
//5. turn on IRQ
//    SetReg(RG_LBAT_IRQ_EN_MAX, 1'b0); // ?? =>1
//    SetReg(RG_LBAT_IRQ_EN_MIN, 1'b0); // ?? =>1
	upmu_set_rg_lbat_irq_en_max(1);
	upmu_set_rg_lbat_irq_en_min(1);
//6. turn on LowBattery Detection
//    SetReg(RG_LBAT_EN_MAX, 1'b1); 
//    SetReg(RG_LBAT_EN_MIN, 1'b1);
	upmu_set_rg_lbat_en_max(1);
	upmu_set_rg_lbat_en_min(1);
//7. Monitor Debounce counts
//    ReadReg(RG_LBAT_DEBOUNCE_COUNT_MAX);
//    ReadReg(BRG_LBAT_DEBOUNCE_COUNT_MIN);
	lbat_debounce_count_max = upmu_get_rg_lbat_debounce_count_max();
	lbat_debounce_count_min = upmu_get_rg_lbat_debounce_count_min();
//8. Read LowBattery Detect Value
//    while(! ReadReg(BRW_RG_ADC_RDY_LBAT));
//    ReadReg(BRW_RG_ADC_OUT_LBAT_7_0);
//    ReadReg(BRW_RG_ADC_OUT_LBAT_9_8);
	//while( upmu_get_rg_adc_rdy_lbat() != 1 )
	//{
	//	printk("2");
	//}
	adc_out_lbat = upmu_get_rg_adc_out_lbat();

    lbat_debounce_count_max = upmu_get_rg_lbat_debounce_count_max();
	lbat_debounce_count_min = upmu_get_rg_lbat_debounce_count_min();
	
//9. Test on VBAT = 4.0 -> 4.2 -> 4.3 and receive interrupt at pmic driver
//	wake_up_pmic();

	printk("LOWBATTERY (AUXADC) interrupt setting .. done (adc_out_lbat=%d, lbat_debounce_count_max=%d, lbat_debounce_count_min=%d) \n", 
	adc_out_lbat, lbat_debounce_count_max, lbat_debounce_count_min);

}


#define ADCCHANNEL_NO 11 //9+MD+GPS
kal_int32 ret_data[ADCCHANNEL_NO];
kal_int32 ret_order[ADCCHANNEL_NO];

#define VOLTAGE_FULL_RANGE 1800
#define ADC_PRECISE 32768

void do_reset_auxadc(void)
{
/*
	upmu_set_rg_auxadc_rst(1);
	mdelay(10);
	upmu_set_rg_auxadc_rst(0);
	mdelay(10);
*/
}

void do_dump_pmic(void)
{
//	kal_uint32 tmp32=0;
	kal_uint32 u2PMUReg = 0;
    kal_uint32 u2Cnt = 0;
//    kal_uint32 default_value_mask = 0;


	for(u2Cnt = 0; u2Cnt < (sizeof(aPMURegBeg_bank0)/sizeof(*aPMURegBeg_bank0)); ++u2Cnt)
	{
	   u2PMUReg = upmu_get_reg_value(    (aPMURegBeg_bank0[u2Cnt][0])  );

	   printk("[%3x]:%4x  ",aPMURegBeg_bank0[u2Cnt][0],u2PMUReg);

	   if(u2Cnt%5==0)
	   {
	   	printk("\n");
	   }

	}		
}

void do_dump_int_status(void)
{

    kal_uint32 ret=0;
//    kal_uint32 ret_val=0;
//    kal_uint32 reg_val=0;
    kal_uint32 int_status_val_0=0;
    kal_uint32 int_status_val_1=0;


        ret=pmic_read_interface(INT_STATUS0,(&int_status_val_0),0xFFFF,0x0);
        ret=pmic_read_interface(INT_STATUS1,(&int_status_val_1),0xFFFF,0x0);

	printk("INT_STATUS0:%x   IN_STATUS1:%x\n ",int_status_val_0,int_status_val_1);
}

kal_uint32  is_auxadc_busy(void)
{
    kal_uint32 ret=0;

    kal_uint32 int_status_val_0=0;

        ret=pmic_read_interface(0x73a,(&int_status_val_0),0x7FFF,0x1);

	return int_status_val_0;

}


void do_print_time(void)
{
	struct timeval t;
	do_gettimeofday(&t);
	printk("time[%d][%d]\n ",(int)t.tv_sec,(int)t.tv_usec);
}


void do_setThermal(kal_uint16 number)
{
	upmu_set_rg_adc_swctrl_en(number);
	upmu_set_baton_tdet_en(number);
	upmu_set_rg_vbuf_calen(number);
	upmu_set_rg_vbuf_en(number);
	upmu_set_rg_auxadc_chsel(4);
}

kal_int32 dvtloop=50;
void do_auxadc_polling(kal_uint16 number,kal_uint16 APMask,kal_uint16 GPSMask,kal_uint16 MDMask)
{

	kal_int32 i=0;
	kal_int32 ret_data[ADCCHANNEL_NO];
	kal_int32 ret_order[ADCCHANNEL_NO];
	kal_int16 order=1;

	for(i=0;i<ADCCHANNEL_NO;i++)
	{
		ret_data[i]=0;
		ret_order[i]=0;
	}
    
    //    0 : BATON2
    //    1 : CH6
    //    2 : THR SENSE2
    //    3 : THR SENSE1
    //    4 : VCDT
    //    5 : BATON1
    //    6 : ISENSE
    //    7 : BATSNS
    //    8 : ACCDET    

	do
	{
		if (ret_order[0]==0 && upmu_get_rg_adc_rdy_baton2() == 1 )
		{
			ret_data[0]=upmu_get_rg_adc_out_baton2();	
			ret_order[0]=order;
			order++;
		}

		if (ret_order[1]==0 && upmu_get_rg_adc_rdy_ch6() == 1 )
		{
			ret_data[1]=upmu_get_rg_adc_out_ch6();	
			ret_order[1]=order;
			order++;
		}

		if (ret_order[2]==0 && upmu_get_rg_adc_rdy_thr_sense2() == 1 )
		{
	
			ret_data[2]=upmu_get_rg_adc_out_thr_sense2();	
			ret_order[2]=order;
			order++;
		}

		if (ret_order[3]==0 && upmu_get_rg_adc_rdy_thr_sense1() == 1 )
		{		
			ret_data[3]=upmu_get_rg_adc_out_thr_sense1();	
			ret_order[3]=order;
			order++;
		}

		if (ret_order[4]==0 && upmu_get_rg_adc_rdy_vcdt() == 1 )
		{	
			ret_data[4]=upmu_get_rg_adc_out_vcdt();	
			ret_order[4]=order;
			order++;
		}

		if (ret_order[5]==0 && upmu_get_rg_adc_rdy_baton1() == 1 )
		{
			ret_data[5]=upmu_get_rg_adc_out_baton1();	
			ret_order[5]=order;
			order++;
		}

		if (ret_order[6]==0 && upmu_get_rg_adc_rdy_isense() == 1 )
		{		
			ret_data[6]=upmu_get_rg_adc_out_isense();	
			ret_order[6]=order;
			order++;
		}

		if (ret_order[7]==0 && upmu_get_rg_adc_rdy_batsns() == 1 )
		{
	
			ret_data[7]=upmu_get_rg_adc_out_batsns();	
			ret_order[7]=order;
			order++;
		}

		if (ret_order[8]==0 && upmu_get_rg_adc_rdy_ch5() == 1 )
		{
			ret_data[8]=upmu_get_rg_adc_out_ch5();	
			ret_order[8]=order;
			order++;
		}

		if (ret_order[9]==0 && upmu_get_rg_adc_rdy_gps() == 1 )
		{

			ret_data[9]=upmu_get_rg_adc_out_gps();	
			ret_order[9]=order;
			order++;
		}

		if (ret_order[10]==0 && upmu_get_rg_adc_rdy_md() == 1 )
		{		
			ret_data[10]=upmu_get_rg_adc_out_md();	
			ret_order[10]=order;
			order++;
		}		

		if (i>dvtloop)
			break;

			printk("[%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d]\n ", 
		upmu_get_rg_adc_rdy_baton2(),
		upmu_get_rg_adc_rdy_ch6(),
		upmu_get_rg_adc_rdy_thr_sense1(),
		upmu_get_rg_adc_rdy_thr_sense2(),
		upmu_get_rg_adc_rdy_vcdt(),
		upmu_get_rg_adc_rdy_baton1(),
		upmu_get_rg_adc_rdy_isense(),
		upmu_get_rg_adc_rdy_batsns(),
		upmu_get_rg_adc_rdy_ch5(),
		upmu_get_rg_adc_rdy_gps(),
		upmu_get_rg_adc_rdy_md());
		
	}
	while(order<=number);

    //    0 : BATON2
    //    1 : CH6
    //    2 : THR SENSE2
    //    3 : THR SENSE1
    //    4 : VCDT
    //    5 : BATON1
    //    6 : ISENSE*
    //    7 : BATSNS*
    //    8 : ACCDET    

	printk("ch0:BATON2	order[%2d] val[0x%4x] [%4d]\n ", ret_order[0], ret_data[0],ret_data[0]*VOLTAGE_FULL_RANGE/ADC_PRECISE);   
	printk("ch1:CH6		order[%2d] val[0x%4x] [%4d]\n ", ret_order[1], ret_data[1],ret_data[1]*VOLTAGE_FULL_RANGE/ADC_PRECISE); 
	printk("ch2:THR2		order[%2d] val[0x%4x] [%4d]\n ", ret_order[2], ret_data[2],ret_data[2]*VOLTAGE_FULL_RANGE/ADC_PRECISE); 
	printk("ch3:THR1		order[%2d] val[0x%4x] [%4d]\n ", ret_order[3], ret_data[3],ret_data[3]*VOLTAGE_FULL_RANGE/ADC_PRECISE); 
	printk("ch4:VCDT		order[%2d] val[0x%4x] [%4d]\n ", ret_order[4], ret_data[4],ret_data[4]*VOLTAGE_FULL_RANGE/ADC_PRECISE); 
	printk("ch5:BATON1	order[%2d] val[0x%4x] [%4d]\n ", ret_order[5], ret_data[5],ret_data[5]*4*VOLTAGE_FULL_RANGE/ADC_PRECISE); 
	printk("ch6:ISENSE	order[%2d] val[0x%4x] [%4d]\n ", ret_order[6], ret_data[6],ret_data[6]*4*VOLTAGE_FULL_RANGE/ADC_PRECISE); 
	printk("ch7:BATSNS	order[%2d] val[0x%4x] [%4d]\n ", ret_order[7], ret_data[7],ret_data[7]*4*VOLTAGE_FULL_RANGE/ADC_PRECISE); 
	printk("ch8:ACCDET	order[%2d] val[0x%4x] [%4d]\n ", ret_order[8], ret_data[8],ret_data[8]*VOLTAGE_FULL_RANGE/ADC_PRECISE); 
	printk("gps:			order[%2d] val[0x%4x] [%4d]\n ", ret_order[9], ret_data[9],ret_data[9]*VOLTAGE_FULL_RANGE/ADC_PRECISE/2); 
       printk("md:			order[%2d] val[0x%4x] [%4d]\n ", ret_order[10], ret_data[10],ret_data[10]*VOLTAGE_FULL_RANGE/ADC_PRECISE/2); 

	printk("[%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d]\n ", 
		upmu_get_rg_adc_rdy_baton2(),
		upmu_get_rg_adc_rdy_ch6(),
		upmu_get_rg_adc_rdy_thr_sense1(),
		upmu_get_rg_adc_rdy_thr_sense2(),
		upmu_get_rg_adc_rdy_vcdt(),
		upmu_get_rg_adc_rdy_baton1(),
		upmu_get_rg_adc_rdy_isense(),
		upmu_get_rg_adc_rdy_batsns(),
		upmu_get_rg_adc_rdy_ch5(),
		upmu_get_rg_adc_rdy_gps(),
		upmu_get_rg_adc_rdy_md());

	do_reset_auxadc();
	   

}


// test case 1.1.1
void pmic_UVVP_PMIC_AUXADC_111(void)
{
	kal_int32 i=0;
	kal_int32 ret_val=0;

	do_reset_auxadc();
	while(is_auxadc_busy()!=0);

	upmu_set_rg_vbuf_en(1);
	printk("\r\n[pmic_UVVP_PMIC_AUXADC_111]\r\n");

    //    0 : BATON2
    //    1 : CH6
    //    2 : THR SENSE2
    //    3 : THR SENSE1
    //    4 : VCDT
    //    5 : BATON1
    //    6 : ISENSE
    //    7 : BATSNS
    //    8 : ACCDET    

	for(i=0;i<=8;i++)
	{
		//if (i==5)
		//do_setThermal(1);


//		if (i==2)
	//	{
			//upmu_set_rg_adc_swctrl_en(1);
		//	upmu_set_baton_tdet_en(1);
		//	upmu_set_rg_vbuf_calen(1);
		//	upmu_set_rg_vbuf_en(1);
		//	upmu_set_rg_auxadc_chsel(4);
		//}
		
		ret_val=PMIC_IMM_GetOneChannelValue(i,1,1); //0:vbat

		//if (i==2)
	//	{
			//upmu_set_rg_adc_swctrl_en(0);
		//	upmu_set_baton_tdet_en(0);
		//}


		//if (i==5)
		//do_setThermal(0);
		//printk("[pmic_UVVP_PMIC_AUXADC_111] avg_num%d, val=%d \n", i, ret_val);

		if (i==0)
		{
			printk("[pmic_UVVP_PMIC_AUXADC_111] %d:BATON2 val=%d \n", i, ret_val);
		}
		else if (i==1)
		{
			printk("[pmic_UVVP_PMIC_AUXADC_111] %d:CH6 val=%d \n", i, ret_val);
		}
		else if (i==3)
		{
			printk("[pmic_UVVP_PMIC_AUXADC_111] %d:THR SENSE1 val=%d \n", i, ret_val);
		}
		else if (i==2)
		{
			printk("[pmic_UVVP_PMIC_AUXADC_111] %d:THR SENSE2 val=%d \n", i, ret_val);
		}
		else if (i==4)
		{
			printk("[pmic_UVVP_PMIC_AUXADC_111] %d:VCDT val=%d \n", i, ret_val);
		}
		else if (i==5)
		{
			printk("[pmic_UVVP_PMIC_AUXADC_111] %d:BATON1 val=%d \n", i, ret_val);
		}		
		else if (i==6)
		{
			printk("[pmic_UVVP_PMIC_AUXADC_111] %d:ISENSE val=%d \n", i, ret_val);
		}
		else if (i==7)
		{
			printk("[pmic_UVVP_PMIC_AUXADC_111] %d:BATSNS val=%d \n", i, ret_val);
		}
		else if (i==8)
		{
			printk("[pmic_UVVP_PMIC_AUXADC_111] %d:ACCDET val=%d \n", i, ret_val);
		}		
		

		
	}

}

void do_set_Adc_external_clock(void)
{

/*
//¤Á¦¨¥~ÄéCLK
SetReg(GPIO0_MODE, 3'd5);
SetReg(RG_AUXADC_SDM_TSTSEL,1'b1)
//set GPO
SetReg(GPIO2_MODE, 3'd7);  //TESTOUT3
SetReg(GPIO9_MODE, 3'd7);  //TESTOUT2
SetReg(GPIO8_MODE, 3'd7);  //TESTOUT1
SetReg(GPIO7_MODE, 3'd7);  //TESTOUT0
*/

	pmic_config_interface(0xc0c0,0x05,0x07,0);	
	upmu_set_rg_auxadc_sdm_tstsel(1);
	pmic_config_interface(0xc0c0,0x07,0x07,6);	
	pmic_config_interface(0xc0c8,0x07,0x07,12);	
	pmic_config_interface(0xc0c8,0x07,0x07,9);	
	pmic_config_interface(0xc0c8,0x07,0x07,6);	
	mdelay(10);

	{
		U32 data1,data2,data3;
		pmic_read_interface(0xc0c0,&data1,0xffff,0); 
		pmic_read_interface(0xc0c8,&data2,0xffff,0); 
		
		pmic_read_interface(TOP_CKTST1,&data3,0xffff,0); 
		printk("setting:[%d][%d][%d]\n ", 
			data1,data2,data3);
		
	}

}

void pmic_UVVP_PMIC_AUXADC_1311(void)
{
//	kal_int32 i=0;
	kal_int32 ret_val=0;

	do_reset_auxadc();
	while(is_auxadc_busy()!=0);

	upmu_set_rg_vbuf_en(1);
	printk("\r\n[pmic_UVVP_PMIC_AUXADC_131]\r\n");

    //    0 : BATON2
    //    1 : CH6
    //    2 : THR SENSE2
    //    3 : THR SENSE1
    //    4 : VCDT
    //    5 : BATON1
    //    6 : ISENSE
    //    7 : BATSNS
    //    8 : ACCDET    

		ret_val=PMIC_IMM_GetOneChannelValue(2,1,1); //0:vbat
		printk("[pmic_UVVP_PMIC_AUXADC_111] %d:THR SENSE2 val=%d \n", 2, ret_val);

		
}


void pmic_UVVP_PMIC_AUXADC_1321(void)
{

//	kal_int32 i=0;
	kal_int32 ret_val=0;

	do_reset_auxadc();
	while(is_auxadc_busy()!=0);

	upmu_set_rg_vbuf_en(1);
	printk("\r\n[pmic_UVVP_PMIC_AUXADC_111]\r\n");

    //    0 : BATON2
    //    1 : CH6
    //    2 : THR SENSE2
    //    3 : THR SENSE1
    //    4 : VCDT
    //    5 : BATON1
    //    6 : ISENSE
    //    7 : BATSNS
    //    8 : ACCDET    

		ret_val=PMIC_IMM_GetOneChannelValue(3,1,1); //0:vbat


		printk("[pmic_UVVP_PMIC_AUXADC_111] %d:THR SENSE1 val=%d \n",3, ret_val);
	
}


void pmic_UVVP_PMIC_AUXADC_131(void)
{

	pmic_config_interface(0x77A,0x5e10,0xffff,0);
	pmic_config_interface(0x710,0x0208,0xffff,0);
}


// test case 1.3.1
void pmic_UVVP_PMIC_AUXADC_13111(void)
{
	do_reset_auxadc();
	do_set_Adc_external_clock();
	upmu_set_rg_vbuf_en(1);

	while(is_auxadc_busy()!=0);

	printk("before sample , rdybit:[%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d] busy:[%d]\n ", 
		upmu_get_rg_adc_rdy_baton2(),
		upmu_get_rg_adc_rdy_ch6(),
		upmu_get_rg_adc_rdy_thr_sense1(),
		upmu_get_rg_adc_rdy_thr_sense2(),
		upmu_get_rg_adc_rdy_vcdt(),
		upmu_get_rg_adc_rdy_baton1(),
		upmu_get_rg_adc_rdy_isense(),
		upmu_get_rg_adc_rdy_batsns(),
		upmu_get_rg_adc_rdy_ch5(),
		upmu_get_rg_adc_rdy_gps(),
		upmu_get_rg_adc_rdy_md(),is_auxadc_busy());
	
	upmu_set_rg_md_rqst(0);
	upmu_set_rg_gps_rqst(0);
	upmu_set_rg_ap_rqst_list(0x0);
	mdelay(10);

	upmu_set_rg_md_rqst(1);
	upmu_set_rg_gps_rqst(1);
	upmu_set_rg_ap_rqst_list(0x1ff);

	do_auxadc_polling(11,0x1ff,1,1);

}



// test case 1.3.2
void pmic_UVVP_PMIC_AUXADC_132(void)
{
	do_reset_auxadc();
	do_set_Adc_external_clock();
	upmu_set_rg_vbuf_en(1);

	while(is_auxadc_busy()!=0);

	printk("before sample , rdybit:[%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d] busy:[%d]\n ", 
		upmu_get_rg_adc_rdy_baton2(),
		upmu_get_rg_adc_rdy_ch6(),
		upmu_get_rg_adc_rdy_thr_sense1(),
		upmu_get_rg_adc_rdy_thr_sense2(),
		upmu_get_rg_adc_rdy_vcdt(),
		upmu_get_rg_adc_rdy_baton1(),
		upmu_get_rg_adc_rdy_isense(),
		upmu_get_rg_adc_rdy_batsns(),
		upmu_get_rg_adc_rdy_ch5(),
		upmu_get_rg_adc_rdy_gps(),
		upmu_get_rg_adc_rdy_md(),is_auxadc_busy());

	upmu_set_rg_md_rqst(0);
	upmu_set_rg_gps_rqst(0);
	upmu_set_rg_ap_rqst_list(0);

	mdelay(10);

	upmu_set_rg_md_rqst(1);
	upmu_set_rg_gps_rqst(1);
	upmu_set_rg_ap_rqst_list(0x1bf);
	udelay(30);
	upmu_set_rg_ap_rqst_list(0x040);

	do_auxadc_polling(11,0x1ff,1,1);
}

// test case 1.3.3
void pmic_UVVP_PMIC_AUXADC_133(void)
{
	do_reset_auxadc();
	do_set_Adc_external_clock();
	upmu_set_rg_vbuf_en(1);
	while(is_auxadc_busy()!=0);

	printk("before sample , rdybit:[%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d] busy:[%d]\n ", 
		upmu_get_rg_adc_rdy_baton2(),
		upmu_get_rg_adc_rdy_ch6(),
		upmu_get_rg_adc_rdy_thr_sense1(),
		upmu_get_rg_adc_rdy_thr_sense2(),
		upmu_get_rg_adc_rdy_vcdt(),
		upmu_get_rg_adc_rdy_baton1(),
		upmu_get_rg_adc_rdy_isense(),
		upmu_get_rg_adc_rdy_batsns(),
		upmu_get_rg_adc_rdy_ch5(),
		upmu_get_rg_adc_rdy_gps(),
		upmu_get_rg_adc_rdy_md(),is_auxadc_busy());

	upmu_set_rg_md_rqst(0);
	upmu_set_rg_gps_rqst(0);
	upmu_set_rg_ap_rqst_list(0);

	mdelay(10);

	upmu_set_rg_ap_rqst_list(0x1bf);

	while( is_auxadc_busy()==0)
	{
		printk("auxadc is not busy \n ");		
	}
	
	upmu_set_rg_md_rqst(1);
	upmu_set_rg_gps_rqst(1);

	upmu_set_rg_ap_rqst_list(0x040);

	do_auxadc_polling(11,0x1ff,1,1);
}

// test case 1.3.4
void pmic_UVVP_PMIC_AUXADC_134(void)
{

	do_reset_auxadc();
	do_set_Adc_external_clock();
	upmu_set_rg_vbuf_en(1);

	while(is_auxadc_busy()!=0);

	printk("before sample , rdybit:[%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d] busy:[%d]\n ", 
		upmu_get_rg_adc_rdy_baton2(),
		upmu_get_rg_adc_rdy_ch6(),
		upmu_get_rg_adc_rdy_thr_sense1(),
		upmu_get_rg_adc_rdy_thr_sense2(),
		upmu_get_rg_adc_rdy_vcdt(),
		upmu_get_rg_adc_rdy_baton1(),
		upmu_get_rg_adc_rdy_isense(),
		upmu_get_rg_adc_rdy_batsns(),
		upmu_get_rg_adc_rdy_ch5(),
		upmu_get_rg_adc_rdy_gps(),
		upmu_get_rg_adc_rdy_md(),is_auxadc_busy());

	upmu_set_rg_md_rqst(0);
	upmu_set_rg_gps_rqst(0);
	upmu_set_rg_ap_rqst_list(0);
	mdelay(10);

	upmu_set_rg_gps_rqst(1);
	while( is_auxadc_busy()==0)
	{
		printk("auxadc is not busy \n ");		
	}
	upmu_set_rg_md_rqst(1);

	do_auxadc_polling(2,0x0,1,1);
}

// test case 1.3.5
void pmic_UVVP_PMIC_AUXADC_135(void)
{

	do_reset_auxadc();
	do_set_Adc_external_clock();
	upmu_set_rg_vbuf_en(1);

	while(is_auxadc_busy()!=0);

	printk("before sample , rdybit:[%d][%d][%d][%d][%d][%d][%d][%d][%d][%d][%d] busy:[%d]\n ", 
		upmu_get_rg_adc_rdy_baton2(),
		upmu_get_rg_adc_rdy_ch6(),
		upmu_get_rg_adc_rdy_thr_sense1(),
		upmu_get_rg_adc_rdy_thr_sense2(),
		upmu_get_rg_adc_rdy_vcdt(),
		upmu_get_rg_adc_rdy_baton1(),
		upmu_get_rg_adc_rdy_isense(),
		upmu_get_rg_adc_rdy_batsns(),
		upmu_get_rg_adc_rdy_ch5(),
		upmu_get_rg_adc_rdy_gps(),
		upmu_get_rg_adc_rdy_md(),is_auxadc_busy());

	upmu_set_rg_md_rqst(0);
	upmu_set_rg_gps_rqst(0);
	upmu_set_rg_ap_rqst_list(0);

	mdelay(10);

	upmu_set_rg_gps_rqst(1);
	while(!upmu_get_rg_adc_rdy_gps());
	
	//print time
	do_print_time();
	upmu_set_rg_md_rqst(1);
	while(!upmu_get_rg_adc_rdy_md())
	{
		printk("polling md \n ");		
	}
	
	//print time
	do_print_time();
	printk("GPS:%d MD:%d (they should the same)\n ",upmu_get_rg_adc_out_gps(),upmu_get_rg_adc_out_md());

}

// test case 1.4.1
void pmic_UVVP_PMIC_AUXADC_141(void)
{
	do_reset_auxadc();


	// SetReg( RG_DATA_REUSE_SEL, 2'b3);
	upmu_set_rg_data_reuse_sel(3);
	upmu_set_rg_md_rqst(0);
	upmu_set_rg_gps_rqst(0);
	upmu_set_rg_ap_rqst_list(0);

	upmu_set_rg_auxadc_sdm_ck_hw_mode(1);
	upmu_set_rg_auxadc_sdm_sel_hw_mode(1);
	upmu_set_rg_clksq_en_aux(0);
	upmu_set_rg_clksq_en_aux_md(0);
	upmu_set_rg_auxadc_sdm_ck_wake_pdn(1);
	
	upmu_set_rg_gps_rqst(1);

	while(!upmu_get_rg_adc_rdy_gps())
	{
		printk("polling GPS \n ");
	}
	printk("get GPS data :%d %d\n ",upmu_get_rg_adc_out_gps(),upmu_get_rg_adc_out_gps()*VOLTAGE_FULL_RANGE/ADC_PRECISE/2);
}

// test case 2.1.1
void pmic_UVVP_PMIC_AUXADC_211(void)
{

//RG_LBAT_VOLT_MAX  = 12'H955
//RG_LBAT_VOLT_MIN  = 12'H871
	//0.set interrupt
	do_dump_int_status();
	upmu_set_rg_int_en_bat_l(1);
	upmu_set_rg_int_en_bat_h(1);

	// 1.setup max voltage threahold
	upmu_set_rg_lbat_volt_max(0x955);

 	// 2.setup min voltage threshold
	upmu_set_rg_lbat_volt_min(0x871);

	// 3.setup detection period
	upmu_set_rg_lbat_det_prd_19_16(0);
	upmu_set_rg_lbat_det_prd_15_0(4);

	// 4.setup max/min debounce time
	upmu_set_rg_lbat_debt_max(2);
	upmu_set_rg_lbat_debt_min(2);

	// 5.turn on iRQ
	upmu_set_rg_lbat_irq_en_max(0);
	upmu_set_rg_lbat_irq_en_min(0);	
	upmu_set_rg_lbat_irq_en_max(0);
	upmu_set_rg_lbat_irq_en_min(1);

	// 6.turn on low battery detection
	upmu_set_rg_lbat_en_max(0);
	upmu_set_rg_lbat_en_min(0);	
	upmu_set_rg_lbat_en_max(0);
	upmu_set_rg_lbat_en_min(1);

	// 7.monitor debounce counts
	printk("RG_LBAT_DEBOUNCE_COUNT:  max[%d]  min[%d]\n ", 
		upmu_get_rg_lbat_debounce_count_max(), 
		upmu_get_rg_lbat_debounce_count_min()); 

	printk("polling "); 
	// 8.read lowbattery detection value
	while(upmu_get_rg_lbat_min_irq_b()==1);
	
	printk("RG_ADC_OUT_LBAT:%x %d\n ", 
		upmu_get_rg_adc_out_lbat(),upmu_get_rg_adc_out_lbat()*1800*4/4095); 

	do_dump_int_status();

}

// test case 2.1.2
void pmic_UVVP_PMIC_AUXADC_212(void)
{
//RG_LBAT_VOLT_MAX  = 12'H955
//RG_LBAT_VOLT_MIN  = 12'H871

       do_dump_int_status();
	//0.set interrupt
	upmu_set_rg_int_en_bat_l(1);
	upmu_set_rg_int_en_bat_h(1);

	// 1.setup max voltage threahold
	upmu_set_rg_lbat_volt_max(0x955);

 	// 2.setup min voltage threshold
	upmu_set_rg_lbat_volt_min(0x871);

	// 3.setup detection period
	upmu_set_rg_lbat_det_prd_19_16(0);
	upmu_set_rg_lbat_det_prd_15_0(4);

	// 4.setup max/min debounce time
	upmu_set_rg_lbat_debt_max(2);
	upmu_set_rg_lbat_debt_min(2);

	// 5.turn on iRQ
	upmu_set_rg_lbat_irq_en_max(0);
	upmu_set_rg_lbat_irq_en_min(0);	
	upmu_set_rg_lbat_irq_en_max(1);
	upmu_set_rg_lbat_irq_en_min(0);

	// 6.turn on low battery detection
	upmu_set_rg_lbat_en_max(0);
	upmu_set_rg_lbat_en_min(0);	
	upmu_set_rg_lbat_en_max(1);
	upmu_set_rg_lbat_en_min(0);

	// 7.monitor debounce counts
	printk("RG_LBAT_DEBOUNCE_COUNT:  max[%d]  min[%d]\n ", 
		upmu_get_rg_lbat_debounce_count_max(), 
		upmu_get_rg_lbat_debounce_count_min()); 

	printk("polling "); 
	// 8.read lowbattery detection value
	while(upmu_get_rg_lbat_max_irq_b()==1);
	
	printk("RG_ADC_OUT_LBAT:%x %d\n ", 
		upmu_get_rg_adc_out_lbat(),upmu_get_rg_adc_out_lbat()*1800*4/4095); 
	do_dump_int_status();
}


// test case 2.1.3
void pmic_UVVP_PMIC_AUXADC_213(void)
{
	do_dump_int_status();
	upmu_set_strup_auxadc_start_sel(0);
	upmu_set_strup_auxadc_rstb_sw(1);
	upmu_set_strup_auxadc_rstb_sel(1);

	//0.1
	pmic_config_interface(0x776,0x01,0x8000,15);	
	pmic_config_interface(0x778,0x01,0x8000,15);	
	upmu_set_rg_adc_deci_gdly(1);

	//0.2
       //upmu_set_rg_adc_lp_en(1);

	//0.set interrupt
	upmu_set_rg_int_en_bat_l(1);
	upmu_set_rg_int_en_bat_h(1);

	// 1.setup max voltage threahold
	upmu_set_rg_lbat_volt_max(0x955);

 	// 2.setup min voltage threshold
	upmu_set_rg_lbat_volt_min(0x871);

	// 3.setup detection period
	upmu_set_rg_lbat_det_prd_19_16(0);
	upmu_set_rg_lbat_det_prd_15_0(4);

	// 4.setup max/min debounce time
	upmu_set_rg_lbat_debt_max(2);
	upmu_set_rg_lbat_debt_min(2);

	// 5.turn on iRQ
	upmu_set_rg_lbat_irq_en_max(0);
	upmu_set_rg_lbat_irq_en_min(0);	
	upmu_set_rg_lbat_irq_en_max(0);
	upmu_set_rg_lbat_irq_en_min(1);

	// 6.turn on low battery detection
	upmu_set_rg_lbat_en_max(0);
	upmu_set_rg_lbat_en_min(0);	
	upmu_set_rg_lbat_en_max(0);
	upmu_set_rg_lbat_en_min(1);

	// 7.monitor debounce counts
	printk("RG_LBAT_DEBOUNCE_COUNT:  max[%d]  min[%d]\n ", 
		upmu_get_rg_lbat_debounce_count_max(), 
		upmu_get_rg_lbat_debounce_count_min()); 

	//enter sleep mode
       upmu_set_rg_srclken_hw_mode(0);
	upmu_set_rg_srclken_en(0);

	printk("polling "); 
	// 8.read lowbattery detection value
	while(upmu_get_rg_lbat_min_irq_b()==1);
	
	printk("RG_ADC_OUT_LBAT:%x %d\n ", 
		upmu_get_rg_adc_out_lbat(),upmu_get_rg_adc_out_lbat()*1800*4/4095); 
	do_dump_int_status();

	upmu_set_rg_srclken_hw_mode(1);
}

// test case 2.1.4
void pmic_UVVP_PMIC_AUXADC_214(void)
{
	do_dump_int_status();
	upmu_set_strup_auxadc_start_sel(0);
	upmu_set_strup_auxadc_rstb_sw(1);
	upmu_set_strup_auxadc_rstb_sel(1);

       //0.1
	pmic_config_interface(0x776,0x01,0x8000,15);	
	pmic_config_interface(0x778,0x01,0x8000,15);	
	upmu_set_rg_adc_deci_gdly(1);

	//0.2
       //upmu_set_rg_adc_lp_en(1);

	//0.set interrupt
	upmu_set_rg_int_en_bat_l(1);
	upmu_set_rg_int_en_bat_h(1);

	// 1.setup max voltage threahold
	upmu_set_rg_lbat_volt_max(0x955);

 	// 2.setup min voltage threshold
	upmu_set_rg_lbat_volt_min(0x871);

	// 3.setup detection period
	upmu_set_rg_lbat_det_prd_19_16(0);
	upmu_set_rg_lbat_det_prd_15_0(4);

	// 4.setup max/min debounce time
	upmu_set_rg_lbat_debt_max(2);
	upmu_set_rg_lbat_debt_min(2);

	// 5.turn on iRQ
	upmu_set_rg_lbat_irq_en_max(0);
	upmu_set_rg_lbat_irq_en_min(0);	
	upmu_set_rg_lbat_irq_en_max(1);
	upmu_set_rg_lbat_irq_en_min(0);

	// 6.turn on low battery detection
	upmu_set_rg_lbat_en_max(0);
	upmu_set_rg_lbat_en_min(0);	
	upmu_set_rg_lbat_en_max(1);
	upmu_set_rg_lbat_en_min(0);

	// 7.monitor debounce counts
	printk("RG_LBAT_DEBOUNCE_COUNT:  max[%d]  min[%d]\n ", 
		upmu_get_rg_lbat_debounce_count_max(), 
		upmu_get_rg_lbat_debounce_count_min()); 

	//enter sleep mode
       upmu_set_rg_srclken_hw_mode(0);
	upmu_set_rg_srclken_en(0);

	printk("polling "); 
	// 8.read lowbattery detection value
	while(upmu_get_rg_lbat_max_irq_b()==1);
	
	printk("RG_ADC_OUT_LBAT:%x %d\n ", 
		upmu_get_rg_adc_out_lbat(),upmu_get_rg_adc_out_lbat()*1800*4/4095); 

	do_dump_int_status();

	upmu_set_rg_srclken_hw_mode(1);
}

// test case 2.1.5
void pmic_UVVP_PMIC_AUXADC_215(void)
{

}

// test case 2.1.6
void pmic_UVVP_PMIC_AUXADC_216(void)
{
	do_dump_int_status();
	upmu_set_rg_source_lbat_sel(1);

	//0.set interrupt
	upmu_set_rg_int_en_bat_l(1);
	upmu_set_rg_int_en_bat_h(1);

	// 1.setup max voltage threahold
	upmu_set_rg_lbat_volt_max(0x955);

 	// 2.setup min voltage threshold
	upmu_set_rg_lbat_volt_min(0x871);

	// 3.setup detection period
	upmu_set_rg_lbat_det_prd_19_16(0);
	upmu_set_rg_lbat_det_prd_15_0(4);

	// 4.setup max/min debounce time
	upmu_set_rg_lbat_debt_max(2);
	upmu_set_rg_lbat_debt_min(2);

	// 5.turn on iRQ
	upmu_set_rg_lbat_irq_en_max(0);
	upmu_set_rg_lbat_irq_en_min(0);	
	upmu_set_rg_lbat_irq_en_max(0);
	upmu_set_rg_lbat_irq_en_min(1);

	// 6.turn on low battery detection
	upmu_set_rg_lbat_en_max(0);
	upmu_set_rg_lbat_en_min(0);	
	upmu_set_rg_lbat_en_max(0);
	upmu_set_rg_lbat_en_min(1);

	// 7.monitor debounce counts
	printk("RG_LBAT_DEBOUNCE_COUNT:  max[%d]  min[%d]\n ", 
		upmu_get_rg_lbat_debounce_count_max(), 
		upmu_get_rg_lbat_debounce_count_min()); 

	printk("polling "); 
	// 8.read lowbattery detection value
	while(upmu_get_rg_lbat_min_irq_b()==1);
	
	printk("RG_ADC_OUT_LBAT:%x %d\n ", 
		upmu_get_rg_adc_out_lbat(),upmu_get_rg_adc_out_lbat()*1800*4/4095); 
	do_dump_int_status();
}


// test case 3.2.1
void pmic_UVVP_PMIC_AUXADC_321(void)
{
	kal_int32 data;

    //    0 : BATON2
    //    1 : CH6
    //    2 : THR SENSE2
    //    3 : THR SENSE1
    //    4 : VCDT
    //    5 : BATON1
    //    6 : ISENSE*
    //    7 : BATSNS*
    //    8 : ACCDET   

	//batsns

	upmu_set_rg_adc_swctrl_en(1);

	upmu_set_rg_adcin_vsen_en(0);
	upmu_set_rg_adcin_vbat_en(1);
	upmu_set_rg_adcin_vsen_mux_en(0);
	upmu_set_rg_adcin_vsen_ext_baton_en(0);
	upmu_set_rg_adcin_chr_en(0);
	upmu_set_baton_tdet_en(0);
	
	upmu_set_rg_baton_en(0);
	upmu_set_rg_auxadc_chsel(0);

	printk("[polling batsns]");
	data=PMIC_IMM_GetOneChannelValue(7,1,1);
	printk("[batsns :%d \n",data);



	//isense

	upmu_set_rg_adc_swctrl_en(1);

	upmu_set_rg_adcin_vsen_en(1);
	upmu_set_rg_adcin_vbat_en(0);
	upmu_set_rg_adcin_vsen_mux_en(1);
	upmu_set_rg_adcin_vsen_ext_baton_en(0);
	upmu_set_rg_adcin_chr_en(0);
	upmu_set_baton_tdet_en(0);
	
	upmu_set_rg_baton_en(0);
	upmu_set_rg_auxadc_chsel(0);

	printk("[polling isense]");
	data=PMIC_IMM_GetOneChannelValue(6,1,1);
	printk("[isense :%d \n",data);

	//CHRIN

	upmu_set_rg_adc_swctrl_en(1);

	upmu_set_rg_adcin_vsen_en(0);
	upmu_set_rg_adcin_vbat_en(0);
	upmu_set_rg_adcin_vsen_mux_en(0);
	upmu_set_rg_adcin_vsen_ext_baton_en(0);
	upmu_set_rg_adcin_chr_en(1);
	upmu_set_baton_tdet_en(0);
	
	upmu_set_rg_baton_en(0);
	upmu_set_rg_auxadc_chsel(2);

	printk("[polling vcdt]");
	data=PMIC_IMM_GetOneChannelValue(4,1,1);
	printk("[vcdt :%d \n",data);

	//BATON1

	upmu_set_rg_adc_swctrl_en(1);

	upmu_set_rg_adcin_vsen_en(0);
	upmu_set_rg_adcin_vbat_en(0);
	upmu_set_rg_adcin_vsen_mux_en(0);
	upmu_set_rg_adcin_vsen_ext_baton_en(0);
	upmu_set_rg_adcin_chr_en(0);
	upmu_set_baton_tdet_en(1);
	
	upmu_set_rg_baton_en(0);
	upmu_set_rg_auxadc_chsel(3);

	printk("[polling baton1]");
	data=PMIC_IMM_GetOneChannelValue(5,1,1);
	printk("[baton1 :%d \n",data);

	//THR_SENSE1

	upmu_set_rg_adc_swctrl_en(1);

	upmu_set_baton_tdet_en(1);
	upmu_set_rg_vbuf_calen(1);
	upmu_set_rg_vbuf_en(1);
	
	upmu_set_rg_auxadc_chsel(4);
	
	printk("[polling THR_SENSE1]");
	data=PMIC_IMM_GetOneChannelValue(2,1,1);
	printk("[THR_SENSE1 :%d \n",data);
	
	//THR_SENSE2

	upmu_set_rg_vbuf_en(1);
	upmu_set_rg_adc_swctrl_en(1);

	upmu_set_rg_adcin_vsen_en(0);
	upmu_set_rg_adcin_vbat_en(0);
	upmu_set_rg_adcin_vsen_mux_en(0);
	upmu_set_rg_adcin_vsen_ext_baton_en(1);
	upmu_set_rg_adcin_chr_en(0);
	upmu_set_baton_tdet_en(0);
	
	upmu_set_rg_baton_en(0);
	upmu_set_rg_vbuf_calen(1);
	upmu_set_rg_auxadc_chsel(4);

	printk("[polling THR SENSE2]");
	data=PMIC_IMM_GetOneChannelValue(3,1,1);
	printk("[THR_SENSE2 :%d \n",data);

	//BATON2

	upmu_set_rg_adc_swctrl_en(1);

	upmu_set_rg_adcin_vsen_en(0);
	upmu_set_rg_adcin_vbat_en(0);
	upmu_set_rg_adcin_vsen_mux_en(0);
	upmu_set_rg_adcin_vsen_ext_baton_en(1);
	upmu_set_rg_adcin_chr_en(0);
	upmu_set_baton_tdet_en(0);
	
	upmu_set_rg_baton_en(0);
	upmu_set_rg_vbuf_calen(0);
	upmu_set_rg_auxadc_chsel(3);

	printk("[polling baton2]");
	data=PMIC_IMM_GetOneChannelValue(0,1,1);
	printk("[baton2 :%d \n",data);
	
}





///////////////////////////////////////////////////////////////////////////////////
//
//  INTERRUPT TEST CASE
//
///////////////////////////////////////////////////////////////////////////////////
void pmic_UVVP_PMIC_INT_451(void)
{
    upmu_set_vio28_en(1);
    upmu_set_rg_vusb_en(1);
    upmu_set_rg_vmc_en(1);
    upmu_set_rg_vmch_en(1);
    upmu_set_rg_vemc_3v3_en(1);
    upmu_set_rg_vgp1_en(1);
    upmu_set_rg_vgp2_en(1);
    upmu_set_rg_vgp3_en(1);
    upmu_set_rg_vcn_1v8_en(1);
    upmu_set_rg_vsim1_en(1);
    upmu_set_rg_vsim2_en(1);
    upmu_set_vrtc_en(1);
    upmu_set_rg_vcam_af_en(1);
    upmu_set_rg_vibr_en(1);
    upmu_set_rg_vm_en(1);
    upmu_set_rg_vrf18_en(1);
    upmu_set_rg_vio18_en(1);
    upmu_set_rg_vcamd_en(1);
    upmu_set_rg_vcam_io_en(1);
    upmu_set_rg_vtcxo_en(1);
    upmu_set_rg_va_en(1);
    upmu_set_rg_vcama_en(1);
    upmu_set_rg_vcn33_en_bt(1);
    upmu_set_rg_vcn28_en(1);

    upmu_set_rg_intrp_ck_pdn(0); //for all interrupt events, turn on interrupt module clock
    upmu_set_rg_int_en_ldo(1);

    printk("[upmu_set_rg_int_en_ldo(1)]");
}
///////////////////////////////////////////////////////////////////////////////////
void pmic_UVVP_PMIC_INT_461(void)
{
    upmu_set_rg_intrp_ck_pdn(0);
    upmu_set_rg_pwmoc_ck_pdn(0); //for buck oc related interrupt , turn on pwmoc_ck
    upmu_set_rg_int_en_vproc(1);
    printk("[upmu_set_rg_int_en_vproc(1)]");
}
///////////////////////////////////////////////////////////////////////////////////
void pmic_UVVP_PMIC_INT_462(void)
{
    upmu_set_rg_intrp_ck_pdn(0);
    upmu_set_rg_pwmoc_ck_pdn(0); //for buck oc related interrupt , turn on pwmoc_ck
    upmu_set_rg_int_en_vsys(1);
    printk("[upmu_set_rg_int_en_vsys(1)]");
}
///////////////////////////////////////////////////////////////////////////////////
void pmic_UVVP_PMIC_INT_463(void)
{
   upmu_set_rg_intrp_ck_pdn(0);
    upmu_set_rg_pwmoc_ck_pdn(0); //for buck oc related interrupt , turn on pwmoc_ck
    upmu_set_rg_int_en_vpa(1);
    printk("[upmu_set_rg_int_en_vpa(1)]");
}
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////////
//
//  BUCK TEST CASE
//
///////////////////////////////////////////////////////////////////////////////////
int g_buck_num=100;
int g_log_buck_vosel=0;

extern int PMIC_IMM_GetOneChannelValue(int dwChannel, int deCount, int trimd);
void read_auxadc_value(void)
{
    int ret=0;
    
    if(g_buck_num == 0)
    {
        //vproc
        mdelay(1);
        if(g_log_buck_vosel==1)
        {
            printk("Reg[%x]=%d, Reg[%x]=%d\n", 
                0x21e, upmu_get_reg_value(0x21e),
                0x224, upmu_get_reg_value(0x224)
                );
        }
        else
        {        
            printk("Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x\n", 
                    0x120, upmu_get_reg_value(0x120),
                    0x216, upmu_get_reg_value(0x216),
                    0x220, upmu_get_reg_value(0x220),
                    0x222, upmu_get_reg_value(0x222),
                    0x224, upmu_get_reg_value(0x224)
                    );        
        }
    }
    else if(g_buck_num == 1)
    {
        //vsys
        mdelay(1);
        if(g_log_buck_vosel==1)
        {
            printk("Reg[%x]=%d, Reg[%x]=%d\n", 
                0x244, upmu_get_reg_value(0x244),
                0x24A, upmu_get_reg_value(0x24A)
                );
        }
        else
        {
            printk("Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x\n", 
                    0x120, upmu_get_reg_value(0x120),
                    0x23c, upmu_get_reg_value(0x23c),
                    0x246, upmu_get_reg_value(0x246),
                    0x248, upmu_get_reg_value(0x248),
                    0x24A, upmu_get_reg_value(0x24A)                    
                    );    
        }
    }
    else if(g_buck_num == 2)
    {
        //vpa
        mdelay(1);
        if(g_log_buck_vosel==1)
        {
            printk("Reg[%x]=%d, Reg[%x]=%d\n", 
                0x312, upmu_get_reg_value(0x312),
                0x318, upmu_get_reg_value(0x318)
                );
        }
        else
        {
            printk("Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x,Reg[%x]=0x%x\n", 
                    0x120, upmu_get_reg_value(0x120),
                    0x30a, upmu_get_reg_value(0x30a),
                    0x314, upmu_get_reg_value(0x314),
                    0x316, upmu_get_reg_value(0x316),
                    0x318, upmu_get_reg_value(0x318)
                    );    
        }
    }
    else 
    {
    }

    //measure in sleep
    upmu_set_rg_auxadc_sdm_ck_sel(1);
    upmu_set_strup_auxadc_start_sel(0);
    upmu_set_strup_auxadc_rstb_sw(1);
    upmu_set_strup_auxadc_rstb_sel(1);

    mdelay(20);
    ret = PMIC_IMM_GetOneChannelValue(5,1,0);    

    ret = ret ; // with J259 and use J258
    
    printk("[read_auxadc_value] ret = %d\n\n", ret);
}
///////////////////////////////////////////////////////////////////////////////////
#if 1
void do_scrxxx_map(int j)
{

        printk("[upmu_set_rg_srcvolt_en(1)] ");
        upmu_set_rg_srclken_en(1);
        read_auxadc_value();

        printk("[upmu_set_rg_srcvolt_en(0)] ");
        upmu_set_rg_srclken_en(0);
        read_auxadc_value();

        printk("[upmu_set_rg_srcvolt_en(1)] ");
        upmu_set_rg_srclken_en(1);
        read_auxadc_value();

}

void do_vproc_en_test(int index_val)
{
    int i,j=0;
    
    g_buck_num=0;
    
    for(i=0;i<=1;i++)
    {
        printk("[upmu_set_vproc_en_ctrl] %d\n", i);
        upmu_set_vproc_en_ctrl(i);        
        
        switch(i){
            case 0:
                printk("[upmu_set_vproc_en(0)] ");
                upmu_set_vproc_en(0);        
                read_auxadc_value();   
                
                printk("[upmu_set_vproc_en(1)] ");
                upmu_set_vproc_en(1);        
                read_auxadc_value();
                break;    

            case 1:    
                    do_scrxxx_map(j);
                break;

            default:
                printk("At %d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    } 

    g_buck_num=100;
}

void do_vsys_en_test(int index_val)
{
    int i,j=0;

    g_buck_num=1;
	pmic_config_interface(STRUP_CON7,0xff,0xff,0);
	
    for(i=0;i<=1;i++)
    {
        printk("[upmu_set_vsys_en_ctrl] %d\n", i);
        upmu_set_vsys_en_ctrl(i);        
        
        switch(i){
            case 0:
                printk("[upmu_set_vsys_en(0)] ");
                upmu_set_vsys_en(0);        
                read_auxadc_value();   
                
                printk("[upmu_set_vsys_en(1)] ");
                upmu_set_vsys_en(1);        
                read_auxadc_value();
                break;    

            case 1:    
                    do_scrxxx_map(j);
                break;

            default:
                printk("At %d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    } 

    g_buck_num=100;
}


void do_vpa_en_test(int index_val)
{
    int i,j=0;
    
    g_buck_num=2;
    
    for(i=0;i<=1;i++)
    {
        printk("[upmu_set_vpa_en_ctrl] %d\n", i);
        upmu_set_vpa_en_ctrl(i);        
        
        switch(i){
            case 0:
                printk("[upmu_set_vpa_en(0)] ");
                upmu_set_vpa_en(0);        
                read_auxadc_value();   
                
                printk("[upmu_set_vpa_en(1)] ");
                upmu_set_vpa_en(1);        
                read_auxadc_value();
                break;    

            case 1:    
                    do_scrxxx_map(j);              
                break;

            default:
                printk("At %d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    }  

    g_buck_num=100;
}
#endif
//////////////////////////////////////////////////////////////////////
#if 1
void do_vproc_vosel_subtest(void)
{
    int i;

    for(i=0;i<=PMIC_VPROC_VOSEL_SLEEP_MASK;i++) 
    { 
        upmu_set_vproc_vosel_sleep(i); 
        printk("[upmu_set_vproc_vosel_sleep] i=%d, ",i);
        if(i==0)
            mdelay(500);
        g_buck_num=0;
        read_auxadc_value();
        g_buck_num=100;
    }
    
    for(i=0;i<=PMIC_VPROC_VOSEL_ON_MASK;i++) 
    { 
        upmu_set_vproc_vosel_on(i); 
        printk("[upmu_set_vproc_vosel_on] i=%d, ",i);
        if(i==0)
            mdelay(500);
        g_buck_num=0;
        read_auxadc_value();
        g_buck_num=100;
    }    
    
}

void do_vsys_vosel_subtest(void)
{
    int i;

    for(i=0;i<=PMIC_VSYS_VOSEL_SLEEP_MASK;i++) 
    { 
        upmu_set_vsys_vosel_sleep(i); 
        printk("[upmu_set_vsram_vosel_sleep] i=%d, ",i);
        if(i==0)
            mdelay(500);
        g_buck_num=1;
        read_auxadc_value();
        g_buck_num=100;
    }
    for(i=0;i<=PMIC_VSYS_VOSEL_ON_MASK;i++) 
    { 
        upmu_set_vsys_vosel_on(i); 
        printk("[upmu_set_vsram_vosel_on] i=%d, ",i);
        if(i==0)
            mdelay(500);
        g_buck_num=1;
        read_auxadc_value();
        g_buck_num=100;
    }
    
}



void do_vpa_vosel_subtest(void)
{
    int i;
    
    for(i=0;i<=PMIC_VPA_VOSEL_SLEEP_MASK;i++) 
    { 
        upmu_set_vpa_vosel_sleep(i); 
        printk("[upmu_set_vpa_vosel_sleep] i=%d, ",i);
        if(i==0)
            mdelay(500);
        g_buck_num=2;
        read_auxadc_value();
        g_buck_num=100;
    }
    for(i=0;i<=PMIC_VPA_VOSEL_ON_MASK;i++) 
    { 
        upmu_set_vpa_vosel_on(i); 
        printk("[upmu_set_vpa_vosel_on] i=%d, ",i);
        if(i==0)
            mdelay(500);
        g_buck_num=2;
        read_auxadc_value();
        g_buck_num=100;
    }
}



void do_vosel_subtest(int index_val)
{
    g_log_buck_vosel=1;

    switch(index_val){
        case 0:
            do_vproc_vosel_subtest();
            break;
        case 1:
            do_vsys_vosel_subtest();
            break;
        case 2:
            do_vpa_vosel_subtest();
            break;
        default:
            printk("[do_vosel_subtest] Invalid channel value(%d)\n", index_val);
            break;     
    }

    g_log_buck_vosel=0;
}

void do_scrxxx_map_vosel(int index_val)
{

                printk("[upmu_set_rg_srclken_en(0)]\n");
                upmu_set_rg_srclken_en(0);
                do_vosel_subtest(index_val);

                printk("[upmu_set_rg_srclken_en(1)]\n");
                upmu_set_rg_srclken_en(1);
                do_vosel_subtest(index_val);

}

void do_vproc_vosel_test(int index_val)
{
    int i,j;
    
    for(i=0;i<=1;i++)
    {
        printk("[upmu_set_vproc_vosel_ctrl] %d\n", i);
        upmu_set_vproc_vosel_ctrl(i);        
        
        switch(i){
            case 0:
                for(j=0;j<=PMIC_VPROC_VOSEL_MASK;j++)
                {
                    upmu_set_vproc_vosel(j);
                    printk("[upmu_set_vproc_vosel] j=%d, ",j);
                    if(j==0)
                        mdelay(500);
                    read_auxadc_value();
                }
                break;    

            case 1:
                do_scrxxx_map_vosel(index_val);
                break;

            default:
                printk("At %d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    }  
}

void do_vsys_vosel_test(int index_val)
{
    int i,j;
    
    for(i=0;i<=1;i++)
    {
        printk("[upmu_set_vsys_vosel_ctrl] %d\n", i);
        upmu_set_vsys_vosel_ctrl(i);        
        
        switch(i){
            case 0:
                for(j=0;j<=PMIC_VSYS_VOSEL_MASK;j++)
                {
                    upmu_set_vsys_vosel(j);
                    printk("[upmu_set_vsys_vosel] j=%d, ",j);
                    if(j==0)
                        mdelay(500);
                    read_auxadc_value();
                }
                break;    

            case 1:
                do_scrxxx_map_vosel(index_val);
                break;

            default:
                printk("At %d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    }  
}

void do_vpa_vosel_test(int index_val)
{
    int i,j;
        
    for(i=0;i<=1;i++)
    {
        printk("[upmu_set_vpa_vosel_ctrl] %d\n", i);
        upmu_set_vpa_vosel_ctrl(i);        
        
        switch(i){
            case 0:
                for(j=0;j<=PMIC_VPA_VOSEL_MASK;j++)
                {
                    upmu_set_vpa_vosel(j);
                    printk("[upmu_set_vpa_vosel] j=%d, ",j);
                    if(j==0)
                        mdelay(500);
                    read_auxadc_value();
                }
                break;    

            case 1:
                do_scrxxx_map_vosel(index_val);
                break;

            default:
                printk("At %d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    }    
}

#endif
//////////////////////////////////////////////////////////////////////
#if 1
void do_vproc_dlc_subtest(void)
{
    int i; 

    printk("[do_vproc_dlc_subtest]\n");
    
    for(i=0;i<=PMIC_VPROC_DLC_SLEEP_MASK;i++)
    {
        upmu_set_vproc_dlc_sleep(i);
        upmu_set_vproc_dlc_n_sleep(i);

        printk("[do_vproc_dlc_subtest] upmu_set_vproc_dlc_sleep=%d, upmu_set_vproc_dlc_n_sleep=%d\n", i, i);

        printk("[do_vproc_dlc_subtest] upmu_get_qi_vproc_dlc=%d, upmu_get_qi_vproc_dlc_n=%d\n",
            upmu_get_qi_vproc_dlc(), upmu_get_qi_vproc_dlc_n());

    }

    printk("\n");

    for(i=0;i<=PMIC_VPROC_DLC_ON_MASK;i++)
    {
        upmu_set_vproc_dlc_on(i);
        upmu_set_vproc_dlc_n_on(i);

        printk("[do_vproc_dlc_subtest] upmu_set_vproc_dlc_on=%d, upmu_set_vproc_dlc_n_on=%d\n", i, i);

        printk("[do_vproc_dlc_subtest] upmu_get_qi_vproc_dlc=%d, upmu_get_qi_vproc_dlc_n=%d\n",
            upmu_get_qi_vproc_dlc(), upmu_get_qi_vproc_dlc_n());                   
    }

    printk("\n");

}
void do_vsys_dlc_subtest(void)
{
    int i; 
    
    printk("[do_vsram_dlc_subtest]\n");
    
    for(i=0;i<=PMIC_VSYS_DLC_SLEEP_MASK;i++)
    {
        upmu_set_vsys_dlc_sleep(i);
        upmu_set_vsys_dlc_n_sleep(i);
    
        printk("[do_vsys_dlc_subtest] upmu_set_vsys_dlc_sleep=%d, upmu_set_vsys_dlc_n_sleep=%d\n", i, i);
    
        printk("[do_vsys_dlc_subtest] upmu_get_qi_vsys_dlc=%d, upmu_get_qi_vsys_dlc_n=%d\n",
            upmu_get_qi_vsys_dlc(), upmu_get_qi_vsys_dlc_n());
    
    }

    printk("\n");
    
    for(i=0;i<=PMIC_VSYS_DLC_ON_MASK;i++)
    {
        upmu_set_vsys_dlc_on(i);
        upmu_set_vsys_dlc_n_on(i);
    
        printk("[do_vsys_dlc_subtest] upmu_set_vsys_dlc_on=%d, upmu_set_vsys_dlc_n_on=%d\n", i, i);
    
        printk("[do_vsys_dlc_subtest] upmu_get_qi_vsys_dlc=%d, upmu_get_qi_vsys_dlc_n=%d\n",
            upmu_get_qi_vsys_dlc(), upmu_get_qi_vsys_dlc_n());                   
    }

    printk("\n");

}

void do_vpa_dlc_subtest(void)
{
    int i; 
    
    printk("[do_vpa_dlc_subtest]\n");
    
    for(i=0;i<=PMIC_VPA_DLC_SLEEP_MASK;i++)
    {
        upmu_set_vpa_dlc_sleep(i);
    
        printk("[do_vpa_dlc_subtest] upmu_set_vpa_dlc_sleep=%d\n", i);
    
        printk("[do_vpa_dlc_subtest] upmu_get_qi_vpa_dlc=%d\n",
            upmu_get_qi_vpa_dlc());
    
    }

    printk("\n");
    
    for(i=0;i<=PMIC_VPA_DLC_ON_MASK;i++)
    {
        upmu_set_vpa_dlc_on(i);
    
        printk("[do_vpa_dlc_subtest] upmu_set_vpa_dlc_on=%d\n", i);
    
        printk("[do_vpa_dlc_subtest] upmu_get_qi_vpa_dlc=%d\n",
            upmu_get_qi_vpa_dlc());                   
    }

    printk("\n");

}


void do_dlc_subtest(int index_val)
{
    switch(index_val){
        case 0:
            do_vproc_dlc_subtest();
            break;
        case 1:
            do_vsys_dlc_subtest();
            break;
        case 2:
            do_vpa_dlc_subtest();
            break;
        default:
            printk("[do_dlc_subtest] Invalid channel value(%d)\n", index_val);
            break;     
    }
}

void do_scrxxx_map_dlc(int index_val)
{
//    int j;
    

    printk("[upmu_set_rg_srclken_en(0)]\n");
    upmu_set_rg_srclken_en(0);
    do_dlc_subtest(index_val);

    printk("[upmu_set_rg_srclken_en(1)]\n");
    upmu_set_rg_srclken_en(1);
    do_dlc_subtest(index_val);
              
   
}

void do_vproc_dlc_test(int index_val)
{
    int i,j;
    
    for(i=0;i<=1;i++)
    {
        printk("[do_vproc_dlc_test] %d\n", i);
        upmu_set_vproc_dlc_ctrl(i);        //0: sw mode , 1: hw mode
        
        switch(i){
            case 0:
                for(j=0;j<=PMIC_VPROC_DLC_MASK;j++)
                {
                    upmu_set_vproc_dlc(j);
                    upmu_set_vproc_dlc_n(j);

                    if( (upmu_get_qi_vproc_dlc()!=j) || (upmu_get_qi_vproc_dlc_n()!=j) )
                    {
                        printk("[do_vproc_dlc_test] fail at upmu_get_qi_vproc_dlc=%d, upmu_get_qi_vproc_dlc_n=%d\n",
                            upmu_get_qi_vproc_dlc(), upmu_get_qi_vproc_dlc_n());
                    }

                    printk("[do_vproc_dlc_test] upmu_set_vproc_dlc=%d, upmu_set_vproc_dlc_n=%d, upmu_get_qi_vproc_dlc=%d, upmu_get_qi_vproc_dlc_n=%d\n",
                            j, j, upmu_get_qi_vproc_dlc(), upmu_get_qi_vproc_dlc_n());
                }
                break;    

            case 1:
                do_scrxxx_map_dlc(index_val);
                break;

            default:
                printk("At %d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    }    
}
void do_vsys_dlc_test(int index_val)
{
    int i,j;
        
    for(i=0;i<=1;i++)
    {
        printk("[do_vsys_dlc_test] %d\n", i);
        upmu_set_vsys_dlc_ctrl(i);        
        
        switch(i){
            case 0:
                for(j=0;j<=PMIC_VSYS_DLC_MASK;j++)
                {
                    upmu_set_vsys_dlc(j);
                    upmu_set_vsys_dlc_n(j);

                    if( (upmu_get_qi_vsys_dlc()!=j) || (upmu_get_qi_vsys_dlc_n()!=j) )
                    {
                        printk("[do_vsys_dlc_test] fail at upmu_get_qi_vsys_dlc=%d, upmu_get_qi_vsys_dlc_n=%d\n",
                            upmu_get_qi_vsys_dlc(), upmu_get_qi_vsys_dlc_n());
                    }

                    printk("[do_vsys_dlc_test] upmu_set_vsys_dlc=%d, upmu_set_vsys_dlc_n=%d, upmu_get_qi_vsys_dlc=%d, upmu_get_qi_vsys_dlc_n=%d\n",
                            j, j, upmu_get_qi_vsys_dlc(), upmu_get_qi_vsys_dlc_n());
                }
                break;    

            case 1:
                do_scrxxx_map_dlc(index_val);
                break;

            default:
                printk("At %d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    } 
}

void do_vpa_dlc_test(int index_val)
{
    int i,j;
        
    for(i=0;i<=1;i++)
    {
        printk("[do_vpa_dlc_test] %d\n", i);
        upmu_set_vpa_dlc_ctrl(i);        
        
        switch(i){
            case 0:
                for(j=0;j<=PMIC_VPA_DLC_MASK;j++)
                {
                    upmu_set_vpa_dlc(j);
    
                    if( (upmu_get_qi_vpa_dlc()!=j) )
                    {
                        printk("[do_vpa_dlc_test] fail at upmu_get_qi_vpa_dlc=%d\n",
                            upmu_get_qi_vpa_dlc());
                    }

                    printk("[do_vpa_dlc_test] upmu_set_vpa_dlc=%d, upmu_get_qi_vpa_dlc=%d\n",
                            j, upmu_get_qi_vpa_dlc());
                }
                break;    
    
            case 1:
                do_scrxxx_map_dlc(index_val);
                break;
    
            default:
                printk("At %d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    } 

    //------------------------------------------------------------------------
    printk("[do_vpa_dlc_test] special case : upmu_set_vpa_dlc_map_en(1);\n");    
    upmu_set_vpa_dlc_map_en(1);
    upmu_set_vpa_vosel_ctrl(0);
    for(i=0;i<=PMIC_VPA_VOSEL_MASK;i++)
    {
        upmu_set_vpa_vosel(i);
        printk("upmu_set_vpa_vosel=%d, upmu_get_qi_vpa_dlc=%d\n", i, upmu_get_qi_vpa_dlc());
    }
    //------------------------------------------------------------------------
    
}

#endif

#if 1

void do_vproc_burst_subtest(void)
{
    int i; 

    printk("[do_vproc_burst_subtest]\n");

    for(i=0;i<=PMIC_VPROC_BURST_SLEEP_MASK;i++)
    {
        upmu_set_vproc_burst_sleep(i);

        printk("[do_vproc_burst_subtest] upmu_set_vproc_bursth_sleep=%d\n", i);

        printk("[do_vproc_burst_subtest] upmu_get_qi_vproc_burst=%d\n",
            upmu_get_qi_vproc_burst());

    }

    printk("\n");

    for(i=0;i<=PMIC_VPROC_BURST_ON_MASK;i++)
    {
        upmu_set_vproc_burst_on(i);

        printk("[do_vpa_burst_subtest] upmu_set_vproc_burst_on=%d\n", i);

        printk("[do_vpa_burst_subtest] upmu_get_qi_vproc_burst=%d\n",
            upmu_get_qi_vproc_burst());                   
    }  

    printk("\n");

}

void do_vsys_burst_subtest(void)
{
    int i; 

    printk("[do_vsys_burst_subtest]\n");

    for(i=0;i<=PMIC_VSYS_BURST_SLEEP_MASK;i++)
    {
        upmu_set_vsys_burst_sleep(i);

        printk("[do_vsys_burst_subtest] upmu_set_vsys_bursth_sleep=%d\n", i);

        printk("[do_vsys_burst_subtest] upmu_get_qi_vsys_burst=%d\n",
            upmu_get_qi_vsys_burst());

    }

    printk("\n");

    for(i=0;i<=PMIC_VSYS_BURST_ON_MASK;i++)
    {
        upmu_set_vsys_burst_on(i);

        printk("[do_vsys_burst_subtest] upmu_set_vsys_burst_on=%d\n", i);

        printk("[do_vsys_burst_subtest] upmu_get_qi_vsys_burst=%d\n",
            upmu_get_qi_vsys_burst());                   
    }  

    printk("\n");

}

void do_vpa_burst_subtest(void)
{
    int i; 

    printk("[do_vpa_burst_subtest]\n");

    for(i=0;i<=PMIC_VPA_BURSTH_SLEEP_MASK;i++)
    {
        upmu_set_vpa_bursth_sleep(i);
        upmu_set_vpa_burstl_sleep(i);

        printk("[do_vpa_burst_subtest] upmu_set_vpa_bursth_sleep=%d, upmu_set_vpa_burstl_sleep=%d\n", i, i);

        printk("[do_vpa_burst_subtest] upmu_get_qi_vpa_burst=%d, upmu_get_qi_vpa_burst_n=%d\n",
            upmu_get_qi_vpa_bursth(), upmu_get_qi_vpa_burstl());

    }

    printk("\n");

    for(i=0;i<=PMIC_VPA_BURSTH_ON_MASK;i++)
    {
        upmu_set_vpa_bursth_on(i);
        upmu_set_vpa_burstl_on(i);

        printk("[do_vpa_burst_subtest] upmu_set_vpa_bursth_on=%d, upmu_set_vpa_burstl_on=%d\n", i, i);

        printk("[do_vpa_burst_subtest] upmu_get_qi_vpa_burst=%d, upmu_get_qi_vpa_burst_n=%d\n",
            upmu_get_qi_vpa_bursth(), upmu_get_qi_vpa_burstl());                   
    }  

    printk("\n");

}


void do_burst_subtest(int index_val)
{
    switch(index_val){
        case 0:
            do_vproc_burst_subtest();
            break;
        case 1:
            do_vsys_burst_subtest();
            break;
        case 2:
            do_vpa_burst_subtest();
            break;
        default:
            printk("[do_burst_subtest] Invalid channel value(%d)\n", index_val);
            break;     
    }
}

void do_scrxxx_map_bursth(int index_val)
{

                printk("[upmu_set_rg_srcvolt_en(0)]\n");
                upmu_set_rg_srclken_en(0);
                do_burst_subtest(index_val);

                printk("[upmu_set_rg_srcvolt_en(1)]\n");
                upmu_set_rg_srclken_en(1);
                do_burst_subtest(index_val);
           
                
}

void do_vproc_burst_test(int index_val)
{
    int i,j;
    
    for(i=0;i<=1;i++)
    {
        printk("[do_vproc_burst_test] %d\n", i);
        upmu_set_vproc_burst_ctrl(i);        
        
        switch(i){
            case 0:
                for(j=0;j<=PMIC_VPROC_BURST_MASK;j++)
                {
                    upmu_set_vproc_burst(j);

                    if( (upmu_get_qi_vproc_burst()!=j) )
                    {
                        printk("[do_vproc_burst_test] fail at upmu_get_qi_vproc_burst=%d\n",
                            upmu_get_qi_vproc_burst());
                    }

                    printk("[do_vproc_burst_test] upmu_set_vproc_burst=%d, upmu_get_qi_vproc_burst=%d\n",
                            j, upmu_get_qi_vproc_burst());
                }
                break;    

            case 1:
                do_scrxxx_map_bursth(index_val);
                break;

            default:
                printk("At %d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    }    
}
void do_vsys_burst_test(int index_val)
{
    int i,j;
        
    for(i=0;i<=1;i++)
    {
        printk("[do_vsram_burst_test] %d\n", i);
        upmu_set_vsys_burst_ctrl(i);        
        
        switch(i){
            case 0:
                for(j=0;j<=PMIC_VSYS_BURST_MASK;j++)
                {
                    upmu_set_vsys_burst(j);

                    if( (upmu_get_qi_vsys_burst()!=j) )
                    {
                        printk("[do_vsram_burst_test] fail at upmu_get_qi_vsram_burst=%d\n",
                            upmu_get_qi_vsys_burst());
                    }

                    printk("[do_vsram_burst_test] upmu_set_vsram_burst=%d, upmu_get_qi_vsram_burst=%d\n",
                            j, upmu_get_qi_vsys_burst());
                }
                break;    

            case 1:
                do_scrxxx_map_bursth(index_val);
                break;

            default:
                printk("At %d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    } 
}

void do_vpa_burst_test(int index_val)
{
    int i,j;
        
    for(i=0;i<=1;i++)
    {
        printk("[do_vpa_burst_test] %d\n", i);
        upmu_set_vpa_burst_ctrl(i);        
        
        switch(i){
            case 0:
                for(j=0;j<=PMIC_VPA_BURSTH_MASK;j++)
                {
                    upmu_set_vpa_bursth(j);
                    upmu_set_vpa_burstl(j);
    
                    if( (upmu_get_qi_vpa_bursth()!=j) || (upmu_get_qi_vpa_burstl()!=j) )
                    {
                        printk("[do_vpa_burst_test] fail at upmu_get_qi_vpa_burst=%d, upmu_get_qi_vpa_burst_n=%d\n",
                            upmu_get_qi_vpa_bursth(), upmu_get_qi_vpa_burstl());
                    }

                    printk("[do_vpa_burst_test] upmu_set_vpa_bursth=%d, upmu_set_vpa_burstl=%d, upmu_get_qi_vpa_bursth=%d, upmu_get_qi_vpa_burstl=%d\n",
                            j, j, upmu_get_qi_vpa_bursth(), upmu_get_qi_vpa_burstl());
                }
                break;    
    
            case 1:
                do_scrxxx_map_bursth(index_val);
                break;
    
            default:
                printk("At %d, Invalid channel value(%d)\n", index_val, i);
                break;    
        }            
    } 
    
}

#endif

void do_vtcxo_test(void)
{
	int i=0;
	int status=0;

	upmu_set_rg_srclken_hw_mode(0);
	
	upmu_set_vtcxo_lp_sel(1);


	
	for(i=0;i<2000;i++)
	{
		upmu_set_rg_srclken_en(0);//in sleep
              if(upmu_get_qi_vtcxo_mode()!=1)
              {
                  printk("[do_vtcxo_test] fail at upmu_get_qi_vtcxo_mode=%d in sleep mode\n",
                      upmu_get_qi_vpa_bursth());
		    status=1;
              }		

		upmu_set_rg_srclken_en(1);//in sleep
              if(upmu_get_qi_vtcxo_mode()!=0)
              {
                  printk("[do_vtcxo_test] fail at upmu_get_qi_vtcxo_mode=%d in non sleep mode\n",
                      upmu_get_qi_vpa_bursth());
		    status=1;
              }	
	}

	if (status==1)
		printk("[do_vtcxo_test] fail \n");
	else
		printk("[do_vtcxo_test] success \n");

}



void pmic_UVVP_PMIC_BUCK_ON_OFF(int index_val)
{   
    printk("[pmic_UVVP_PMIC_BUCK_ON_OFF] start....\n");

    upmu_set_rg_srclken_hw_mode(0); // 0:SW control

    switch(index_val){
      case 0:
        do_vproc_en_test(index_val);
        break;

      case 1:
        do_vsys_en_test(index_val);
        break;

      case 2:
        do_vpa_en_test(index_val);
        break;
       
	  default:
        printk("[pmic_UVVP_PMIC_BUCK_ON_OFF] Invalid channel value(%d)\n", index_val);
        break;
        
    }
    
    printk("[pmic_UVVP_PMIC_BUCK_ON_OFF] end....\n");
}
///////////////////////////////////////////////////////////////////////////////////
void pmic_UVVP_PMIC_BUCK_VOSEL(int index_val)
{
//    int i=0; 
    
    printk("[pmic_UVVP_PMIC_BUCK_VOSEL] start....\n");

    upmu_set_rg_srclken_hw_mode(0); // 0:SW control

    upmu_set_vproc_en_ctrl(0);
    upmu_set_vsys_en_ctrl(0);
    upmu_set_vpa_en_ctrl(0);

    //upmu_set_rg_vproc_modeset(1);
    //upmu_set_rg_vpa_modeset(1);
    //upmu_set_rg_vsys_modeset(1);	


    switch(index_val){
      case 0:
        upmu_set_vproc_en(1);
        g_buck_num=0;
        g_log_buck_vosel=1;
        do_vproc_vosel_test(index_val);
        g_buck_num=100;
        g_log_buck_vosel=0;
        break;

      case 1:
        upmu_set_vsys_en(1);
        g_buck_num=1;
        g_log_buck_vosel=1;
        do_vsys_vosel_test(index_val);
        g_buck_num=100;
        g_log_buck_vosel=0;
        break;

      case 2:
        upmu_set_vpa_en(1);
        g_buck_num=2;
        g_log_buck_vosel=1;
        do_vpa_vosel_test(index_val);
        g_buck_num=100;
        g_log_buck_vosel=0;
        break;

       
	  default:
        printk("[pmic_UVVP_PMIC_BUCK_VOSEL] Invalid channel value(%d)\n", index_val);
        break;
        
    }
    
    printk("[pmic_UVVP_PMIC_BUCK_VOSEL] end....\n");
}
///////////////////////////////////////////////////////////////////////////////////
void pmic_UVVP_PMIC_BUCK_DLC(int index_val)
{
//    int i=0; 
    
    printk("[pmic_UVVP_PMIC_DLC_VOSEL] start....\n");

    upmu_set_rg_srclken_hw_mode(0);

    upmu_set_vproc_en_ctrl(0);
    upmu_set_vsys_en_ctrl(0);
    upmu_set_vpa_en_ctrl(0);

    switch(index_val){
      case 0:
        upmu_set_vproc_en(1);
        do_vproc_dlc_test(index_val);
        break;

      case 1:
        upmu_set_vsys_en(1);
        do_vsys_dlc_test(index_val);
        break;

      case 2:
        upmu_set_vpa_en(1);
        do_vpa_dlc_test(index_val);
        break;
       
	  default:
        printk("[pmic_UVVP_PMIC_BUCK_DLC] Invalid channel value(%d)\n", index_val);
        break;
        
    }
    
    printk("[pmic_UVVP_PMIC_BUCK_DLC] end....\n");
}
///////////////////////////////////////////////////////////////////////////////////
void pmic_UVVP_PMIC_BUCK_BURST(int index_val)
{
    //int i=0; 
    
    printk("[pmic_UVVP_PMIC_BUCK_BURST] start....\n");

    upmu_set_rg_srclken_hw_mode(0); // 0:SW control

    upmu_set_vproc_en_ctrl(0);
    upmu_set_vsys_en_ctrl(0);
    upmu_set_vpa_en_ctrl(0);


    switch(index_val){
      case 0:
        upmu_set_vproc_en(1);
        do_vproc_burst_test(index_val);
        break;

      case 1:
        upmu_set_vsys_en(1);
        do_vsys_burst_test(index_val);
        break;

      case 2:
        upmu_set_vpa_en(1);
        do_vpa_burst_test(index_val);
        break;

	  default:
        printk("[pmic_UVVP_PMIC_BUCK_BURST] Invalid channel value(%d)\n", index_val);
        break;
        
    }
    
    printk("[pmic_UVVP_PMIC_BUCK_BURST] end....\n");
}


///////////////////////////////////////////////////////////////////////////////////
//
//  LDO TEST CASE
//
///////////////////////////////////////////////////////////////////////////////////
void pmic_UVVP_PMIC_LDO_ON_OFF(int index_val)
{
    printk("[pmic_UVVP_PMIC_LDO_ON_OFF] start....\n");

    switch(index_val){
      case 0:
        hwPowerOn(MT6323_POWER_LDO_VIO28,    VOL_DEFAULT, "ldo_test");    
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VIO28,     "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VIO28,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();        
        hwPowerDown(MT6323_POWER_LDO_VIO28,     "ldo_test");
        read_auxadc_value();
        break;

      case 1:
        hwPowerOn(MT6323_POWER_LDO_VUSB,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VUSB,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VUSB,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VUSB,      "ldo_test");
        read_auxadc_value();
        break;

      case 2:
        hwPowerOn(MT6323_POWER_LDO_VMC,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VMC,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VMC,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VMC,      "ldo_test");
        read_auxadc_value();
        break;

      case 3:
        hwPowerOn(MT6323_POWER_LDO_VMCH,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VMCH,     "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VMCH,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VMCH,     "ldo_test");
        read_auxadc_value();
        break;

      case 4:
        hwPowerOn(MT6323_POWER_LDO_VEMC_3V3, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VEMC_3V3,  "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VEMC_3V3, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VEMC_3V3,  "ldo_test");
        read_auxadc_value();
        break;

      case 5:
        hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP1,  "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP1,  "ldo_test");
        read_auxadc_value();
        break;

      case 6:
        hwPowerOn(MT6323_POWER_LDO_VGP2,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP2,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP2,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP2,      "ldo_test");
        read_auxadc_value();
        break;

      case 7:  
        hwPowerOn(MT6323_POWER_LDO_VGP3,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP3,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP3,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP3,      "ldo_test");
        read_auxadc_value();
        break;

      case 8:  
        hwPowerOn(MT6323_POWER_LDO_VCN_1V8,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCN_1V8,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCN_1V8,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCN_1V8,      "ldo_test");
        read_auxadc_value();        
        break;

      case 9:  
        hwPowerOn(MT6323_POWER_LDO_VSIM1,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VSIM1,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VSIM1,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VSIM1,      "ldo_test");
        read_auxadc_value();
        break;

      case 10:  
        hwPowerOn(MT6323_POWER_LDO_VSIM2,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VSIM2,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VSIM2,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VSIM2,      "ldo_test");
        read_auxadc_value();
        break;

      case 11:  
        hwPowerOn(MT6323_POWER_LDO_VRTC,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VRTC,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VRTC,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VRTC,      "ldo_test");
        read_auxadc_value();
        break;

      case 12:  
        hwPowerOn(MT6323_POWER_LDO_VCAM_AF,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAM_AF,     "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAM_AF,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAM_AF,     "ldo_test");
        read_auxadc_value();
        break;

      case 13:
        hwPowerOn(MT6323_POWER_LDO_VIBR,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VIBR,     "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VIBR,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VIBR,     "ldo_test");
        read_auxadc_value();
        break;

      case 14:  
        hwPowerOn(MT6323_POWER_LDO_VM,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VM,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VM,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VM,      "ldo_test");
        read_auxadc_value();
        break;

      case 15:  
        hwPowerOn(MT6323_POWER_LDO_VRF18,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VRF18,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VRF18,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VRF18,      "ldo_test");
        read_auxadc_value();
        break;

      case 16:  
        hwPowerOn(MT6323_POWER_LDO_VIO18,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VIO18,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VIO18,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VIO18,      "ldo_test");
        read_auxadc_value();
        break;

      case 17:  
        hwPowerOn(MT6323_POWER_LDO_VCAMD,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAMD,     "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAMD,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAMD,     "ldo_test");
        read_auxadc_value();        
        break;

      case 18:  
        hwPowerOn(MT6323_POWER_LDO_VCAM_IO,  VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAM_IO,   "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAM_IO,  VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAM_IO,   "ldo_test");
        read_auxadc_value();
        break;

      case 19:  
	hwPowerOn(MT6323_POWER_LDO_VTCXO,    VOL_DEFAULT, "ldo_test");
	read_auxadc_value();
	hwPowerDown(MT6323_POWER_LDO_VTCXO,     "ldo_test");
	read_auxadc_value();
	hwPowerOn(MT6323_POWER_LDO_VTCXO,    VOL_DEFAULT, "ldo_test");
	read_auxadc_value();
	hwPowerDown(MT6323_POWER_LDO_VTCXO,     "ldo_test");
	read_auxadc_value();

	//special test
	hwPowerOn(MT6323_POWER_LDO_VTCXO,    VOL_DEFAULT, "ldo_test");
	do_vtcxo_test();
	hwPowerDown(MT6323_POWER_LDO_VTCXO,     "ldo_test");
		
        break;

      case 20:  
        hwPowerOn(MT6323_POWER_LDO_VA,  VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VA,   "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VA,  VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VA,   "ldo_test");
        read_auxadc_value();        
        break;

      case 21:  
        hwPowerOn(MT6323_POWER_LDO_VCAMA,       VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAMA,        "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAMA,       VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAMA,        "ldo_test");
        read_auxadc_value();
        break;

      case 22:  
        hwPowerOn(MT6323_POWER_LDO_VCN33_BT,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCN33_BT,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCN33_BT,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCN33_BT,      "ldo_test");
        read_auxadc_value();
        break;

      case 23:  
        hwPowerOn(MT6323_POWER_LDO_VCN28,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCN28,     "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCN28,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCN28,     "ldo_test");
        read_auxadc_value();
        break;
        
	  default:
        printk("[pmic_UVVP_PMIC_LDO_ON_OFF] Invalid channel value(%d)\n", index_val);
        break;
        
    }

    printk("[pmic_UVVP_PMIC_LDO_ON_OFF] end....\n");
    
}



///////////////////////////////////////////////////////////////////////////////////
void pmic_UVVP_PMIC_LDO_VOSEL(int index_val)
{
    printk("[pmic_UVVP_PMIC_LDO_VOSEL] start....\n");

    switch(index_val){
      case 0:
        hwPowerOn(MT6323_POWER_LDO_VIO28,    VOL_DEFAULT, "ldo_test");    
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VIO28,      "ldo_test");
        read_auxadc_value();
        break;

      case 1:
        hwPowerOn(MT6323_POWER_LDO_VUSB,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VUSB,      "ldo_test");
        read_auxadc_value();
        break;

      case 2:
        hwPowerOn(MT6323_POWER_LDO_VMC,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VMC,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VMC,     VOL_1800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VMC,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VMC,     VOL_3300, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VMC,      "ldo_test");
        read_auxadc_value();
        break;

      case 3:
        hwPowerOn(MT6323_POWER_LDO_VMCH,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VMCH,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VMCH,    VOL_3000, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VMCH,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VMCH,    VOL_3300, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VMCH,      "ldo_test");
        read_auxadc_value();
        break;

      case 4:
        hwPowerOn(MT6323_POWER_LDO_VEMC_3V3, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VEMC_3V3,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VEMC_3V3, VOL_3000, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VEMC_3V3,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VEMC_3V3, VOL_3300, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VEMC_3V3,      "ldo_test");
        read_auxadc_value();
        break;

      case 5:
        hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP1,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_1200, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP1,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_1300, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP1,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_1500, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP1,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_1800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP1,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_2500, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP1,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_2800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP1,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_3000, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP1,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_3300, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP1,      "ldo_test");
        read_auxadc_value();
        break;

      case 6:       
        hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP2,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_1200, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP2,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_1300, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP2,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_1500, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP2,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_1800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP2,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_2500, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP2,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_2800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP2,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_3000, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP2,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP2, VOL_3300, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP2,      "ldo_test");
        read_auxadc_value();
        break;

      case 7:  
        hwPowerOn(MT6323_POWER_LDO_VGP3, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP3,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP3, VOL_1200, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP3,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP3, VOL_1300, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP3,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP3, VOL_1500, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP3,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP3, VOL_1800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP3,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP3, VOL_2500, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP3,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP3, VOL_2800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP3,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP3, VOL_3000, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP3,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VGP3, VOL_3300, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VGP3,      "ldo_test");
        read_auxadc_value();
        break;

      case 8:
        hwPowerOn(MT6323_POWER_LDO_VCN_1V8,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCN_1V8,      "ldo_test");
        read_auxadc_value();
        break;

      case 9:  
        hwPowerOn(MT6323_POWER_LDO_VSIM1, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VSIM1,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VSIM1, VOL_1800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VSIM1,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VSIM1, VOL_3000, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VSIM1,      "ldo_test");
        read_auxadc_value();
        break;

      case 10:  
        hwPowerOn(MT6323_POWER_LDO_VSIM2, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VSIM2,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VSIM2, VOL_1800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VSIM2,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VSIM2, VOL_3000, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VSIM2,      "ldo_test");
        read_auxadc_value();
        break;

      case 11:
        hwPowerOn(MT6323_POWER_LDO_VRTC,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VRTC,      "ldo_test");
        read_auxadc_value();
        break;

      case 12:  
        hwPowerOn(MT6323_POWER_LDO_VCAM_AF, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAM_AF,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAM_AF, VOL_1200, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAM_AF,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAM_AF, VOL_1300, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAM_AF,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAM_AF, VOL_1500, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAM_AF,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAM_AF, VOL_1800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAM_AF,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAM_AF, VOL_2500, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAM_AF,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAM_AF, VOL_2800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAM_AF,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAM_AF, VOL_3000, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAM_AF,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAM_AF, VOL_3300, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAM_AF,      "ldo_test");
        read_auxadc_value();
        break;

      case 13:  
        hwPowerOn(MT6323_POWER_LDO_VIBR, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VIBR,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VIBR, VOL_1200, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VIBR,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VIBR, VOL_1300, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VIBR,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VIBR, VOL_1500, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VIBR,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VIBR, VOL_1800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VIBR,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VIBR, VOL_2500, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VIBR,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VIBR, VOL_2800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VIBR,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VIBR, VOL_3000, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VIBR,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VIBR, VOL_3300, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VIBR,      "ldo_test");
        read_auxadc_value();
        break;

      case 14:  
        hwPowerOn(MT6323_POWER_LDO_VM, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VM,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VM, VOL_1200, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VM,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VM, VOL_1350, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VM,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VM, VOL_1500, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VM,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VM, VOL_1800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VM,      "ldo_test");
        read_auxadc_value();        
        break;

      case 15:
        hwPowerOn(MT6323_POWER_LDO_VRF18,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VRF18,      "ldo_test");
        read_auxadc_value();
        break;
		
      case 16:
        hwPowerOn(MT6323_POWER_LDO_VIO18,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VIO18,      "ldo_test");
        read_auxadc_value();
        break;

      case 17:
        hwPowerOn(MT6323_POWER_LDO_VCAMD, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAMD,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAMD, VOL_1200, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAMD,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAMD, VOL_1350, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAMD,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAMD, VOL_1500, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAMD,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAMD, VOL_1800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAMD,      "ldo_test");
        read_auxadc_value();
        break;

      case 18:
        hwPowerOn(MT6323_POWER_LDO_VCAM_IO,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAM_IO,      "ldo_test");
        read_auxadc_value();
        break;

      case 19:
        hwPowerOn(MT6323_POWER_LDO_VTCXO,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VTCXO,      "ldo_test");
        read_auxadc_value();
        break;		

      case 20:  
        hwPowerOn(MT6323_POWER_LDO_VA, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VA,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VA, VOL_1800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VA,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VA, VOL_2500, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VA,      "ldo_test");
        read_auxadc_value();
        break;

      case 21:  
        hwPowerOn(MT6323_POWER_LDO_VCAMA, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAMA,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAMA, VOL_1500, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAMA,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAMA, VOL_1800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAMA,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAMA, VOL_2500, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAMA,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCAMA, VOL_2800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCAMA,      "ldo_test");
        read_auxadc_value();		
        break;

      case 22:  
        hwPowerOn(MT6323_POWER_LDO_VCN33_BT,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCN33_BT,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCN33_BT,     VOL_3300, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCN33_BT,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCN33_BT,     VOL_3400, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCN33_BT,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCN33_BT,     VOL_3500, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCN33_BT,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCN33_BT,     VOL_3600, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCN33_BT,      "ldo_test");
        read_auxadc_value();
        break;

      case 23:  
        hwPowerOn(MT6323_POWER_LDO_VCN28,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCN28,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCN28,    VOL_1800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCN28,      "ldo_test");
        read_auxadc_value();
        hwPowerOn(MT6323_POWER_LDO_VCN28,    VOL_2800, "ldo_test");
        read_auxadc_value();
        hwPowerDown(MT6323_POWER_LDO_VCN28,      "ldo_test");
        read_auxadc_value();
        break;

	  default:
        printk("[pmic_UVVP_PMIC_LDO_VOSEL] Invalid channel value(%d)\n", index_val);
        break;
        
    }
    
    printk("[pmic_UVVP_PMIC_LDO_VOSEL] end....\n");
}
///////////////////////////////////////////////////////////////////////////////////
void pmic_UVVP_PMIC_LDO_CAL(int index_val)
{
    int i=0;
    
    printk("[pmic_UVVP_PMIC_LDO_VOSEL] start....\n");

    switch(index_val){
      case 0:
        hwPowerOn(MT6323_POWER_LDO_VIO28,    VOL_DEFAULT, "ldo_test");    
        read_auxadc_value();
        
        for(i=0;i<=PMIC_RG_VIO28_CAL_MASK;i++)
        {
            upmu_set_rg_vio28_cal(i);
            printk("[MT65XX_POWER_LDO_VIO28] cal=%d, ",i);
            read_auxadc_value();
        }

        hwPowerDown(MT6323_POWER_LDO_VIO28,      "ldo_test");
        read_auxadc_value();
        break;

      case 1:
        hwPowerOn(MT6323_POWER_LDO_VUSB,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        
        for(i=0;i<=PMIC_RG_VUSB_CAL_MASK;i++)
        {
            upmu_set_rg_vusb_cal(i);
            printk("[MT65XX_POWER_LDO_VUSB] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VUSB,      "ldo_test");
        read_auxadc_value();        
        break;

      case 2:
        hwPowerOn(MT6323_POWER_LDO_VMC,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VMC_CAL_MASK;i++)
        {
            upmu_set_rg_vmc_cal(i);
            printk("[MT65XX_POWER_LDO_VMC] cal=%d, ",i);
            read_auxadc_value();
        }    
        
        hwPowerDown(MT6323_POWER_LDO_VMC,      "ldo_test");
        read_auxadc_value();        
        break;

      case 3:
        hwPowerOn(MT6323_POWER_LDO_VMCH,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VMCH_CAL_MASK;i++)
        {
            upmu_set_rg_vmch_cal(i);
            printk("[MT65XX_POWER_LDO_VMCH] cal=%d, ",i);
            read_auxadc_value();
        }    
        
        hwPowerDown(MT6323_POWER_LDO_VMCH,     "ldo_test");
        read_auxadc_value();
        break;

      case 4:
        hwPowerOn(MT6323_POWER_LDO_VEMC_3V3, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VEMC_3V3_CAL_MASK;i++)
        {
            upmu_set_rg_vemc_3v3_cal(i);
            printk("[MT65XX_POWER_LDO_VEMC_3V3] cal=%d, ",i);
            read_auxadc_value();
        }    
        
        hwPowerDown(MT6323_POWER_LDO_VEMC_3V3,  "ldo_test");
        read_auxadc_value();       
        break;

      case 5:
        hwPowerOn(MT6323_POWER_LDO_VGP1,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VGP1_CAL_MASK;i++)
        {
            upmu_set_rg_vgp1_cal(i);
            printk("[MT65XX_POWER_LDO_VGP1] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VGP1,      "ldo_test");
        read_auxadc_value();
        break;

      case 6:  
        hwPowerOn(MT6323_POWER_LDO_VGP2,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VGP2_CAL_MASK;i++)
        {
            upmu_set_rg_vgp2_cal(i);
            printk("[MT65XX_POWER_LDO_VGP2] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VGP2,      "ldo_test");
        read_auxadc_value();
        break;

      case 7:  
        hwPowerOn(MT6323_POWER_LDO_VGP3,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VGP3_CAL_MASK;i++)
        {
            upmu_set_rg_vgp3_cal(i);
            printk("[MT65XX_POWER_LDO_VGP3] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VGP3,      "ldo_test");
        read_auxadc_value();       
        break;

      case 8:
        hwPowerOn(MT6323_POWER_LDO_VCN_1V8, VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VCN_1V8_CAL_MASK;i++)
        {
            upmu_set_rg_vcn_1v8_cal(i);
            printk("[MT65XX_POWER_LDO_VCN_1V8] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VCN_1V8,  "ldo_test");
        read_auxadc_value();
        break;


      case 9:  
        hwPowerOn(MT6323_POWER_LDO_VSIM1,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VSIM1_CAL_MASK;i++)
        {
            upmu_set_rg_vsim1_cal(i);
            printk("[MT65XX_POWER_LDO_VSIM1] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VSIM1,      "ldo_test");
        read_auxadc_value();
        break;

      case 10:  
        hwPowerOn(MT6323_POWER_LDO_VSIM2,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VSIM2_CAL_MASK;i++)
        {
            upmu_set_rg_vsim2_cal(i);
            printk("[MT65XX_POWER_LDO_VSIM2] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VSIM2,      "ldo_test");
        read_auxadc_value();
        break;

      case 11:  
        hwPowerOn(MT6323_POWER_LDO_VRTC,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
/*
        for(i=0;i<=PMIC_RG_VRTC_CAL_MASK;i++)
        {
            upmu_set_rg_vrtc_cal(i);
            printk("[MT65XX_POWER_LDO_VGP6] cal=%d, ",i);
            read_auxadc_value();
        }
*/        
        hwPowerDown(MT6323_POWER_LDO_VRTC,      "ldo_test");
        read_auxadc_value();
        break;

      case 12:  
        hwPowerOn(MT6323_POWER_LDO_VCAM_AF,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VCAM_AF_CAL_MASK;i++)
        {
            upmu_set_rg_vcam_af_cal(i);
            printk("[MT65XX_POWER_LDO_VCAM_AF] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VCAM_AF,     "ldo_test");
        read_auxadc_value();
        break;

      case 13:
        hwPowerOn(MT6323_POWER_LDO_VIBR,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VIBR_CAL_MASK;i++)
        {
            upmu_set_rg_vibr_cal(i);
            printk("[MT65XX_POWER_LDO_VIBR] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VIBR,     "ldo_test");
        read_auxadc_value();
        break;

      case 14:  
        hwPowerOn(MT6323_POWER_LDO_VM,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VM_CAL_MASK;i++)
        {
            upmu_set_rg_vm_cal(i);
            printk("[MT65XX_POWER_LDO_VM] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VM,      "ldo_test");
        read_auxadc_value();
        break;

      case 15:  
        hwPowerOn(MT6323_POWER_LDO_VRF18,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();
        
        for(i=0;i<=PMIC_RG_VRF18_CAL_MASK;i++)
        {
            upmu_set_rg_vrf18_cal(i);
            printk("[MT65XX_POWER_LDO_VRF18] cal=%d, ",i);
            read_auxadc_value();
        }
               
        hwPowerDown(MT6323_POWER_LDO_VRF18,      "ldo_test");
        read_auxadc_value();
        break;

      case 16:  
        hwPowerOn(MT6323_POWER_LDO_VIO18,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VIO18_CAL_MASK;i++)
        {
            upmu_set_rg_vio18_cal(i);
            printk("[MT65XX_POWER_LDO_VIO18] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VIO18,      "ldo_test");
        read_auxadc_value();
        break;

      case 17:  
        hwPowerOn(MT6323_POWER_LDO_VCAMD,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VCAMD_CAL_MASK;i++)
        {
            upmu_set_rg_vcamd_cal(i);
            printk("[MT65XX_POWER_LDO_VCAMD] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VCAMD,     "ldo_test");
        read_auxadc_value();       
        break;

      case 18:  
        hwPowerOn(MT6323_POWER_LDO_VCAM_IO,  VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VCAM_IO_CAL_MASK;i++)
        {
            upmu_set_rg_vcam_io_cal(i);
            printk("[MT65XX_POWER_LDO_VCAM_IO] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VCAM_IO,   "ldo_test");
        read_auxadc_value();
        break;

      case 19:  
        hwPowerOn(MT6323_POWER_LDO_VTCXO,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VTCXO_CAL_MASK;i++)
        {
            upmu_set_rg_vtcxo_cal(i);
            printk("[MT65XX_POWER_LDO_VTCXO] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VTCXO,     "ldo_test");
        read_auxadc_value();
        break;

      case 20:  
        hwPowerOn(MT6323_POWER_LDO_VA,  VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VA_CAL_MASK;i++)
        {
            upmu_set_rg_va_cal(i);
            printk("[MT65XX_POWER_LDO_VA] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VA,   "ldo_test");
        read_auxadc_value();       
        break;

      case 21:  
        hwPowerOn(MT6323_POWER_LDO_VCAMA,       VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VCAMA_CAL_MASK;i++)
        {
            upmu_set_rg_vcama_cal(i);
            printk("[MT65XX_POWER_LDO_VCAMA] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VCAMA,        "ldo_test");
        read_auxadc_value();
        break;

      case 22:  
        hwPowerOn(MT6323_POWER_LDO_VCN33_BT,     VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VCN33_CAL_MASK;i++)
        {
            upmu_set_rg_vcn33_cal(i);
            printk("[MT65XX_POWER_LDO_VCN33] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VCN33_BT,      "ldo_test");
        read_auxadc_value();
        break;

      case 23:  
        hwPowerOn(MT6323_POWER_LDO_VCN28,    VOL_DEFAULT, "ldo_test");
        read_auxadc_value();

        for(i=0;i<=PMIC_RG_VCN28_CAL_MASK;i++)
        {
            upmu_set_rg_vcn28_cal(i);
            printk("[MT65XX_POWER_LDO_VCN28] cal=%d, ",i);
            read_auxadc_value();
        }
        
        hwPowerDown(MT6323_POWER_LDO_VCN28,     "ldo_test");
        read_auxadc_value();
        break;
        
	  default:
        printk("[pmic_UVVP_PMIC_LDO_VOSEL] Invalid channel value(%d)\n", index_val);
        break;
        
    }
    
    printk("[pmic_UVVP_PMIC_LDO_VOSEL] end....\n");
}



//---------------------------------------------------------------------------
// IOCTL
//---------------------------------------------------------------------------
long uvvp_pmic_ioctl(struct file *file,unsigned int cmd, unsigned long arg)
{
	printk("\r\n******** uvvp_pmic_ioctl cmd[%d]********\r\n",cmd);
	
	switch (cmd) {
		default:
			return -1;

	//  General TEST CASE
	case UVVP_PMIC_GET_VERSION:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_GET_VERSION ********\r\n");	
		pmic_get_chip_version_ldvt();
		return 0;					
	case UVVP_PMIC_GET_PCHR_CHRDET:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_GET_PCHR_CHRDET ********\r\n");
		pmic_get_PCHR_CHRDET_ldvt();			
		return 0;							

	// Top TEST CASE        
	case UVVP_PMIC_VERIFY_DEFAULT_VALUE:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_VERIFY_DEFAULT_VALUE ********\r\n");
		pmic_VERIFY_DEFAULT_VALUE_ldvt();			
		return 0;
	case UVVP_PMIC_TOP_WR_1:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_TOP_WR_1 ********\r\n");
		pmic_UVVP_PMIC_TOP_WR(0x5a5a);			
		return 0;
	case UVVP_PMIC_TOP_WR_2:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_TOP_WR_2 ********\r\n");
		pmic_UVVP_PMIC_TOP_WR(0xa5a5);			
		return 0;			

    //  AUXADC TEST CASE					
	case UVVP_PMIC_AUXADC_111:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_AUXADC_111 ********\r\n");
		pmic_UVVP_PMIC_AUXADC_111();			
		return 0;
	case UVVP_PMIC_AUXADC_131:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_AUXADC_131 ********\r\n");
		pmic_UVVP_PMIC_AUXADC_131();			
		return 0;
	case UVVP_PMIC_AUXADC_132:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_AUXADC_132 ********\r\n");
		pmic_UVVP_PMIC_AUXADC_132();			
		return 0;
	case UVVP_PMIC_AUXADC_133:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_AUXADC_133 ********\r\n");
		pmic_UVVP_PMIC_AUXADC_133();			
		return 0;   
	case UVVP_PMIC_AUXADC_134:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_AUXADC_134 ********\r\n");
		pmic_UVVP_PMIC_AUXADC_134();			
		return 0;
	case UVVP_PMIC_AUXADC_135:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_AUXADC_135 ********\r\n");
		pmic_UVVP_PMIC_AUXADC_135();			
		return 0;
	case UVVP_PMIC_AUXADC_141:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_AUXADC_141 ********\r\n");
		pmic_UVVP_PMIC_AUXADC_141();			
		return 0;
	case UVVP_PMIC_AUXADC_211:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_AUXADC_211 ********\r\n");
		pmic_UVVP_PMIC_AUXADC_211();			
		return 0; 
	case UVVP_PMIC_AUXADC_212:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_AUXADC_212 ********\r\n");
		pmic_UVVP_PMIC_AUXADC_212();			
		return 0; 
	case UVVP_PMIC_AUXADC_213:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_AUXADC_213 ********\r\n");
		pmic_UVVP_PMIC_AUXADC_213();			
		return 0; 
	case UVVP_PMIC_AUXADC_214:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_AUXADC_214 ********\r\n");
		pmic_UVVP_PMIC_AUXADC_214();			
		return 0; 
	case UVVP_PMIC_AUXADC_215:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_AUXADC_215 ********\r\n");
		pmic_UVVP_PMIC_AUXADC_215();			
		return 0; 
	case UVVP_PMIC_AUXADC_216:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_AUXADC_216 ********\r\n");
		pmic_UVVP_PMIC_AUXADC_216();			
		return 0; 		
	case UVVP_PMIC_AUXADC_321:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_AUXADC_321 ********\r\n");
		pmic_UVVP_PMIC_AUXADC_321();			
		return 0; 	
		

	//  INTERRUPT TEST CASE
	case UVVP_PMIC_INT_451:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_INT_451 ********\r\n");
		pmic_UVVP_PMIC_INT_451();			
		return 0;
	case UVVP_PMIC_INT_461:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_INT_461 ********\r\n");
		pmic_UVVP_PMIC_INT_461();           
		return 0;
	case UVVP_PMIC_INT_462:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_INT_462 ********\r\n");
		pmic_UVVP_PMIC_INT_462();           
		return 0;
	case UVVP_PMIC_INT_463:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_INT_463 ********\r\n");
		pmic_UVVP_PMIC_INT_463();           
		return 0;
    case UVVP_PMIC_INT_464:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_INT_464 ********\r\n");
		return 0;
	case UVVP_PMIC_INT_465:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_INT_465 ********\r\n");
		return 0;
	case UVVP_PMIC_INT_466:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_INT_466 ********\r\n");
		return 0;
	case UVVP_PMIC_INT_467:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_INT_467 ********\r\n");
		return 0;
    case UVVP_PMIC_INT_468:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_INT_468 ********\r\n");		
		return 0;
			
	//  LDO TEST CASE			
	case UVVP_PMIC_LDO_ON_OFF_0:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_0 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(0);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_1:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_1 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(1);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_2:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_2 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(2);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_3:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_3 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(3);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_4:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_4 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(4);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_5:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_5 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(5);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_6:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_6 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(6);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_7:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_7 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(7);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_8:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_8 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(8);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_9:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_9 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(9);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_10:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_10 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(10);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_11:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_11 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(11);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_12:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_12 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(12);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_13:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_13 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(13);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_14:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_14 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(14);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_15:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_15 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(15);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_16:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_16 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(16);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_17:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_17 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(17);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_18:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_18 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(18);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_19:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_19 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(19);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_20:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_20 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(20);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_21:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_21 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(21);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_22:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_22 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(22);			
		return 0;
	case UVVP_PMIC_LDO_ON_OFF_23:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_ON_OFF_23 ********\r\n");
		pmic_UVVP_PMIC_LDO_ON_OFF(23);			
		return 0;
	//----------------------------------------------------------------------------    
	case UVVP_PMIC_LDO_VOSEL_0:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_0 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(0);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_1:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_1 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(1);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_2:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_2 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(2);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_3:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_3 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(3);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_4:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_4 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(4);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_5:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_5 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(5);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_6:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_6 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(6);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_7:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_7 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(7);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_8:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_8 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(8);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_9:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_9 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(9);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_10:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_10 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(10);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_11:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_11 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(11);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_12:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_12 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(12);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_13:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_13 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(13);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_14:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_14 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(14);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_15:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_15 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(15);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_16:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_16 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(16);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_17:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_17 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(17);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_18:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_18 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(18);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_19:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_19 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(19);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_20:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_20 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(20);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_21:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_21 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(21);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_22:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_22 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(22);			
		return 0;
	case UVVP_PMIC_LDO_VOSEL_23:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_VOSEL_23 ********\r\n");
		pmic_UVVP_PMIC_LDO_VOSEL(23);			
		return 0;		
	//---------------------------------------------------------------------------- 
	case UVVP_PMIC_LDO_CAL_0:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_0 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(0);			
		return 0;
	case UVVP_PMIC_LDO_CAL_1:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_1 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(1);			
		return 0;
	case UVVP_PMIC_LDO_CAL_2:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_2 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(2);			
		return 0;
	case UVVP_PMIC_LDO_CAL_3:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_3 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(3);			
		return 0;
	case UVVP_PMIC_LDO_CAL_4:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_4 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(4);			
		return 0;
	case UVVP_PMIC_LDO_CAL_5:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_5 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(5);			
		return 0;
	case UVVP_PMIC_LDO_CAL_6:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_6 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(6);			
		return 0;
	case UVVP_PMIC_LDO_CAL_7:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_7 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(7);			
		return 0;
	case UVVP_PMIC_LDO_CAL_8:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_8 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(8);			
		return 0;
	case UVVP_PMIC_LDO_CAL_9:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_9 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(9);			
		return 0;
	case UVVP_PMIC_LDO_CAL_10:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_10 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(10);			
		return 0;
	case UVVP_PMIC_LDO_CAL_11:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_11 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(11);			
		return 0;
	case UVVP_PMIC_LDO_CAL_12:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_12 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(12);			
		return 0;
	case UVVP_PMIC_LDO_CAL_13:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_13 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(13);			
		return 0;
	case UVVP_PMIC_LDO_CAL_14:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_14 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(14);			
		return 0;
	case UVVP_PMIC_LDO_CAL_15:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_15 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(15);			
		return 0;
	case UVVP_PMIC_LDO_CAL_16:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_16 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(16);			
		return 0;
	case UVVP_PMIC_LDO_CAL_17:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_17 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(17);			
		return 0;
	case UVVP_PMIC_LDO_CAL_18:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_18 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(18);			
		return 0;
	case UVVP_PMIC_LDO_CAL_19:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_19 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(19);			
		return 0;
	case UVVP_PMIC_LDO_CAL_20:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_20 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(20);			
		return 0;
	case UVVP_PMIC_LDO_CAL_21:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_21 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(21);			
		return 0;
	case UVVP_PMIC_LDO_CAL_22:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_22 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(22);			
		return 0;
	case UVVP_PMIC_LDO_CAL_23:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_LDO_CAL_23 ********\r\n");
		pmic_UVVP_PMIC_LDO_CAL(23);			
		return 0;

		//  BUCK TEST CASE		
	case UVVP_PMIC_BUCK_ON_OFF_0:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_BUCK_ON_OFF_0 ********\r\n");
		pmic_UVVP_PMIC_BUCK_ON_OFF(0);			
		return 0;
	case UVVP_PMIC_BUCK_ON_OFF_1:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_BUCK_ON_OFF_1 ********\r\n");
		pmic_UVVP_PMIC_BUCK_ON_OFF(1);			
		return 0;
	case UVVP_PMIC_BUCK_ON_OFF_2:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_BUCK_ON_OFF_2 ********\r\n");
		pmic_UVVP_PMIC_BUCK_ON_OFF(2);			
		return 0;
	//----------------------------------------------------------------------------
	case UVVP_PMIC_BUCK_VOSEL_0:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_BUCK_VOSEL_0 ********\r\n");
		pmic_UVVP_PMIC_BUCK_VOSEL(0);			
		return 0;
	case UVVP_PMIC_BUCK_VOSEL_1:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_BUCK_VOSEL_1 ********\r\n");
		pmic_UVVP_PMIC_BUCK_VOSEL(1);			
		return 0;
	case UVVP_PMIC_BUCK_VOSEL_2:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_BUCK_VOSEL_2 ********\r\n");
		pmic_UVVP_PMIC_BUCK_VOSEL(2);			
		return 0;

	//----------------------------------------------------------------------------
	case UVVP_PMIC_BUCK_DLC_0:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_BUCK_DLC_0 ********\r\n");
		pmic_UVVP_PMIC_BUCK_DLC(0);			
		return 0;
	case UVVP_PMIC_BUCK_DLC_1:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_BUCK_DLC_1 ********\r\n");
		pmic_UVVP_PMIC_BUCK_DLC(1);			
		return 0;
	case UVVP_PMIC_BUCK_DLC_2:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_BUCK_DLC_2 ********\r\n");
		pmic_UVVP_PMIC_BUCK_DLC(2);			
		return 0;
	//----------------------------------------------------------------------------
	case UVVP_PMIC_BUCK_BURST_0:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_BUCK_BURST_0 ********\r\n");
		pmic_UVVP_PMIC_BUCK_BURST(0);			
		return 0;
	case UVVP_PMIC_BUCK_BURST_1:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_BUCK_BURST_1 ********\r\n");
		pmic_UVVP_PMIC_BUCK_BURST(1);			
		return 0;
	case UVVP_PMIC_BUCK_BURST_2:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_BUCK_BURST_2 ********\r\n");
		pmic_UVVP_PMIC_BUCK_BURST(2);			
		return 0;
		
	case UVVP_PMIC_DUMP_MEMORY:
		printk("\r\n******** uvvp_pmic_ioctl UVVP_PMIC_BUCK_BURST_2 ********\r\n");
		do_dump_pmic();			
		return 0;

        //  DRIVER TEST CASE (coverd by SA)
        //  P-chr TEST CASE (coverd by SA)
		//  FGADC TEST CASE (coverd by UVVP_PMIC_VERIFY_DEFAULT_VALUE and SA)	
            
	}

	return 0;	
}

static int uvvp_pmic_open(struct inode *inode, struct file *file)
{
	return 0;
}

static struct file_operations uvvp_pmic_fops = {
	.owner           = THIS_MODULE,
		
	.open            = uvvp_pmic_open,
	.unlocked_ioctl  = uvvp_pmic_ioctl,
	.compat_ioctl    = uvvp_pmic_ioctl,
};

static struct miscdevice uvvp_pmic_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = pmicname,
	.fops = &uvvp_pmic_fops,
};

static int __init uvvp_pmic_init(void)
{
    int ret;
	printk("\r\n******** uvvp_pmic_init  ********\r\n");
	ret = misc_register(&uvvp_pmic_dev);
    if(ret){
		printk("register driver failed\n");
	}
	return 0;
}

static void __exit uvvp_pmic_exit(void)
{

}

module_init(uvvp_pmic_init);
module_exit(uvvp_pmic_exit);


