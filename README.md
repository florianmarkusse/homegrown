# UEFI-bootloader & OS

run

```
cmd/install-dependencies.sh
cmd/build-go.sh
cmd/compile-run.elf
```

## To run as a standalone operating system

```
# Plug in USB or other bootable device into your computer
# Find out first on what file system your device is
lsblk # You should see there your device if it is connected
# Fill out the of command with the right path from the above command
sudo dd bs=4M if=FLOS_UEFI_IMAGE.hdd of=/dev/sdc1 conv=notrunc
# Restart your computer with the device still in there
# Go to the boot menu and you should find it there, may need to hit F12
# to go to boot system during startup or something else related to your machine
```

## x86_64 only

Will currently (probably) break intermittently because of interrupts clobbering the red zone. Set up separate stack to handle interrupts ok.

### MP system

- Need to take care of MMTR setting? If I end up doing that.
