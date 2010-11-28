TARGET= asbestos

PS3_COMPILERS= $(PS3DEV)/ppu/bin

PPU_CC = $(PS3_COMPILERS)/ppu-gcc
PPU_OBJCOPY = $(PS3_COMPILERS)/ppu-objcopy

LDFLAGS = -nostartfiles -nostdlib -mbig-endian -Wl,--gc-sections -Wl,-T,stage2.ld
CFLAGS = -ffreestanding -mbig-endian  -m64 \
	-Os -Wall -I. -ffunction-sections

OBJS = start.o  main.o hvcall.o
STAGE2 = ../data/stage2.bin

all: $(STAGE2)

asbestos.elf: $(OBJS)
	$(PPU_CC) $(LDFLAGS) -o $@ $(OBJS)

%.o : %.S
	$(PPU_CC) $(CFLAGS) -c $< -o $@
%.o : %.c
	$(PPU_CC) $(CFLAGS) -c $< -o $@

$(STAGE2) : $(TARGET).elf
	$(PPU_OBJCOPY) -O binary $< $@
	

# Target: clean project.
clean:
	@rm -f *.o
	@rm -f ../data/stage2.bin
	@rm -f $(TARGET).bin
	@rm -f $(TARGET).elf
.PHONY: all
