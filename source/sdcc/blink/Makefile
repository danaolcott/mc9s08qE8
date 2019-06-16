##############################################
# Makefile for use with SDCC Compiler and 
# STM8S103F3 Processor
# 
MCU  = mc9s08qe8
ARCH = hc08
DEVICE = MC9S08QE8
PORT = /dev/usbdm4
TARGET  ?= main.ihx

SRCS    := $(wildcard *.c)
OBJS     = $(SRCS:.c=.rel)

CC       = sdcc
LD       = sdld
OBJCOPY  = sdobjcopy

# Compile Flags
CFLAGS   = -m$(ARCH) -p$(MCU) --std-sdcc99
CFLAGS  += -D$(DEVICE)

# Stack and Code Space
# CFLAGS  += --stack-auto
CFLAGS  += --stack-loc 0x00FF
CFLAGS  += --code-loc 0xE000

# Include Paths
I_PATH1 = .
I_PATH2 = /usr/share/sdcc/include
I_PATH3 = /usr/share/sdcc/lib/hc08
INCLUDE = -I$(I_PATH1) -I$(I_PATH2) -I$(I_PATH3) 

# Linker Flags
LDFLAGS  = -m$(ARCH) -l$(ARCH) --out-fmt-ihx

all: $(TARGET) size

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) -o $@

%.rel: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

size:
	@$(OBJCOPY) -I ihex --output-target=binary $(TARGET) $(TARGET).bin
	@echo "----------"
	@echo "Image size:"
	@stat -L -c %s $(TARGET).bin

flash: $(TARGET)
	stm8flash -c stlinkv2 -p $(MCU) -d $(PORT) -w $(TARGET)

clean:
	rm -f *.map *.asm *.rel *.ihx *.o *.sym *.lk *.lst *.rst *.cdb *.bin *.s37

.PHONY: clean all flash
