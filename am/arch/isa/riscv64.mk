CROSS_COMPILE := riscv64-unknown-elf-
COMMON_FLAGS  += -fno-pic -march=rv64gcv -mcmodel=medany -fvisibility=hidden -nostdlib
CFLAGS        += $(COMMON_FLAGS) -static
ASFLAGS       += $(COMMON_FLAGS) -O0
LDFLAGS       += -melf64lriscv
