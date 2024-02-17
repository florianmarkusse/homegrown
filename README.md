# Folder with bad ideas

run 
```
./install-dependencies.sh
echo export PATH=\$PATH:~/opt/cross/x86_64/bin >> ~/.bashrc # or whatever shell and target u use.  
```

## x86_64 only
Will currently (probably) break intermittently because of interrupts clobbering the red zone. Set up separate stack to handle interrupts ok.


## Qemu

```
grub-mkrescue -o myos.iso qemu

qemu-system-x86_64 -kernel myos.bin
qemu-system-x86_64                                 \
  -accel tcg,thread=single                       \
  -cpu core2duo                                  \
  -m 128                                         \
  -no-reboot                                     \
  -drive format=raw,media=cdrom,file=result.iso    \
  -serial stdio                                  \
  -smp 1                                         \
  -usb                                           \
  -vga std
```
