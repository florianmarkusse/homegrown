Hi there,

I really like the bootloaders you have written and took it upon myself to try and create a UEFI-bootloader myself. However, it currently only works on Qemu, yet it crashes immediately on my own hardware. I was wondering if you would be up to have a look at the project and can snuff out what is going wrong. I understand that this is not an issue with your project, so feel free to close it if you do not wish to do this.

The code is at [https://gitlab.com/florianmarkusse/homegrown/-/blob/master/projects/uefi/code/src/main.c?ref_type=heads](https://gitlab.com/florianmarkusse/homegrown/-/blob/master/projects/uefi/code/src/main.c?ref_type=heads).

It seems to triple fault the moment I jump into the kernel from my hardware and I am a little stumped as to what can be missing:
- I set up a new memory map after exiting boot services with 
    - The first 4GiB identity mapped (side note: do you perhaps know what UEFI memory types need to be mapped in the OS? The UEFI spec is pretty hand-wavy I find.)
    - and the kernel mapped to the higher half
- Set up dummy exception handlers 
- Set up a new stack that is also mapped

Perhaps something is going on with the red zone? This seems to not be the case as UEFI disables interrupts by default. Also, people seem to turn off the red zone for their kernels. It seems a little silly to turn it off tho and not make use of it.

I may have some other questions, but if you can point me to what is causing my hardware to triple fault, I would already be very grateful.


Thanks in advance
