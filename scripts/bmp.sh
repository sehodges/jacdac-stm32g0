#!/bin/sh

cat <<EOF
file built/binary.elf
target extended-remote | ./scripts/openocd -s ./scripts -f stlink-v2-1.cfg -f stm32g0x.cfg -f gdbdebug.cfg
EOF
