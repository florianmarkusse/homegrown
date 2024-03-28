- fix install script
    - cp /usr/share/ovmf/OVMF.fd bios.bin  !!!!!
- clang -m64 -ffreestanding -fno-stack-protector -Wall -Wextra -Wconversion -Wno-sign-conversion -Wdouble-promotion -Wvla -W -nostdinc -nostdlib -fno-stack-check -mno-red-zone -fshort-wchar  -target x86_64-unknown-windows hello.c -Iinclude -c -o compiled.o

- qemu-system-x86_64 -drive if=pflash,file=uefi/bios.bin,format=raw
- 
dd if=/dev/zero of=fat.img bs=1k count=1440
mformat -i fat.img -f 1440 ::
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mcopy -i fat.img efi_application.efi ::/EFI/BOOT/BOOTX64.EFI
mkgpt -o hdimage.bin --image-size 4096 --part fat.img --type system

