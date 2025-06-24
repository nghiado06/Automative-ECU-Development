# ===== Project Target =====
TARGET = main

# ===== Toolchain =====
CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

# ===== Directories =====
SPL_DIR    = SPL
CORE_DIR   = Core
STARTUP    = startup_stm32f103.s

# ===== Includes =====
CFLAGS  = -mcpu=cortex-m3 -mthumb -O0 -g -Wall -ffreestanding -nostdlib \
          -I$(SPL_DIR)/inc \
          -I$(CORE_DIR)/Dio \
          -I$(CORE_DIR)/Port \
          -I$(CORE_DIR) \
          -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER

LDFLAGS = -T stm32f103.ld -nostdlib -Wl,--gc-sections

# ===== Sources =====
SRCS_C = main.c \
         $(wildcard $(SPL_DIR)/src/*.c) \
         $(wildcard $(CORE_DIR)/Dio/*.c) \
         $(wildcard $(CORE_DIR)/Port/*.c)

SRCS_S = $(STARTUP)

OBJS   = $(SRCS_C:.c=.o) $(SRCS_S:.s=.o)

# ===== Targets =====
all: $(TARGET).bin

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: $(TARGET).bin
	openocd -f interface/stlink.cfg -f target/stm32f1x.cfg \
		-c "program $(TARGET).bin 0x08000000 verify reset exit"

clean:
	rm -f $(OBJS) $(TARGET).elf $(TARGET).bin

.PHONY: all clean flash
