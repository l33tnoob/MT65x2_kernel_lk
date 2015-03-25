#ifndef __MT_EMI_BM_H__
#define __MT_EMI_BW_H__

#define EMI_ARBA    (EMI_BASE + 0x100)
#define EMI_ARBB    (EMI_BASE + 0x108)
#define EMI_ARBC    (EMI_BASE + 0x110)
#define EMI_ARBD    (EMI_BASE + 0x118)
#define EMI_ARBE    (EMI_BASE + 0x120)
#define EMI_ARBF    (EMI_BASE + 0x128)

#define EMI_BMEN    (EMI_BASE + 0x400)
#define EMI_BCNT    (EMI_BASE + 0x408)
#define EMI_TACT    (EMI_BASE + 0x410)
#define EMI_TSCT    (EMI_BASE + 0x418)
#define EMI_WACT    (EMI_BASE + 0x420)
#define EMI_WSCT    (EMI_BASE + 0x428)
#define EMI_BACT    (EMI_BASE + 0x430)
#define EMI_BSCT    (EMI_BASE + 0x438)
#define EMI_MSEL    (EMI_BASE + 0x440)
#define EMI_TSCT2   (EMI_BASE + 0x448)
#define EMI_TSCT3   (EMI_BASE + 0x450)
#define EMI_WSCT2   (EMI_BASE + 0x458)
#define EMI_WSCT3   (EMI_BASE + 0x460)
#define EMI_MSEL2   (EMI_BASE + 0x468)
#define EMI_MSEL3   (EMI_BASE + 0x470)
#define EMI_MSEL4   (EMI_BASE + 0x478)
#define EMI_MSEL5   (EMI_BASE + 0x480)
#define EMI_MSEL6   (EMI_BASE + 0x488)
#define EMI_MSEL7   (EMI_BASE + 0x490)
#define EMI_MSEL8   (EMI_BASE + 0x498)
#define EMI_MSEL9   (EMI_BASE + 0x4A0)
#define EMI_MSEL10  (EMI_BASE + 0x4A8)
#define EMI_BMID0   (EMI_BASE + 0x4B0)
#define EMI_BMID1   (EMI_BASE + 0x4B8)
#define EMI_BMID2   (EMI_BASE + 0x4C0)
#define EMI_BMID3   (EMI_BASE + 0x4C8)
#define EMI_BMID4   (EMI_BASE + 0x4D0)
#define EMI_BMID5   (EMI_BASE + 0x4D8)

#define EMI_TTYPE1               (EMI_BASE+0x0500)
#define EMI_TTYPE2               (EMI_BASE+0x0508)
#define EMI_TTYPE3               (EMI_BASE+0x0510)
#define EMI_TTYPE4               (EMI_BASE+0x0518)
#define EMI_TTYPE5               (EMI_BASE+0x0520)
#define EMI_TTYPE6               (EMI_BASE+0x0528)
#define EMI_TTYPE7               (EMI_BASE+0x0530)
#define EMI_TTYPE8               (EMI_BASE+0x0538)
#define EMI_TTYPE9               (EMI_BASE+0x0540)
#define EMI_TTYPE10             (EMI_BASE+0x0548)
#define EMI_TTYPE11             (EMI_BASE+0x0550)
#define EMI_TTYPE12             (EMI_BASE+0x0558)
#define EMI_TTYPE13             (EMI_BASE+0x0560)
#define EMI_TTYPE14             (EMI_BASE+0x0568)
#define EMI_TTYPE15             (EMI_BASE+0x0570)
#define EMI_TTYPE16             (EMI_BASE+0x0578)
#define EMI_TTYPE17             (EMI_BASE+0x0580)
#define EMI_TTYPE18             (EMI_BASE+0x0588)
#define EMI_TTYPE19             (EMI_BASE+0x0590)
#define EMI_TTYPE20             (EMI_BASE+0x0598)
#define EMI_TTYPE21             (EMI_BASE+0x05A0)
#define EMI_BSCT2                 (EMI_BASE+0x05A8) 
#define EMI_BSCT3                 (EMI_BASE+0x05B0) 

#define DRAMC_PAGE_HIT      (EMI_BASE + 0x420)
#define DRAMC_PAGE_MISS     (EMI_BASE + 0x430)
#define DRAMC_INTERBANK     (EMI_BASE + 0x438)
//#define DRAMC_IDLE_COUNT        (EMI_BASE + 0x12B0)


typedef enum
{
    DRAMC_R2R,
    DRAMC_R2W,
    DRAMC_W2R,
    DRAMC_W2W,
    DRAMC_ALL
} DRAMC_Cnt_Type;

typedef enum
{
    BM_BOTH_READ_WRITE,
    BM_READ_ONLY,
    BM_WRITE_ONLY
} BM_RW_Type;

enum 
{
    BM_TRANS_TYPE_1BEAT = 0x0,
    BM_TRANS_TYPE_2BEAT,                        
    BM_TRANS_TYPE_3BEAT,
    BM_TRANS_TYPE_4BEAT,
    BM_TRANS_TYPE_5BEAT,                    
    BM_TRANS_TYPE_6BEAT,                        
    BM_TRANS_TYPE_7BEAT,
    BM_TRANS_TYPE_8BEAT,
    BM_TRANS_TYPE_9BEAT,                        
    BM_TRANS_TYPE_10BEAT,                    
    BM_TRANS_TYPE_11BEAT,
    BM_TRANS_TYPE_12BEAT,
    BM_TRANS_TYPE_13BEAT,                    
    BM_TRANS_TYPE_14BEAT,                    
    BM_TRANS_TYPE_15BEAT,
    BM_TRANS_TYPE_16BEAT,
    BM_TRANS_TYPE_1Byte = 0 << 4,
    BM_TRANS_TYPE_2Byte = 1 << 4,
    BM_TRANS_TYPE_4Byte = 2 << 4,
    BM_TRANS_TYPE_8Byte = 3 << 4,
   // BM_TRANS_TYPE_16Byte = 4 << 4,              // CM [20121102] different from MT6575/77
    BM_TRANS_TYPE_BURST_WRAP = 0 << 6,
    BM_TRANS_TYPE_BURST_INCR = 1 << 6
};

#define BM_MASTER_AP_MCU        (0x01)
#define BM_MASTER_CONN_SYS    (0x02)
#define BM_MASTER_MMSYS          (0x04)
#define BM_MASTER_MD_MCU        (0x08)
#define BM_MASTER_MD_HW        (0x10)
#define BM_MASTER_PERI          (0x20)
#define BM_MASTER_ALL           (0x3F)

#define BUS_MON_EN      (0x00000401)
#define BUS_MON_PAUSE   (0x00000002)
#define BC_OVERRUN      (0x00000100)

#define BM_COUNTER_MAX  (21)

#define BM_REQ_OK           (0)
#define BM_ERR_WRONG_REQ    (-1)
#define BM_ERR_OVERRUN      (-2)

extern void BM_Init(void);
extern void BM_DeInit(void);
extern void BM_Enable(const unsigned int enable);
//extern void BM_Disable(void);
extern void BM_Pause(void);
extern void BM_Continue(void);
extern unsigned int BM_IsOverrun(void);
extern void BM_SetReadWriteType(const unsigned int ReadWriteType);
extern int BM_GetBusCycCount(void);
extern unsigned int BM_GetTransAllCount(void);
extern int BM_GetTransCount(const unsigned int counter_num);
extern int BM_GetWordAllCount(void);
extern int BM_GetWordCount(const unsigned int counter_num);
extern unsigned int BM_GetBandwidthWordCount(void);
extern unsigned int BM_GetOverheadWordCount(void);
extern unsigned int BM_GetBusBusyAllCount(void);    
extern unsigned int BM_GetBusBusyCount(const unsigned int counter_num);
extern unsigned int BM_GetEMIClockCount(void);    
extern unsigned int BM_get_emi_freq(void);
extern int BM_GetTransTypeCount(const unsigned int counter_num);
extern int BM_SetMonitorCounter(const unsigned int counter_num, const unsigned int master, const unsigned int trans_type);
extern int BM_SetMaster(const unsigned int counter_num, const unsigned int master);
extern int BM_SetIDSelect(const unsigned int counter_num, const unsigned int id, const unsigned int enable);
extern int BM_SetUltraHighFilter(const unsigned int counter_num, const unsigned int enable);
extern int BM_SetLatencyCounter(void);
extern int BM_GetLatencyCycle(const unsigned int counter_num);
extern void MCI_Mon_Enable(void);
extern void MCI_Mon_Disable(void);
extern void MCI_Event_Set(unsigned int evt0, unsigned int evt1);
extern void MCI_Event_Read(void);
extern unsigned int MCI_GetEventCount(int evt_counter);

extern unsigned int DRAMC_GetPageHitCount(DRAMC_Cnt_Type CountType);
extern unsigned int DRAMC_GetPageMissCount(DRAMC_Cnt_Type CountType);
extern unsigned int DRAMC_GetInterbankCount(DRAMC_Cnt_Type CountType);
extern unsigned int DRAMC_GetIdleCount(void);

#endif  /* !__MT_EMI_BW_H__ */
