#pragma once

#include <ntddk.h>

#define POOLTAG 0x54534d56 // VMST, reversed


#define IA32_VMX_BASIC          0x480
#define IA32_VMX_CR0_FIXED0     0x486
#define IA32_VMX_CR0_FIXED1     0x487
#define IA32_VMX_CR4_FIXED0     0x488
#define IA32_VMX_CR4_FIXED1     0x489
 
BOOLEAN IsVmxSupported();
BOOLEAN InitializeVmx();
BOOLEAN TeardownVmx();


typedef union _CRO {
	UINT64 value;
	struct {
		UINT64 PE : 1;        // Protected Mode Enable
		UINT64 MP : 1;        // Monitor Co-Processor
		UINT64 EM : 1;        // Emulation
		UINT64 TS : 1;        // Task Switched
		UINT64 ET : 1;        // Extension Type
		UINT64 NE : 1;        // Numeric Error
		UINT64 reserved1 : 10; // Reserved (6-15)
		UINT64 WP : 1;        // Write Protect
		UINT64 reserved2 : 1; // Reserved (17)
		UINT64 AM : 1;        // Alignment Mask
		UINT64 reserved3 : 10; // Reserved (19-28)
		UINT64 NW : 1;        // Not-Write Through
		UINT64 CD : 1;        // Cache Disable
		UINT64 PG : 1;        // Paging
		UINT64 reserved4 : 32; // Reserved (32-63)
	} bits;
} CR0;

typedef union _CR4 {
    UINT64 value;
    struct {
        unsigned vme : 1;
        unsigned pvi : 1;
        unsigned tsd : 1;
        unsigned de : 1;
        unsigned pse : 1;
        unsigned pae : 1;
        unsigned mce : 1;
        unsigned pge : 1;
        unsigned pce : 1;
        unsigned osfxsr : 1;
        unsigned osxmmexcpt : 1;
        unsigned reserved1 : 2;
        unsigned vmxe : 1;
        unsigned smxe : 1;
        unsigned reserved2 : 2;
        unsigned pcide : 1;
        unsigned osxsave : 1;
        unsigned reserved3 : 1;
        unsigned smep : 1;
        unsigned smap : 1;
    } fields;
} CR4;

extern "C" void inline AsmEnableVmxOperation(void);

