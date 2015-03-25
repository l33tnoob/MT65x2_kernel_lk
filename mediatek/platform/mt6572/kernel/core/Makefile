#
# Makefile for the linux kernel.
#
ifeq ($(MTK_INTERNAL), yes)
ifeq ($(MTK_USE_RESERVED_EXT_MEM), yes)
KBUILD_CFLAGS += -DMTK_USE_RESERVED_EXT_MEM
endif
endif

#obj-y := core.o irq.o timer.o mt_dbg.o
obj-y := core.o irq.o timer.o mt_dbg.o
obj-y += mt_devs.o mt_gpt.o mt_boot.o board.o mt_pm_init.o
obj-y += ca7_timer.o
ifeq ($(CONFIG_SMP),y)
ccflags-y += -I$(srctree)
obj-y += mt-smp.o hotplug.o mt-headsmp.o mt_hotplug.o mt_hotplug_mechanism.o
#obj-y += hotplug.o mt-headsmp.o mt_hotplug.o mt_hotplug_mechanism.o
endif
obj-y += eint.o mt_freqhopping.o
obj-$(CONFIG_MT65XX_TRACER) += pmu_v7.o mon_interface.o mt_mon.o mt_emi_bm.o
obj-y += emi_bwl.o emi_mpu.o
obj-y += camera_isp.o
obj-y += camera_pipe_mgr.o
#obj-y += camera_eis.o
#obj-y += camera_fdvt.o
ifneq ($(MTK_PROJECT), mt6572td_evb1_ldvt)
endif
obj-y += mt_pm_ldo.o
obj-y += innercache.o mt_cache_dump.o
# MT6572 GDMA is controlled by CONNSYS
#obj-y += mt_dma.o

ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/power \
             -I$(MTK_PATH_SOURCE)/drivers/wdk
obj-y += mt_spm.o mt_spm_pcm.o mt_spm_suspend.o mt_spm_dpidle.o mt_spm_mcdi.o mt_spm_mtcmos.o mt_spm_mcdi_ldvt.o mt_spm_wdt.o spm_fs.o
obj-y += mt_sleep.o
obj-y += mt_cpufreq.o
obj-y += mt_gpufreq.o
obj-y += mt_cpu_ss.o
obj-y += mt_dormant.o cpu_dormant.o
obj-y += mt_dcm.o mt_idle.o
obj-y += mt_clkmgr.o mt_clkmgr_common.o
#obj-y += mt_golden_setting.o
obj-y += mt_ptp.o
obj-y += sbchk_base.o
#obj-y += sec_osal.o
obj-$(CONFIG_MTK_CIRQ) += mt_cirq.o
obj-$(CONFIG_MTK_MCI) += mt_mci.o
obj-y += mtk_ccci_helper.o
obj-y += mssv.o

ifneq ($(CONFIG_MTK_CCCI_DEVICES),y)
ccflags-y  += -D__USING_DUMMY_CCCI_API__
endif

ccflags-y += -I$(MTK_PATH_PLATFORM)/drivers/power \
             -I$(MTK_PATH_PLATFORM)/drivers/gpu/mali/mali/include \
             -I$(MTK_PATH_PLATFORM)/drivers/gpu/mali/mali/platform \
             -I$(MTK_PATH_PLATFORM)/drivers/devinfo \
             -I$(MTK_PATH_SOURCE)/drivers/wdk

# Hibernation
AFLAGS_swsusp.o	:= -DTEXT_OFFSET=$(TEXT_OFFSET)
obj-$(CONFIG_HIBERNATION)  += cpu_hibernate.o swsusp.o

ifneq ($(MTK_TABLET_HARDWARE), )
ccflags-y += -DMTK_TABLET_PLATFORM=\"$(shell echo $(MTK_TABLET_HARDWARE) | tr a-z A-Z )\"
endif


#KBUILD_AFLAGS	:=$(KBUILD_AFLAGS:-msoft-float=-Wa,-mfpu=neon)
#KT: modify for slt vfp test
KBUILD_AFLAGS	:=$(KBUILD_AFLAGS:-msoft-float=-Wa,-mfpu=neon-vfpv4)
