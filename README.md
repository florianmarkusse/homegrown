# Directory with bad ideas

run 
```
./install-dependencies.sh
echo export PATH=\$PATH:~/opt/cross/x86_64/bin >> ~/.bashrc # or whatever shell and target u use.  
```

## To build
```
code/build.sh
```

## To run in Qemu
```
./run-qemu.sh -i code/build/bootloader.iso
```

## To run as a standalone operating system
```
# Plug in USB or other bootable device into your computer
# Find out first on what file system your device is
lsblk # You should see there your device if it is connected
# Fill out the of command with the right path from the above command
sudo dd bs=4M if=/home/florian/Desktop/homegrown/code/build/bootloader.iso of=/dev/sdc1 conv=fdatasync
# Restart your computer with the device still in there
# Go to the boot menu and you should find it there, may need to hit F12
# to go to boot system during startup or something else related to your machine 
```

## x86_64 only
Will currently (probably) break intermittently because of interrupts clobbering the red zone. Set up separate stack to handle interrupts ok.
