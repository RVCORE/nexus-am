NAME := vectortest
SRCS := $(shell find -L ./src/ -name "*.[cS]")

include $(AM_HOME)/Makefile.app

# CROSS_COMPILE := riscv64-unknown-elf-
# COMMON_FLAGS  := -fno-pic -march=rv64gcv -mcmodel=medany
# CFLAGS        += $(COMMON_FLAGS) -static
# ASFLAGS       += $(COMMON_FLAGS) -O0
# LDFLAGS       += -melf64lriscv

# CC        = $(CROSS_COMPILE)gcc
# CXX       = $(CROSS_COMPILE)g++
# LD        = $(CROSS_COMPILE)ld
# AR        = $(CROSS_COMPILE)ar
# OBJDUMP   = $(CROSS_COMPILE)objdump
# OBJCOPY   = $(CROSS_COMPILE)objcopy

# BUILD_DIR = $(abspath ./build)
# COMMON_FLAGS  := -fno-pic -march=rv64gcv -mcmodel=medany
# ELF = $(BUILD_DIR)/$(NAME)-pk.elf

# $(ELF):
# 	mkdir -p $(BUILD_DIR)
# 	$(CC) $(COMMON_FLAGS) $(SRCS) -o $@
# 	$(OBJDUMP) -d $(ELF) > $(BUILD_DIR)/$(NAME).txt

# dts: $(ELF)
# 	spike --dump-dts $(ELF) > $(BUILD_DIR)/custom.dts

# run: $(ELF)
# 	spike --isa=rv64gcv pk $(ELF) 2> $(BUILD_DIR)/run.log

# clean:
# 	rm -rf $(BUILD_DIR)