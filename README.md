# Folder with bad ideas

run 
```
./install-dependencies.sh
echo export PATH=\$PATH:~/opt/cross/bin >> ~/.bashrc # or whatever shell u use.  
```

Will currently (probably) break intermittently because of interrupts clobbering the red zone. Set up separate stack to handle interrupts ok.


## Build
```
i686-elf-as boot.s -o boot.o
i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
i686-elf-gcc -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o kernel.o -lgcc
```

## Qemu

```
grub-mkrescue -o myos.iso qemu

qemu-system-i386 -kernel myos.bin
qemu-system-i386                                 \
  -accel tcg,thread=single                       \
  -cpu core2duo                                  \
  -m 128                                         \
  -no-reboot                                     \
  -drive format=raw,media=cdrom,file=myos.iso    \
  -serial stdio                                  \
  -smp 1                                         \
  -usb                                           \
  -vga std
```

## Maybe if ever do something serious lol
Warning: GNU GRUB, the bootloader used by grub-mkrescue, is licensed under the GNU General Public License. Your iso file contains copyrighted material under that license and redistributing it in violation of the GPL constitutes copyright infringement. The GPL requires you publish the source code corresponding to the bootloader. You need to get the exact source package corresponding to the GRUB package you have installed from your distribution, at the time grub-mkrescue is invoked (as distro packages are occasionally updated). You then need to publish that source code along with your ISO to satisfy the GPL. Alternative, you can build GRUB from source code yourself. Clone the latest GRUB git from savannah (do not use their last release from 2012, it's severely out of date). Run autogen.sh, ./configure and make dist. That makes a GRUB tarball. Extract it somewhere, then build GRUB from it, and install it in a isolated prefix. Add that to your PATH and ensure its grub-mkrescue program is used to produce your iso. Then publish the GRUB tarball of your own making along with your OS release. You're not required to publish the source code of your OS at all, only the code for the bootloader that's inside the iso. 

