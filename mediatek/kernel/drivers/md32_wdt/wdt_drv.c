#include <mach/md32_wdt.h>
#include <linux/workqueue.h>
#include <linux/aee.h>
#include <mach/mt_typedefs.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/slab.h>

#define MD32_IRQ_ID 0x0
#define MD32_TEST 1
#define MD32_AP_DEBUG  1
#if(MD32_AP_DEBUG == 1)
#define dbg_msg printk
#else
#define dbg_msg(...)
#endif

static md32_wdt_func WDT_FUN;
static md32_assert_func  ASSERT_FUN;
static struct work_struct reset_md32_wdt;
static struct work_struct reset_md32_assert;
static struct workqueue_struct *WDT_wq;
static struct workqueue_struct *ASSERT_wq;
/*simulator*/
static char IPC_data[]= "IPC Data = 1";

int reset_md32_by_wdt_func()
{
  int index;
#define TEST_PHY_SIZE 0x10000
  int log[16], i;
  char *ptr;
  printk("reset_md32_func\n");
 //1. AEE function
  /*
  memset(log, 0, sizeof(log));
  ptr = kmalloc(TEST_PHY_SIZE, GFP_KERNEL);
  if (ptr == NULL) {
      printk( "proc_read_generate_ee kmalloc fail\n");
      return -1;
  } 
  for (i = 0; i < TEST_PHY_SIZE; i++) {
      ptr[i] = (i % 26) + 'A';
  }
  aed_md_exception(log, 0, (int *)ptr, TEST_PHY_SIZE, __FILE__);
  kfree(ptr);
*/
  //1. AEE function 
  aee_kernel_exception("MD32","MD32 WDT Time out ");
  //2. Reset md32
  printk("WDT Reset md32 ok!\n");
  //3. Call driver's callback 
  for(index=0;index<MD32_MAX_USER;index++)
  {
    if(WDT_FUN.in_use[index] && WDT_FUN.reset_func[index]!=NULL )
    {
      WDT_FUN.reset_func[index](WDT_FUN.private_data[index]);
    }
  } 
}

void reset_md32_by_assert_func()
{
 int index;
 printk("reset_md32_func\n");
 //1. AEE function
 //aed_md_exception(const int *log, int log_size, const int *phy, int phy_size, const char* detail);
 aee_kernel_exception("MD32","MD32 ASSERT ");
 //2. Reset md32
 printk("ASSERT Reset md32 ok!\n");
 //3. Call driver's callback
 for(index=0;index<MD32_MAX_USER;index++)
  {
    if(ASSERT_FUN.in_use[index] && ASSERT_FUN.reset_func[index]!=NULL )
    {
      ASSERT_FUN.reset_func[index](ASSERT_FUN.private_data[index]);
    }
  }
}



char* get_IPC_msg()
{
  dbg_msg("get IPC Msg\n");
  return IPC_data; 
}
void dump_Regs()
{
 dbg_msg("dump_Regs\n");
}
void DMEM_abort_handler(void)
{
  dbg_msg("[MD32] DMEM Abort\n");
  dump_Regs();
}
void PMEM_abort_hander(void)
{
  dbg_msg("[MD32] PMEM Abort\n");
  dump_Regs();
}
void IPC_handler(void)
{
  int index;
  dbg_msg("In IPC_handler\n");
  dbg_msg("[MD32]:%s\n", get_IPC_msg());
  dump_Regs();
  for(index=0;index<MD32_MAX_USER;index++)
  {
    dbg_msg("in use - index %d,%d\n",index,ASSERT_FUN.in_use[index]);
    if((ASSERT_FUN.in_use[index]==1) && (ASSERT_FUN.assert_func[index]!=NULL) )
    {
      dbg_msg("do call back - index %d\n",index);
      ASSERT_FUN.assert_func[index](ASSERT_FUN.private_data[index]);
    }
  }
  queue_work(ASSERT_wq,&reset_md32_assert);
}
void wdt_isr(void)
{
  dump_Regs();
  int index;
  dbg_msg("In wdt_isr\n");
//  WRITE_REG(READ_REG(WDT_CON)& 0xfffff,WDT_CON);  
  for(index=0;index<MD32_MAX_USER;index++)
  {
    dbg_msg("in use - index %d,%d\n",index,WDT_FUN.in_use[index]);
    if((WDT_FUN.in_use[index]==1) && (WDT_FUN.wdt_func[index]!=NULL) )
    {
      dbg_msg("do call back - index %d\n",index);
      WDT_FUN.wdt_func[index](WDT_FUN.private_data[index]);
    }
  }
  queue_work(WDT_wq,&reset_md32_wdt);
}

void md32_error_handler(int irq) //add parameter for simulation
{
  int md32_irq;
   md32_irq=irq;
  //1. Identify interrupt 
   dbg_msg("Identify interrupt");
//   md32_irq=READ_REG(MD2HOST_IPCR);verify in FPGA
   if(md32_irq==DMEM_DISP_INT)
   {
     DMEM_abort_handler();
  //   WRITE_REG(DMEM_DISP_INT,MD2HOST_IPCR);
   }
   else if(md32_irq==PMEM_DISP_INT)
   {
     PMEM_abort_hander();
 //    WRITE_REG(PMEM_DISP_INT,MD2HOST_IPCR);
   }   
   else if(md32_irq==WDT_INT)
   {
     wdt_isr();
 //    WRITE_REG(WDT_INT,MD2HOST_IPCR);
   }
   else if(md32_irq==MD32_IPC_INT)
   {
     IPC_handler();
 //    WRITE_REG(MD32_IPC_INT,MD2HOST_IPCR);
   }
   else
   {
     dbg_msg("undefined interrupt\n");
     ASSERT(1);
   }   

}

int alloc_md32_assert_func(void)
{
  int index;
  for(index=0;index<MD32_MAX_USER;index++)
  {
    if(!ASSERT_FUN.in_use[index])
    {
      ASSERT_FUN.in_use[index]=1;
      return index;
    }
  }
  return -1;
}

int alloc_md32_wdt_func(void)
{
  int index;
  for(index=0;index<MD32_MAX_USER;index++)
  {
    if(!WDT_FUN.in_use[index])
    {
      WDT_FUN.in_use[index]=1; 
      return index;
    }
  }
  return -1;
}
int md32_wdt_register_handler_services( void WDT_FUNC_PTR(void*),void WDT_RESET(void), void* private_data, char *module_name)
{
  int index_cur;
  index_cur=alloc_md32_wdt_func();
  dbg_msg("wdt register index %d\n",index_cur);
  if(index_cur<0)
  {
    dbg_msg("WDT_FUNC is full");
    return -1;
  }
  WDT_FUN.wdt_func[index_cur]=WDT_FUNC_PTR;
  WDT_FUN.reset_func[index_cur]=WDT_RESET;
  WDT_FUN.private_data[index_cur]=private_data;
  strcpy(WDT_FUN.MODULE_NAME[index_cur],module_name);  
  return 1;
}

int md32_assert_register_handler_services( void ASSERT_FUNC_PTR(void*),void ASSERT_RESET (void), void* private_data, char *module_name)
{
  int index_cur;
  index_cur=alloc_md32_assert_func();
  dbg_msg("assert register index %d\n",index_cur);
  if(index_cur<0)
  {
    dbg_msg("ASSERT_FUNC is full");
    return -1;
  }
  ASSERT_FUN.assert_func[index_cur]=ASSERT_FUNC_PTR;
  ASSERT_FUN.reset_func[index_cur]=ASSERT_RESET;
  ASSERT_FUN.private_data[index_cur]=private_data;
  strcpy(ASSERT_FUN.MODULE_NAME[index_cur],module_name);
  return 1;
}


int free_md32_wdt_func(char *module_name)
{
  int index;
  dbg_msg("Flush works in WDT work queue\n");
  flush_workqueue(WDT_wq);
  dbg_msg("Free md32_wdt structure\n");
  for(index=0;index<MD32_MAX_USER;index++)
  {
    if(strcmp(module_name,WDT_FUN.MODULE_NAME[index])==0)
    {
      WDT_FUN.in_use[index]=0;
      WDT_FUN.wdt_func[index]=NULL;
      WDT_FUN.reset_func[index]=NULL;
      WDT_FUN.private_data[index]=NULL;
      return 0;
    }
  }
  dbg_msg("Can't free %s\n",module_name);
  return -1;
}

int free_md32_assert_func(char *module_name)
{
  int index;
  dbg_msg("Flush works in ASSERT work queue\n");
  flush_workqueue(ASSERT_wq);
  dbg_msg("Free md32_assert structure\n");
  for(index=0;index<MD32_MAX_USER;index++)
  {
    if(strcmp(module_name,ASSERT_FUN.MODULE_NAME[index])==0)
    {
      ASSERT_FUN.in_use[index]=0;
      ASSERT_FUN.assert_func[index]=NULL;
      ASSERT_FUN.reset_func[index]=NULL;
      ASSERT_FUN.private_data[index]=NULL;
      return 0;
    }
  }
  dbg_msg("Can't free %s\n",module_name);
  return -1;
}

void reset_md32_assert_func()
{
  memset(&WDT_FUN,0,sizeof(WDT_FUN));
}
void reset_md32_wdt_func()
{
  memset(&ASSERT_FUN,0,sizeof(ASSERT_FUN)); 
}



static struct platform_driver mt_md32_drv =
{
    .driver = {
        .name = "md32",
        .bus = &platform_bus_type,
        .owner = THIS_MODULE,
    },
};

#ifdef MD32_TEST

void wdt_func(void * p)
{
  dbg_msg("client wdt  call back\n");
}
void wdt_reset(void)
{
  dbg_msg("client wdt reset  call back\n");
}
void assert_func(void *p)
{
  dbg_msg("client assert call back\n");
}
void assert_reset(void)
{
 dbg_msg("client assert reset call back\n");
}

static ssize_t foo_show(struct device_driver *driver, char *buf){
   int md32_irq;
   int ret;
   char *pri;
   char *module;
   dbg_msg("Simulation start ==> Test WDT Registration and interrupt\n");
   //1.simulator initialization
   md32_irq= WDT_INT;
   pri="WDT private data";
   module="MD32_WDT";
   //1.Registration
   ret=md32_wdt_register_handler_services(wdt_func,wdt_reset,pri ,module ); 
   if (ret<0)
   {
    dbg_msg("md32_wdt_register failed\n");
   }
   md32_error_handler(md32_irq);
   ret=free_md32_wdt_func(module);
   if (ret<0)
   {
    dbg_msg("md32_wdt_free failed\n");
   }
   
   dbg_msg("\nWDT Verfication PASS\n\n");
   dbg_msg("Simulation start ==> Test ASSERT Registration and interrupt\n");
   //1.simulator initialization
   md32_irq= MD32_IPC_INT;
   pri="ASSERT private data";
   module="MD32_ASSERT";
   //1.Registration
   ret=md32_assert_register_handler_services(assert_func,assert_reset,pri ,module );
   if (ret<0)
   {
    dbg_msg("md32_wdt_register failed\n");
   }
   md32_error_handler(md32_irq);
   ret=free_md32_assert_func(module);
   if (ret<0)
   {
    dbg_msg("md32_wdt_free failed\n");
   }
   dbg_msg("\nASSERT Verfication PASS\n\n");


   dbg_msg("Simulation start ==> Test DMEM Abort\n ");
   //1.simulator initialization
   md32_irq= DMEM_DISP_INT;
   md32_error_handler(md32_irq);
   dbg_msg("\nDMEM abort PASS\n\n");


   dbg_msg("Simulation start ==> Test DMEM Abort\n ");
   //1.simulator initialization
   md32_irq= PMEM_DISP_INT;
   md32_error_handler(md32_irq);
   dbg_msg("\nDMEM PASS\n\n");
   return 0;
}
static ssize_t foo_store(struct device_driver *driver, const char *buf, size_t count){
   return count;
}
DRIVER_ATTR(md32_test, 0644, foo_show, foo_store);
void testcase_init()
{
  int ret;
  ret = driver_register(&mt_md32_drv.driver);
  if (ret) {
        pr_err("Fail to register mt_md32_drv");
    }
  
  ret = driver_create_file(&mt_md32_drv.driver, &driver_attr_md32_test);
    if (ret) {
        pr_err("Fail to create mt_md32_drv sysfs files");
    }

    dbg_msg("create sysfs interface complete\n");
}
#endif
void md32_interrupt_init()
{
  //1. Reset Structures
  reset_md32_assert_func();
  reset_md32_wdt_func();
  //2. register interrupt to CTP
//  request_irq(MD32_IRQ_ID, md32_error_handler, IRQF_TRIGGER_HIGH, "EINT", NULL);
  //3.initial work queue
  WDT_wq=create_workqueue("MD32_WDT_WQ");
  ASSERT_wq=create_workqueue("MD32_ASSERT_WQ");
  INIT_WORK(&reset_md32_wdt,(void(*)(void))reset_md32_by_wdt_func);
  INIT_WORK(&reset_md32_assert,(void(*)(void))reset_md32_by_assert_func);
#ifdef MD32_TEST
  testcase_init();
#endif
}

arch_initcall(md32_interrupt_init);





