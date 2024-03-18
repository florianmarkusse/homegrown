# Current steps

```
make all
```

```
dd if=/dev/zero of=fat.img bs=1k count=1440
mformat -i fat.img -f 1440 ::
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mcopy -i fat.img efi_application.efi ::/EFI/BOOT/BOOTX64.EFI
mkgpt -o hdimage.bin --image-size 4096 --part fat.img --type system
```

Neither of these work :(
```
qemu-system-x86_64 -drive if=pflash,file=fat.img,format=raw
qemu-system-x86_64 -drive if=pflash,file=hdimage.bin,format=raw
```

It does work!
qemu-system-x86_64 -drive if=pflash,file=bios.bin,format=raw -drive file=fat.img,index=2,media=cdrom -net none
