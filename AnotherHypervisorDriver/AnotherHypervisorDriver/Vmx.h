#pragma once

#define POOLTAG 0x54534d56 // VMST, reversed
 
BOOLEAN IsVmxSupported();

BOOLEAN InitializeVmx();

extern "C" void inline AsmEnableVmxOperation(void);

