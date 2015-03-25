#ifndef CACHE_H
#define CACHE_H

#define PL310_BASE 0xc000E000
#define DYNAMIC_CLOCK_GATING_ENABLE     (1 << 1)
#define CACHE_LINE_SIZE                 32
#define CACHE_LINE_MASK                 (CACHE_LINE_SIZE - 1)


#define CACHE_ALLIGN(addr)  \
		addr &= ~ CACHE_LINE_MASK

#define RW_REG    volatile unsigned
#define RO_REG    const volatile unsigned 
 
struct pl310_regs                                                               
{                                                                               
  RO_REG CacheID;                                                  
  RO_REG CacheType;                                                 
  RW_REG Reserved0[(0x100-0x08)/4];  // 0x008-0x0FC                  
  RW_REG Control;                                                       
  RW_REG AuxControl;                                                    
  RW_REG TagRAMLatencyControl;                                          
  RW_REG DataRAMLatencyControl;                                         
  RW_REG Reserved1[(0x200-0x110)/4]; // 0x110-0x1FC                  
  RW_REG EventCntControl;                                                 
  RW_REG EvtCnt1Cfg;                                                
  RW_REG EvtCnt0Cfg;                                                
  RW_REG EvtCnt1Value;                                                 
  RW_REG EvtCnt0Value;                                                 
  RW_REG InterrupMask;                                                   
  RO_REG MaskInterruptStatus;                                       
  RO_REG RawInterrupStatus;                                        
  RW_REG InterruptClear;                                                    
  RW_REG Reserved2[(0x730-0x224)/4]; // 0x224-0x72c                  
  RW_REG CacheSync;                                                  
  RW_REG Reserved71[(0x770-0x734)/4];// 0x734-0x76C                  
  RW_REG InvalidateLineByPA;                                              
  RW_REG Reserved72[(0x77C-0x774)/4];// 0x774-0x778                  
  RW_REG InvalidateByWay;                                                 
  RW_REG Reserved73[(0x7B0-0x780)/4];// 0x780-0x7AC                  
  RW_REG CleanLineByPA;                                              
  RW_REG Reserved74;                                                 
  RW_REG CleanLineBySetWay;                                        
  RW_REG CleanByWay;                                                 
  RW_REG Reserved75[(0x7F0-0x7C0)/4];// 0x7C0-0x7EC                  
  RW_REG FlushLineByPA;                                             
  RW_REG Reserved76;                                                 
  RW_REG FlushLineBySetWay;                                       
  RW_REG FlushByWay;                                            
  RW_REG Reserved77[(0x900-0x800)/4];// 0x800-0x8FC                  
  RW_REG DataLockdown0ByWay;                                         
  RW_REG InstructionLockdown0ByWay;                                        
  RW_REG DataLockdown1ByWay;                                         
  RW_REG InstructionLockdown1ByWay;                                        
  RW_REG DataLockdown2ByWay;                                         
  RW_REG InstructionLockdown2ByWay;                                        
  RW_REG DataLockdown3ByWay;                                         
  RW_REG InstructionLockdown3ByWay;                                        
  RW_REG DataLockdown4ByWay;                                         
  RW_REG InstructionLockdown4ByWay;                                        
  RW_REG DataLockdown5ByWay;                                         
  RW_REG InstructionLockdown5ByWay;                                        
  RW_REG DataLockdown6ByWay;                                         
  RW_REG InstructionLockdown6ByWay;                                        
  RW_REG DataLockdown7ByWay;                                         
  RW_REG InstructionLockdown7ByWay;                                        
  RW_REG Reserved90[(0x950-0x940)/4];// 0x940-0x94C                  
  RW_REG LockdownByLineEn;                                       
  RW_REG UnlockAllLinesByWay;                                        
  RW_REG Reserved91[(0xC00-0x958)/4];// 0x958-0x9FC                  
  RW_REG AddressFilteringStart;                                      
  RW_REG AddressFilteringEnd;                                        
  RW_REG Reserved12[(0xF40-0xC08)/4];// 0xC08-0xF3C                  
  RW_REG DebugControl;                                                  
  RW_REG Reserved150[(0xF60-0xF44)/4];// 0xF44-0xF5C                 
  RW_REG PrefetchControl;                                               
  RW_REG Reserved151[(0xF80-0xF64)/4];// 0xF64-0xF7C                 
  RW_REG PowerControl;                                                  
};                                                                              
 
#endif