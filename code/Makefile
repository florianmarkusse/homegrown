#
# mykernel/c/Makefile
#
# Copyright (C) 2017 - 2021 bzt (bztsrc@gitlab)
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use, copy,
# modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
# This file is part of the BOOTBOOT Protocol package.
# @brief An example Makefile for sample kernel
#
#

CFLAGS = -Wall -fpic -ffreestanding -fno-stack-protector -nostdinc -nostdlib -Iinclude
LDFLAGS =  -nostdlib -n -T link.ld
STRIPFLAGS =  -s -K mmio -K fb -K bootboot -K environment -K initstack

all: mykernel.x86_64.elf mykernel.aarch64.elf mykernel.riscv64.elf

mykernel.x86_64.elf: src/kernel.c
	x86_64-testos-elf-gcc $(CFLAGS) -c src/kernel.c -o build/kernel.o
	x86_64-testos-elf-ld -r -b binary -o build/font.o font.psf
	x86_64-testos-elf-ld $(LDFLAGS) build/kernel.o build/font.o -o build/mykernel.x86_64.elf
	x86_64-testos-elf-strip $(STRIPFLAGS) build/mykernel.x86_64.elf
	x86_64-testos-elf-readelf -hls build/mykernel.x86_64.elf >build/mykernel.x86_64.txt

mykernel.aarch64.elf: kernel.c
	aarch64-elf-gcc $(CFLAGS) -c kernel.c -o kernel.o
	aarch64-elf-ld -r -b binary -o font.o font.psf
	aarch64-elf-ld $(LDFLAGS) kernel.o font.o -o mykernel.aarch64.elf
	aarch64-elf-strip $(STRIPFLAGS) mykernel.aarch64.elf
	aarch64-elf-readelf -hls mykernel.aarch64.elf >mykernel.aarch64.txt

mykernel.riscv64.elf: kernel.c
	riscv64-elf-gcc $(CFLAGS) -c kernel.c -o kernel.o
	riscv64-elf-ld -r -b binary -o font.o font.psf
	riscv64-elf-ld $(LDFLAGS) kernel.o font.o -o mykernel.riscv64.elf
	riscv64-elf-strip $(STRIPFLAGS) mykernel.riscv64.elf
	riscv64-elf-readelf -hls mykernel.riscv64.elf >mykernel.riscv64.txt

clean:
	rm *.o *.elf *.txt || true

