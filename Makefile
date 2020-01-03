PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)as
CUBE = STM32CubeG0
DRV = $(CUBE)/Drivers
DEFINES = -DUSE_HAL_DRIVER -DSTM32G031xx -DUSE_FULL_ASSERT
CFLAGS = $(DEFINES) \
	-mcpu=cortex-m0plus -mthumb -mfloat-abi=soft  \
	-Os -g3 -Wall -ffunction-sections
BUILT = built
HEADERS = $(wildcard src/*.h)
TEMPLATE = $(CUBE)/Projects/NUCLEO-G031K8/Templates
include toolkit/halsrc.mk
C_SRC += $(wildcard src/*.c) 
C_SRC += $(HALSRC)
C_SRC += $(DRV)/BSP/STM32G0xx_Nucleo_32/stm32g0xx_nucleo_32.c
C_SRC += toolkit/system_stm32g0xx.c
AS_SRC = toolkit/startup_stm32g031xx.s

V = @

LD_SCRIPT = toolkit/STM32G031K8Tx_FLASH.ld
OBJ = $(addprefix $(BUILT)/,$(C_SRC:.c=.o) $(AS_SRC:.s=.o))

CPPFLAGS = \
	-I$(DRV)/STM32G0xx_HAL_Driver/Inc \
	-I$(DRV)/STM32G0xx_HAL_Driver/Inc/Legacy \
	-I$(DRV)/CMSIS/Device/ST/STM32G0xx/Include \
	-I$(DRV)/CMSIS/Include \
	-I$(DRV)/BSP/STM32G0xx_Nucleo_32 \
	-Isrc

LDFLAGS = -specs=nosys.specs -specs=nano.specs \
	-T"$(LD_SCRIPT)" -Wl,-Map=$(BUILT)/output.map -Wl,--gc-sections

all:
	$(MAKE) -j8 $(BUILT)/binary.hex

r: run

run: all
	./scripts/flash.sh

gdb:
	./scripts/bmp.sh > built/debug.gdb
	arm-none-eabi-gdb --command=built/debug.gdb

$(BUILT)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo CC $<
	$(V)$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

$(wildcard $(BUILT)/src/*.o): $(HEADERS)

$(BUILT)/%.o: %.s
	@mkdir -p $(dir $@)
	@echo AS $<
	$(V)$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

$(BUILT)/binary.elf: $(OBJ) Makefile
	@echo LD $@
	$(V)$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) -lm

$(BUILT)/binary.hex: $(BUILT)/binary.elf
	@echo HEX $<
	$(PREFIX)objcopy -O ihex $< $@
	$(PREFIX)size $<

clean:
	rm -rf $(BUILT)
