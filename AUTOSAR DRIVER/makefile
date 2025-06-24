# ===== Project Target =====
TARGET = main

# ===== Toolchain =====
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

# ===== Directories =====
CORE_DIR  = IO_Driver/Core
SPL_DIR   = SPL
STARTUP_DIR = Startup

# ===== Includes =====
CFLAGS  = -mcpu=cortex-m3 -mthumb -O0 -g -Wall -ffreestanding -nostdlib \
          -I$(SPL_DIR)/inc \
          -I$(CORE_DIR)/DIO \
          -I$(CORE_DIR)/PORT \
          -I$(CORE_DIR)/PWM \
          -I$(CORE_DIR) \
          -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER

LDFLAGS = -T stm32f103.ld -nostdlib -Wl,--gc-sections

# ===== Sources =====
SRCS_C  = main.c \
          $(wildcard $(SPL_DIR)/src/*.c) \
          $(wildcard $(CORE_DIR)/DIO/*.c) \
          $(wildcard $(CORE_DIR)/PORT/*.c) \
          $(wildcard $(CORE_DIR)/PWM/*.c)


SRCS_S  = $(STARTUP_DIR)/startup_stm32f103.s

OBJS   = $(SRCS_C:.c=.o) $(SRCS_S:.s=.o)

# ===== Targets =====
all: $(TARGET).bin

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS) $(STARTUP_DIR)/stm32f103.ld
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: $(TARGET).bin
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
		-c "program $(TARGET).bin 0x08000000 verify reset exit"

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).bin

.PHONY: all clean flash
