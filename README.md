# Directory with bad ideas

run 
```
./install-dependencies.sh
echo export PATH=\$PATH:~/opt/cross/x86_64/bin >> ~/.bashrc # or whatever shell and target u use.  
cd code
make mykernel.x86_64.elf
cd ../
./create-image.sh
./test-os.sh
```

## x86_64 only
Will currently (probably) break intermittently because of interrupts clobbering the red zone. Set up separate stack to handle interrupts ok.


## To run end-to-end
```
./build.sh && ../create-image.sh && ../test-os.sh
code/build.sh -m Debug && ./create-image.sh && ./test-os.sh
```

## Using gdb to debug

```
# Compile in debug mod and tell QEMU to halt execution until gdb is connected
code/build.sh -m Debug && ./create-image.sh && ./test-os.sh -g

# In another terminal
gdb
target remote localhost:1234
# Load symbols from file or whatever the executable is called.
file code/build/testos-Debug
info functions
break _start
layout asm
c
# Now you can go over the assembly per instruction with 'ni'
```
