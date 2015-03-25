#
# external modem control driver
#

ifneq ($(MTK_EXTERNAL_MODEM_SLOT),)

# EVDO check ----------------------------
ifeq ($(EVDO_DT_SUPPORT), yes)

obj-  := dummy.o

else
ifeq ($(CUSTOM_KERNEL_EXTMD), mt6280)
ccflags-y  += -DMTK_EXTERNAL_MODEM_SLOT_NUM=6280
obj-y += 6280_ctl_chr_dev.o
else
obj-y += ext_md_ctl_chr_dev.o
endif
endif
# EVDO check end ------------------------

else

obj-  := dummy.o

endif


#clean-files += $(patsubst $(obj)/%,%,$(wildcard $(obj)/*~))

#ccflags-y  += -I$(objtree)/../bionic/libc/kernel/common/linux  -I$(obj)/include

