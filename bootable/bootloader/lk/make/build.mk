# comment out or override if you want to see the full output of each command
NOECHO ?= @

ifeq ($(if $(filter eng,$(TARGET_BUILD_VARIANT)),$(if $(filter yes, $(MTK_MEM_PRESERVED_MODE_ENABLE)),yes)),yes)
# eng build & enable mem preserved mode
$(BOOTLOADER_OUT)/build-$(PROJECT)/no_pad_lk_header.bin: $(BOOTLOADER_OUT)/build-$(PROJECT)/lk_header.bin
	cp -f $(BOOTLOADER_OUT)/build-$(PROJECT)/lk_header.bin $(BOOTLOADER_OUT)/build-$(PROJECT)/no_pad_lk_header.bin

$(BOOTLOADER_OUT)/build-$(PROJECT)/pad_for_lk.bin: $(BOOTLOADER_OUT)/build-$(PROJECT)/no_pad_lk_header.bin
	@echo padding_size $(shell expr 512 - `stat -c %s $(BOOTLOADER_OUT)/build-$(PROJECT)/no_pad_lk_header.bin` % 512)
	dd if=/dev/zero of=$(BOOTLOADER_OUT)/build-$(PROJECT)/pad_for_lk.bin bs=1 count=0 seek=$(shell expr 512 - `stat -c %s $(BOOTLOADER_OUT)/build-$(PROJECT)/no_pad_lk_header.bin` % 512) 2> /dev/null

.PHONY: $(BOOTLOADER_OUT)/build-$(PROJECT)/lk_header.bin
$(BOOTLOADER_OUT)/build-$(PROJECT)/lk_header.bin: $(OUTELF)
	@echo generating image: $@
	$(NOECHO)$(SIZE) $<
	$(NOCOPY)$(OBJCOPY) -O binary $< $(BOOTLOADER_OUT)/build-$(PROJECT)/lk-no-mtk-header.bin
	../../../mediatek/build/tools/mkimage $(BOOTLOADER_OUT)/build-$(PROJECT)/lk-no-mtk-header.bin LK > $(BOOTLOADER_OUT)/build-$(PROJECT)/lk_header.bin

$(BOOTLOADER_OUT)/build-$(PROJECT)/merge_pre.bin: ../../../mediatek/preloader/Download/flash/preloader_$(PROJECT).bin_sram ../../../mediatek/preloader/Download/flash/preloader_$(PROJECT).bin_mem
	cat ../../../mediatek/preloader/Download/flash/preloader_$(PROJECT).bin_sram ../../../mediatek/preloader/Download/flash/preloader_$(PROJECT).bin_mem > $@

$(BOOTLOADER_OUT)/build-$(PROJECT)/lk_pad.bin: $(BOOTLOADER_OUT)/build-$(PROJECT)/pad_for_lk.bin $(BOOTLOADER_OUT)/build-$(PROJECT)/no_pad_lk_header.bin $(BOOTLOADER_OUT)/build-$(PROJECT)/merge_pre.bin
	cat $(BOOTLOADER_OUT)/build-$(PROJECT)/no_pad_lk_header.bin $(BOOTLOADER_OUT)/build-$(PROJECT)/pad_for_lk.bin > $(BOOTLOADER_OUT)/build-$(PROJECT)/lk_pad.bin

$(OUTBIN): $(BOOTLOADER_OUT)/build-$(PROJECT)/lk_pad.bin $(BOOTLOADER_OUT)/build-$(PROJECT)/merge_pre.bin
	cat $(BOOTLOADER_OUT)/build-$(PROJECT)/lk_pad.bin $(BOOTLOADER_OUT)/build-$(PROJECT)/merge_pre.bin  > $@
else

# user build or do not enable mem preserved mode
$(OUTBIN): $(OUTELF)
	@echo generating image: $@
	$(NOECHO)$(SIZE) $<
	$(NOCOPY)$(OBJCOPY) -O binary $< $@
	$(NOECHO)cp -f $@ $(BOOTLOADER_OUT)/build-$(PROJECT)/lk-no-mtk-header.bin
	../../../mediatek/build/tools/mkimage $@ LK > $(BOOTLOADER_OUT)/build-$(PROJECT)/lk_header.bin
	$(NOECHO)mv $(BOOTLOADER_OUT)/build-$(PROJECT)/lk_header.bin $@
endif

ifeq ($(ENABLE_TRUSTZONE), 1)
$(OUTELF): $(ALLOBJS) $(LINKER_SCRIPT) $(OUTPUT_TZ_BIN)
ifeq ($(BUILD_SEC_LIB),yes)
	@echo delete old security library
	@rm -rf $(LK_TOP_DIR)/app/mt_boot/lib/libauth.a
	@rm -rf $(LK_TOP_DIR)/app/mt_boot/lib/libsec.a
	@rm -rf $(LK_TOP_DIR)/app/mt_boot/lib/libsplat.a
	@rm -rf $(LK_TOP_DIR)/app/mt_boot/lib/libdevinfo.a
	@echo linking security library
	@ar cq $(LK_TOP_DIR)/app/mt_boot/lib/libauth.a $(AUTH_OBJS)
	@ar cq $(LK_TOP_DIR)/app/mt_boot/lib/libsec.a $(SEC_OBJS)
	@ar cq $(LK_TOP_DIR)/app/mt_boot/lib/libsplat.a $(SEC_PLAT_OBJS)
	@ar cq $(LK_TOP_DIR)/app/mt_boot/lib/libdevinfo.a $(DEVINFO_OBJS)
endif
	@echo linking $@
	$(NOECHO)$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) $(OUTPUT_TZ_BIN) $(ALLOBJS) $(LIBGCC) $(LIBSEC) $(LIBSEC_PLAT) -o $@
else
$(OUTELF): $(ALLOBJS) $(LINKER_SCRIPT)
ifeq ($(BUILD_SEC_LIB),yes)
	@echo delete old security library
	@rm -rf $(LK_TOP_DIR)/app/mt_boot/lib/libauth.a
	@rm -rf $(LK_TOP_DIR)/app/mt_boot/lib/libsec.a
	@rm -rf $(LK_TOP_DIR)/app/mt_boot/lib/libsplat.a
	@rm -rf $(LK_TOP_DIR)/app/mt_boot/lib/libdevinfo.a
	@echo linking security library
	@ar cq $(LK_TOP_DIR)/app/mt_boot/lib/libauth.a $(AUTH_OBJS)
	@ar cq $(LK_TOP_DIR)/app/mt_boot/lib/libsec.a $(SEC_OBJS)
	@ar cq $(LK_TOP_DIR)/app/mt_boot/lib/libsplat.a $(SEC_PLAT_OBJS)
	@ar cq $(LK_TOP_DIR)/app/mt_boot/lib/libdevinfo.a $(DEVINFO_OBJS)
endif
	@echo linking $@
	$(NOECHO)$(LD) $(LDFLAGS) -T $(LINKER_SCRIPT) $(ALLOBJS) $(LIBGCC) $(LIBSEC) $(LIBSEC_PLAT) -o $@
endif

$(OUTELF).sym: $(OUTELF)
	@echo generating symbols: $@
	$(NOECHO)$(OBJDUMP) -t $< | $(CPPFILT) > $@

$(OUTELF).lst: $(OUTELF)
	@echo generating listing: $@
	$(NOECHO)$(OBJDUMP) -Mreg-names-raw -d $< | $(CPPFILT) > $@

$(OUTELF).debug.lst: $(OUTELF)
	@echo generating listing: $@
	$(NOECHO)$(OBJDUMP) -Mreg-names-raw -S $< | $(CPPFILT) > $@

$(OUTELF).size: $(OUTELF)
	@echo generating size map: $@
	$(NOECHO)$(NM) -S --size-sort $< > $@

ifeq ($(ENABLE_TRUSTZONE), 1)
$(OUTPUT_TZ_BIN): $(INPUT_TZ_BIN)
	@echo generating TZ output from TZ input
	$(NOECHO)$(OBJCOPY) -I binary -B arm -O elf32-littlearm $(INPUT_TZ_BIN) $(OUTPUT_TZ_BIN)
endif

include arch/$(ARCH)/compile.mk

