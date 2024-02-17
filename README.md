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

