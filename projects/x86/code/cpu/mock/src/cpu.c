#include "platform-abstraction/cpu.h"
#include "shared/types/types.h"

// TODO: Actually write tests thet fail because these values are not set
// Then implement them so that they pass ???
// Would be nice to have some validation that the virtual stuff works at least
U64 rdmsr([[maybe_unused]] U32 msr) { return 0; }

void wrmsr([[maybe_unused]] U32 msr, [[maybe_unused]] U64 value) {}

void flushTLB() {}

void flushCPUCaches() {}
