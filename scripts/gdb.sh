#!/bin/sh

./scripts/bmp.sh > built/debug.gdb
arm-none-eabi-gdb --command=built/debug.gdb
