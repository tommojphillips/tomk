# ============================================================
# TOMK Makefile GNUWin (GNU Make 3.81 for Windows)
# ============================================================

OUT_FN := kernel

DEBUG ?= 1

CC := i686-elf-gcc
AS := nasm
LD := i686-elf-ld
OBJCOPY := objcopy
OBJDUMP := i686-elf-objdump
LINKER := src/$(OUT_FN)/linker.ld

CFLAGS := -std=gnu99 -ffreestanding -Wall -Wextra -march=i386 -MMD -MP
ASFLAGS := -f elf32
LDFLAGS := -m elf_i386

ifeq ($(DEBUG),1)
	CFG := debug
	CFLAGS += -g
	ASFLAGS += -g
else
	CFG := release
	CFLAGS += -O2
endif

OBJ_DIR := obj/$(CFG)
OUT_DIR := bin/$(CFG)

INCLUDES := \
	-Isrc/kernel/include \
	-Isrc/driver/include \
	-Isrc/libc/include \
	-Isrc/disasm

# ------------------------------------------------------------
# Source files
# ------------------------------------------------------------

rwildcard = $(foreach d,$(wildcard $(1)/*),$(call rwildcard,$(d),$(2))) $(wildcard $(1)/$(2))

BOOT_ASM := $(call rwildcard,src/boot,*.asm)

LIBC_C := $(call rwildcard,src/libc,*.c)
LIBC_ASM := $(call rwildcard,src/libc,*.asm)

DRIVER_C := $(call rwildcard,src/driver,*.c)
DRIVER_ASM := $(call rwildcard,src/driver,*.asm)

KERNEL_C := $(call rwildcard,src/kernel,*.c)
KERNEL_ASM := $(call rwildcard,src/kernel,*.asm)

DISASM_C := $(call rwildcard,src/disasm,*.c)
DISASM_ASM := $(call rwildcard,src/disasm,*.asm)

# ------------------------------------------------------------
# Object files
# ------------------------------------------------------------

BOOT_OBJ := \
	$(patsubst src/boot/%.asm,$(OBJ_DIR)/boot/%.asm.o,$(BOOT_ASM))

LIBC_OBJ := \
	$(patsubst src/libc/%.c,$(OBJ_DIR)/libc/%.c.o,$(LIBC_C)) \
	$(patsubst src/libc/%.asm,$(OBJ_DIR)/libc/%.asm.o,$(LIBC_ASM))

DRIVER_OBJ := \
	$(patsubst src/driver/%.c,$(OBJ_DIR)/driver/%.c.o,$(DRIVER_C)) \
	$(patsubst src/driver/%.asm,$(OBJ_DIR)/driver/%.asm.o,$(DRIVER_ASM))

KERNEL_OBJ := \
	$(patsubst src/kernel/%.c,$(OBJ_DIR)/kernel/%.c.o,$(KERNEL_C)) \
	$(patsubst src/kernel/%.asm,$(OBJ_DIR)/kernel/%.asm.o,$(KERNEL_ASM))

DISASM_OBJ := \
	$(patsubst src/disasm/%.c,$(OBJ_DIR)/disasm/%.c.o,$(DISASM_C)) \
	$(patsubst src/disasm/%.asm,$(OBJ_DIR)/disasm/%.asm.o,$(DISASM_ASM))

OBJECTS := \
	$(BOOT_OBJ) \
	$(LIBC_OBJ) \
	$(DRIVER_OBJ) \
	$(KERNEL_OBJ) \
	$(DISASM_OBJ)

# ------------------------------------------------------------
# Dependency files
# ------------------------------------------------------------

DEPS := $(OBJECTS:.o=.d)

# ------------------------------------------------------------
# Default target
# ------------------------------------------------------------

.PHONY: all

all: $(OUT_DIR)/$(OUT_FN).elf

# ------------------------------------------------------------
# Directories
# ------------------------------------------------------------

$(OUT_DIR):
	@if not exist "$(OUT_DIR)" mkdir "$(OUT_DIR)"
	
$(OBJ_DIR):
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"
# ------------------------------------------------------------
# Link
# ------------------------------------------------------------

$(OUT_DIR)/$(OUT_FN).elf: $(OBJECTS) $(LINKER) $(OUT_DIR)
	@$(LD) $(LDFLAGS) -Map=$(OUT_DIR)/$(OUT_FN).map -T $(LINKER) -o $@ $(OBJECTS)

ifeq ($(DEBUG),1)
	@$(OBJCOPY) --only-keep-debug $@ $(OUT_DIR)/$(OUT_FN).sym
	@$(OBJCOPY) --strip-debug $@
	@echo out -^> $(OUT_DIR)/$(OUT_FN).sym
endif
	@echo out -^> $(OUT_DIR)/$(OUT_FN).map
	@echo out -^> $@

# ------------------------------------------------------------
# C compilation
# ------------------------------------------------------------

$(OBJ_DIR)/libc/%.c.o: src/libc/%.c
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	@$(CC) -c $< -o $@ $(INCLUDES) -DLIBK $(CFLAGS) >nul 2>&1
	@echo $<

$(OBJ_DIR)/driver/%.c.o: src/driver/%.c
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	@$(CC) -c $< -o $@ $(INCLUDES) $(CFLAGS)
	@echo $<

$(OBJ_DIR)/kernel/%.c.o: src/kernel/%.c
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	@$(CC) -c $< -o $@ $(INCLUDES) $(CFLAGS)
	@echo $<

$(OBJ_DIR)/disasm/%.c.o: src/disasm/%.c
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	@$(CC) -c $< -o $@ $(INCLUDES) $(CFLAGS)
	@echo $<

# ------------------------------------------------------------
# Assembly
# ------------------------------------------------------------

$(OBJ_DIR)/boot/%.asm.o: src/boot/%.asm
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	@$(AS) $(ASFLAGS) -MD $(@:.o=.d) -o $@ $<
	@echo $<

$(OBJ_DIR)/libc/%.asm.o: src/libc/%.asm
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	@$(AS) $(ASFLAGS) -MD $(@:.o=.d) -o $@ $<
	@echo $<

$(OBJ_DIR)/driver/%.asm.o: src/driver/%.asm
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	@$(AS) $(ASFLAGS) -MD $(@:.o=.d) -o $@ $<
	@echo $<

$(OBJ_DIR)/kernel/%.asm.o: src/kernel/%.asm
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	@$(AS) $(ASFLAGS) -MD $(@:.o=.d) -o $@ $<
	@echo $<

$(OBJ_DIR)/disasm/%.asm.o: src/disasm/%.asm
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	@$(AS) $(ASFLAGS) -MD $(@:.o=.d) -o $@ $<
	@echo $<

# ------------------------------------------------------------
# Clean
# ------------------------------------------------------------

.PHONY: clean

clean:
	@if exist "obj" rmdir /S /Q "obj"
	@if exist "bin" rmdir /S /Q "bin"

# ------------------------------------------------------------
# Dependency inclusion
# ------------------------------------------------------------

-include $(DEPS)
