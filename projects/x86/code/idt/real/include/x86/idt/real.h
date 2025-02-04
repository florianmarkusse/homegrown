#ifndef X86_IDT_REAL_H
#define X86_IDT_REAL_H

void initIDTTest(void *long_jmp[5]);
bool *getTriggeredFaults();
void resetTriggeredFaults();

bool compareInterrupts(bool *expectedFaults);

#endif
