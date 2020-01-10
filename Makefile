PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)as

TARGET = g031

CUBE = STM32Cube$(SERIES)
DRV = $(CUBE)/Drivers
DEFINES = -DUSE_HAL_DRIVER -DUSE_FULL_ASSERT -DUSE_FULL_LL_DRIVER
WARNFLAGS = -Wall -Werror
CFLAGS = $(DEFINES) \
	-mthumb -mfloat-abi=soft  \
	-Os -g3 -Wall -ffunction-sections \
	$(WARNFLAGS)
BUILT = built/$(TARGET)
HEADERS = $(wildcard src/*.h)

OPENOCD = ./scripts/openocd -s ./scripts -f stlink-v2-1.cfg -f stm32g0x.cfg

include targets/$(TARGET)/config.mk

C_SRC += $(wildcard src/*.c) 
C_SRC += $(HALSRC)

C_SRC += targets/$(TARGET)/system.c
AS_SRC = targets/$(TARGET)/startup.s
LD_SCRIPT = targets/$(TARGET)/linker.ld

V = @

OBJ = $(addprefix $(BUILT)/,$(C_SRC:.c=.o) $(AS_SRC:.s=.o))

CPPFLAGS = \
	-I$(DRV)/STM32$(SERIES)xx_HAL_Driver/Inc \
	-I$(DRV)/STM32$(SERIES)xx_HAL_Driver/Inc/Legacy \
	-I$(DRV)/CMSIS/Device/ST/STM32$(SERIES)xx/Include \
	-I$(DRV)/CMSIS/Include \
	-Itargets/$(TARGET) \
	-Isrc

LDFLAGS = -specs=nosys.specs -specs=nano.specs \
	-T"$(LD_SCRIPT)" -Wl,-Map=$(BUILT)/output.map -Wl,--gc-sections

all:
	$(MAKE) -j8 $(BUILT)/binary.hex

drop:
	$(MAKE) TARGET=g031 all
	$(MAKE) TARGET=f031 all

r: run

run: all
	$(OPENOCD) -c "program $(BUILT)/binary.elf verify reset exit"

gdb:
	echo "file $(BUILT)/binary.elf" > built/debug.gdb
	echo "target extended-remote | $(OPENOCD) -f gdbdebug.cfg" >> built/debug.gdb
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
