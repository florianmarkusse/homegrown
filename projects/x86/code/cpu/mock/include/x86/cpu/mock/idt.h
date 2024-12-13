#ifndef X86_CPU_MOCK_IDT_H
#define X86_CPU_MOCK_IDT_H

void initIDTTest(void *long_jmp[5]);
bool *getTriggeredFaults();
void resetTriggeredFaults();

bool compareInterrupts(bool *expectedFaults);

#endif
