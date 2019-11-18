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
CFLAGS  += --funsigned-char

# Include Paths
I_PATH1 = .
I_PATH2 = /usr/share/sdcc/include
I_PATH3 = /usr/share/sdcc/lib/hc08
INCLUDE = -I$(I_PATH1) -I$(I_PATH2) -I$(I_PATH3) 

# Linker Flags
LIB_PATH = /usr/share/sdcc/lib/hc08
LDFLAGS  = -m$(ARCH)  --lib-path$(LIB_PATH) -lhc08.lib

LDFLAGS += --out-fmt-ihx
LDFLAGS += --iram-size 0x1FF
LDFLAGS += --stack-loc 0xFF
LDFLAGS += --stack-auto
LDFLAGS += --code-loc 0xE000
LDFLAGS += --data-loc 0x00
LDFLAGS += --oldralloc
LDFLAGS += --xram-loc 0x60
LDFLAGS += --xram-size 0x1FF


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
