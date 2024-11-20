#pragma once
#include <ntddk.h>
#include "Source.h"



ULONGLONG VirtualToPhysicalAddress(PVOID Va);

PVOID64 PhysicalToVirtualAddress(LONGLONG Pa);

BOOLEAN AllocateVmcsRegion(IN VIRTUAL_MACHINE_STATE* GuestState);

BOOLEAN AllocateVmxonRegion(IN VIRTUAL_MACHINE_STATE* VmGuestState);


