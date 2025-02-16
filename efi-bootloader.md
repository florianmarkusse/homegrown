# Steps of UEFI application to launch kernel

Arch-specific steps are marked.

1. Set up root address for memory mapping
   - x86: level 4 page table address
2. Collect cpu information
   - x86: cpuids
3. x86: Calibrate clock cycles with rdtsc, not sure why this is done? To calculate how many cycles to wait I guess until a time is over. But we are never waiting?
4. Get kernel information: size/location
5. Read kernel into memory
6. Map kernel into location
   - x86: way of mapping
7. Bootstrap processor work
   1. x86: disable PIC & NMI
   2. x86: prepare GDT
8. x86: Identity map all available memory. I think this is done to avoid issues with <1MiB addresses? Why are we doing it for all memory tho?
9. x86: Identity map GOP frame buffer. (Later I am remapping it in kernel with a PATMapping - maybe do it here if we can do pat mapping in uefi too?
10. Allocate/map/set kernel parameters
11. Get RDSP. Still unclear as to its purpose. Using it for APIC yes, but how? Should be kept tho for future.
12. x86: Check and enable features
    1. GPE global memory paging. (Isn't this already turned on?)
    2. Enable FPU.
    3. Enable SSE (but doesn't work yet?)
    4. Enable XSAVE (Commented out currently, does my CPU support this or not?)
    5. Enable AVX (Commented out currently, does my CPU support this or not?)
13. Exit boot services
14. x86: switch to new memory mapping
15. x86: Jump to kernel

## Proposed new way of initializing X86

### X86 Init

1. cli, the assembly instruction
2. Set up memory mapping (Is this the same as in kernel? If we expose the virtual map, maybe?)
3. Collect CPU information
4. Check and enable features
   1. GPE global memory paging. (Isn't this already turned on?)
   2. Enable FPU.
   3. Enable SSE (but doesn't work yet?)
   4. Enable PAT?
   5. Enable XSAVE (Commented out currently, does my CPU support this or not?)
   6. Enable AVX (Commented out currently, does my CPU support this or not?)
5. Bootstrap processor work
   1. x86: disable PIC & NMI (Necessary??? What are we doing here?)
   2. x86: prepare GDT

### UEFI common (?)

1. Get kernel information: size/location
2. Read kernel into memory
3. Map kernel into location
4. x86: Identity map all available memory. I think this is done to avoid issues with <1MiB addresses? Why are we doing it for all memory tho?
5. x86: Identity map GOP frame buffer. (Later I am remapping it in kernel with a PATMapping - maybe do it here if we can do pat mapping in uefi too?
6. Allocate/map/set kernel parameters
7. Get RDSP. Still unclear as to its purpose. Using it for APIC yes, but how? Should be kept tho for future.
8. Exit boot services

### X86 to kernel

1. x86: switch to new memory mapping
2. x86: Jump to kernel
