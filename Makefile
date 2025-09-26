\
    include $(DEVKITARM)/ds_rules

    TARGET = flappy_ds
    BUILD = build
    SOURCES = source/main.c
    CFLAGS += -O2 -Wall -Wno-unused-variable -Wno-unused-function
    LDFLAGS += -Wl,-Map,$(BUILD)/$(TARGET).map
    DATA = data/bird.img data/bird.pal data/tube.img data/tube.pal data/font.img data/font.pal
    all: $(BUILD)/$(TARGET).nds
    $(BUILD)/$(TARGET).nds: $(BUILD)/$(TARGET).elf
    	@echo "Packaging .nds (use nds-bootstrap or devkitPro tools to produce final .nds)"
    	@$(Q)cp $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).nds || true
    clean:
    	rm -rf $(BUILD)
