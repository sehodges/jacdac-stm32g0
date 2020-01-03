#!/bin/sh

set -e
make
./scripts/openocd -s ./scripts -f stlink-v2-1.cfg -f stm32g0x.cfg -c "program built/binary.elf verify reset exit"
