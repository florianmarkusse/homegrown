#ifndef PLATFORM_ABSTRACTION_MEMORY_MANAGEMENT_STATUS_H
#define PLATFORM_ABSTRACTION_MEMORY_MANAGEMENT_STATUS_H

#ifdef X86_ARCHITECTURE

void appendPhysicalMemoryManagerStatus();
void appendVirtualMemoryManagerStatus();
#else
#error "Could not match ARCHITECTURE"
#endif

#endif
