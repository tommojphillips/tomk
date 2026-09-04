# ============================================================
# TOMK Makefile GNUWin (GNU Make 3.81 for Windows)
# ============================================================

OUT_FN := tomk

DEBUG ?= 1

# ------------------------------------------------------------
# Toolchain
# ------------------------------------------------------------

CC      := i686-elf-gcc
AS      := nasm
AR      := i686-elf-ar
LD      := i686-elf-ld
OBJCOPY := objcopy
OBJDUMP := i686-elf-objdump

LINKER := src/linker.ld

# ------------------------------------------------------------
# Flags
# ------------------------------------------------------------

CFLAGS := \
	-std=gnu99 \
	-ffreestanding \
	-Wall \
	-Wextra \
	-march=i386 \
	-MMD \
	-MP

ASFLAGS := \
	-f elf32

LDFLAGS := \
	-m elf_i386

ifeq ($(DEBUG),1)
	CFG := debug
	CFLAGS += -g
	ASFLAGS += -g
else
	CFG := release
	CFLAGS += -O2
endif

# ------------------------------------------------------------
# Directories
# ------------------------------------------------------------

SRC_DIR := src
OBJ_DIR := obj/$(CFG)
OUT_DIR := bin/$(CFG)
LIB_DIR := lib/$(CFG)

# ------------------------------------------------------------
# Include paths
# ------------------------------------------------------------

INCLUDES := \
	-I$(SRC_DIR)/libc/include \
	-I$(SRC_DIR)/mm/include \
	-I$(SRC_DIR)/kd/include \
	-I$(SRC_DIR)/disasm/include \
	-I$(SRC_DIR)/driver/include \
	-I$(SRC_DIR)/video/include \
	-I$(SRC_DIR)/init/include \
	-I$(SRC_DIR)/ke/include \
	-I$(SRC_DIR)/include

# ------------------------------------------------------------
# Modules
# ------------------------------------------------------------

MODULES := \
	libc \
	mm \
	kd \
	disasm \
	driver \
	video \
	shell \
	ke \
	init

# ------------------------------------------------------------
# Recursive source finder
# ------------------------------------------------------------

rwildcard = $(foreach d,$(wildcard $(1)/*),$(call rwildcard,$(d),$(2))) $(wildcard $(1)/$(2))

# ------------------------------------------------------------
# Generate module source/object/library variables
# ------------------------------------------------------------

define MODULE_template

$(1)_C := \
	$$(call rwildcard,$$(SRC_DIR)/$(1),*.c)

$(1)_ASM := \
	$$(call rwildcard,$$(SRC_DIR)/$(1),*.asm)

$(1)_OBJ := \
	$$(patsubst $$(SRC_DIR)/$(1)/%.c,$$(OBJ_DIR)/$(1)/%.c.o,$$($(1)_C)) \
	$$(patsubst $$(SRC_DIR)/$(1)/%.asm,$$(OBJ_DIR)/$(1)/%.asm.o,$$($(1)_ASM))

$(1)_LIB := \
	$$(LIB_DIR)/$(1).a

endef

$(foreach module,$(MODULES),$(eval $(call MODULE_template,$(module))))

# ------------------------------------------------------------
# All module libraries
# ------------------------------------------------------------

LIBS := \
	$(foreach module,$(MODULES),$($(module)_LIB))

# ------------------------------------------------------------
# All objects
# ------------------------------------------------------------

OBJECTS := \
	$(foreach module,$(MODULES),$($(module)_OBJ))

# ------------------------------------------------------------
# Dependency files
# ------------------------------------------------------------

DEPS := \
	$(OBJECTS:.o=.d)

# ------------------------------------------------------------
# Default target
# ------------------------------------------------------------

.PHONY: all

all: $(OUT_DIR)/$(OUT_FN).elf

# ------------------------------------------------------------
# Directory creation
# ------------------------------------------------------------

$(OBJ_DIR):
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"

$(LIB_DIR):
	@if not exist "$(LIB_DIR)" mkdir "$(LIB_DIR)"

$(OUT_DIR):
	@if not exist "$(OUT_DIR)" mkdir "$(OUT_DIR)"

# ------------------------------------------------------------
# Static libraries
# ------------------------------------------------------------

define MODULE_library_rule

$$($(1)_LIB): $$($(1)_OBJ)
	@if not exist "$$(dir $$@)" mkdir "$$(dir $$@)"
	@$$(AR) rcs $$@ $$^
	@echo $$@
endef

$(foreach module,$(MODULES),$(eval $(call MODULE_library_rule,$(module))))

# ------------------------------------------------------------
# Link
# ------------------------------------------------------------

$(OUT_DIR)/$(OUT_FN).elf: $(LIBS) $(LINKER) $(OUT_DIR)
	@echo Linking...
	@$(LD) $(LDFLAGS) \
		-Map=$(OUT_DIR)/$(OUT_FN).map \
		-T $(LINKER) \
		-o $@ \
		--start-group \
		$(LIBS) \
		--end-group

ifeq ($(DEBUG),1)
	@$(OBJCOPY) --only-keep-debug $@ $(OUT_DIR)/$(OUT_FN).sym
	@$(OBJCOPY) --strip-debug $@
	@echo $(OUT_DIR)/$(OUT_FN).sym
endif

	@echo $(OUT_DIR)/$(OUT_FN).map
	@echo $@

# ------------------------------------------------------------
# C compilation
# ------------------------------------------------------------

$(OBJ_DIR)/libc/%.c.o: $(SRC_DIR)/libc/%.c
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	@$(CC) -c $< -o $@ $(INCLUDES) -DLIBK $(CFLAGS)
	@echo $<

$(OBJ_DIR)/%.c.o: $(SRC_DIR)/%.c
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	@$(CC) -c $< -o $@ $(INCLUDES) $(CFLAGS)
	@echo $<

# ------------------------------------------------------------
# Assembly
# ------------------------------------------------------------

$(OBJ_DIR)/%.asm.o: $(SRC_DIR)/%.asm
	@if not exist "$(dir $@)" mkdir "$(dir $@)"
	@$(AS) $(ASFLAGS) -MD $(@:.o=.d) -o $@ $<
	@echo $<

# ------------------------------------------------------------
# Clean
# ------------------------------------------------------------

.PHONY: clean

clean:
	@if exist "obj" rmdir /S /Q "obj"
	@if exist "lib" rmdir /S /Q "lib"
	@if exist "bin" rmdir /S /Q "bin"

# ------------------------------------------------------------
# Dependency inclusion
# ------------------------------------------------------------

-include $(DEPS)
