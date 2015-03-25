obj-  := dummy.o

ifeq ($(CONFIG_THERMAL), y)
else
    obj-y += mtk_thermal_dummy.o
endif

obj-$(CONFIG_THERMAL) += mtk_thermal_platform.o
obj-$(CONFIG_THERMAL) += mtk_ts_pa_thput.o
ifneq ($(CONFIG_MTK_COMBO_WIFI),)
    ifeq ($(MTK_COMBO_CHIP),CONSYS_6572)
        obj-$(CONFIG_THERMAL) += mtk_ts_wmt.o
    endif
endif

# Only included in non-EVB projects
ifeq ($(CONFIG_POWER_EXT), y)
else
    ifeq ($(CONFIG_MTK_SMART_BATTERY), y)
        # Always align the Makefile in mediatek/platform/mt6572/kernel/drivers/power/
        ifeq ($(MTK_NCP1851_SUPPORT),yes)
            obj-$(CONFIG_THERMAL) += mtk_cooler_bcct.o
        else
            ifeq ($(MTK_BQ24196_SUPPORT),yes) 
                obj-$(CONFIG_THERMAL) += mtk_cooler_bcct.o
            else
                # FAN5405 or PMIC6320 pulse charging or new charger IC
                obj-$(CONFIG_THERMAL) += mtk_cooler_bcct.o
            endif
        endif
    endif
endif



ifeq ($(CONFIG_THERMAL),y)
    obj-y += mtk_ts_cpu.o
    obj-y += mtk_ts_pmic.o
    obj-y += mtk_ts_pa.o    

    ifeq ($(CONFIG_POWER_EXT), y)
    else
        ifeq ($(CONFIG_MTK_SMART_BATTERY), y)
            # Always align the Makefile in mediatek/platform/mt6589/kernel/drivers/power/
            ifeq ($(MTK_NCP1851_SUPPORT),yes)
                obj-y += mtk_ts_battery.o
            else
                ifeq ($(MTK_BQ24196_SUPPORT),yes)
                    obj-y += mtk_ts_battery.o
                else
                    # FAN5405 or PMIC pulse charging
                    obj-y += mtk_ts_battery.o
                endif
            endif
        endif
    endif
    ifeq ($(MTK_PCB_BATTERY_SENSOR), yes)
        obj-y += mtk_ts_battery2.o
    endif
endif
                                          
