CROSS_COMPILE := riscv64-unknown-elf-
COMMON_FLAGS  += -fno-pic -march=rv64gcv -mcmodel=medany -fvisibility=hidden -nostdlib -nostartfiles -D__AM_SPIKE__
CFLAGS        += $(COMMON_FLAGS) -O2
ASFLAGS       += $(COMMON_FLAGS) -O0
LDFLAGS       += -melf64lriscv

AM_SRCS +=	spike/trm.c \
						spike/htif.c \
						spike/start.S \

CFLAGS += -I$(AM_HOME)/am/src/spike/include
CFLAGS += -fdata-sections -ffunction-sections
CFLAGS += -DMAINARGS=\"$(mainargs)\"

CFLAGS  += -DISA_H=\"riscv.h\"
LDFLAGS += -T $(AM_HOME)/am/src/spike/linker.ld --gc-sections -e _start

image:
	@echo + LD "->" $(BINARY_REL).elf
	@$(LD) $(LDFLAGS) -o $(BINARY).elf $(LINK_FILES)
	@$(OBJDUMP) -d $(BINARY).elf > $(BINARY).txt
	@echo + OBJCOPY "->" $(BINARY_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(BINARY).elf $(BINARY).bin

run:
	$(SPIKE_HOME)/build/spike --isa=rv64gchv_Svnapot --varch=vlen:256,elen:64 $(BINARY).elf 
