#include "ExtendedPageTable.h"
#include "Memory.h"
#include "Source.h"
#include <ntddk.h>
#include <wdf.h>
#include <wdm.h>
#include <intrin.h>
#include "Vmx.h"
#include "Msr.h"

#define EPT_DIRTYANDACCESS_ENABLED 0


UINT64 InitialExtendedPageTablePointer()
{
	PAGED_CODE();
	EPTP* EptPointer = (EPTP*)ExAllocatePool2(POOL_FLAG_NON_PAGED, PAGE_SIZE, POOLTAG);
	EPT_PML4E* EptPml4e = (EPT_PML4E*)ExAllocatePool2(POOL_FLAG_NON_PAGED, PAGE_SIZE, POOLTAG);
	EPT_PDPTE* EptPdPte = (EPT_PDPTE*)ExAllocatePool2(POOL_FLAG_NON_PAGED, PAGE_SIZE, POOLTAG);
	EPT_PDE* EptPde = (EPT_PDE*)ExAllocatePool2(POOL_FLAG_NON_PAGED, PAGE_SIZE, POOLTAG);
	EPT_PTE* EptPte = (EPT_PTE*)ExAllocatePool2(POOL_FLAG_NON_PAGED, PAGE_SIZE, POOLTAG);

	// Lazy error handling, will leak memory if allocation fails
	if (EptPointer == NULL || EptPml4e == NULL || EptPdPte == NULL || EptPde == NULL || EptPte == NULL)
	{
		DbgPrint("[*] Error: Couldn't Allocate Memory for EPT Structures");
		return 0;
	}

	RtlZeroMemory(EptPointer, PAGE_SIZE);
	RtlZeroMemory(EptPml4e, PAGE_SIZE);
	RtlZeroMemory(EptPdPte, PAGE_SIZE);
	RtlZeroMemory(EptPde, PAGE_SIZE);
	RtlZeroMemory(EptPte, PAGE_SIZE);

	UINT32 nGuestPages = 8;
	UINT64 GuestMemAddr = (UINT64)ExAllocatePool2(POOL_FLAG_NON_PAGED, PAGE_SIZE * nGuestPages, POOLTAG);
	if (GuestMemAddr == NULL)
	{
		DbgPrint("[*] Error: Couldn't Allocate Memory for Guest Pages");
		return 0;
	}
	RtlZeroMemory((PVOID64)GuestMemAddr, PAGE_SIZE * nGuestPages);

	// Creating the first n entries in the EPT Level 1 Table
	for (size_t i = 0; i < nGuestPages; i++)
	{
		EptPte[i].Fields.AccessedFlag = 0;
		EptPte[i].Fields.DirtyFlag = 0;
		EptPte[i].Fields.EPTMemoryType = 6;
		EptPte[i].Fields.Execute = 1;
		EptPte[i].Fields.ExecuteForUserMode = 0;
		EptPte[i].Fields.IgnorePAT = 0;
		EptPte[i].Fields.PhysicalAddress = (VirtualToPhysicalAddress((PVOID)(GuestMemAddr + (i * PAGE_SIZE))) / PAGE_SIZE);
		EptPte[i].Fields.Read = 1;
		EptPte[i].Fields.SuppressVE = 0;
		EptPte[i].Fields.Write = 1;
	}

	// Setting up PDE
	EptPde[0].Fields.Accessed = 0;
	EptPde[0].Fields.Execute = 1;
	EptPde[0].Fields.ExecuteForUserMode = 0;
	EptPde[0].Fields.Ignored1 = 0;
	EptPde[0].Fields.Ignored2 = 0;
	EptPde[0].Fields.Ignored3 = 0;
	EptPde[0].Fields.PhysicalAddress = (VirtualToPhysicalAddress(&EptPte[0]) / PAGE_SIZE);
	EptPde[0].Fields.Read = 1;
	EptPde[0].Fields.Reserved1 = 0;
	EptPde[0].Fields.Reserved2 = 0;
	EptPde[0].Fields.Write = 1;

	// Setting up PDPTE
	EptPdPte[0].Fields.Accessed = 0;
	EptPdPte[0].Fields.Execute = 1;
	EptPdPte[0].Fields.ExecuteForUserMode = 0;
	EptPdPte[0].Fields.Ignored1 = 0;
	EptPdPte[0].Fields.Ignored2 = 0;
	EptPdPte[0].Fields.Ignored3 = 0;
	EptPdPte[0].Fields.PhysicalAddress = (VirtualToPhysicalAddress(&EptPde[0]) / PAGE_SIZE);
	EptPdPte[0].Fields.Read = 1;
	EptPdPte[0].Fields.Reserved1 = 0;
	EptPdPte[0].Fields.Reserved2 = 0;
	EptPdPte[0].Fields.Write = 1;

	// Setting up PML4E
	EptPml4e[0].Fields.Accessed = 0;
	EptPml4e[0].Fields.Execute = 1;
	EptPml4e[0].Fields.ExecuteForUserMode = 0;
	EptPml4e[0].Fields.Ignored1 = 0;
	EptPml4e[0].Fields.Ignored2 = 0;
	EptPml4e[0].Fields.Ignored3 = 0;
	EptPml4e[0].Fields.PhysicalAddress = (VirtualToPhysicalAddress(&EptPdPte[0]) / PAGE_SIZE);
	EptPml4e[0].Fields.Read = 1;
	EptPml4e[0].Fields.Reserved1 = 0;
	EptPml4e[0].Fields.Reserved2 = 0;
	EptPml4e[0].Fields.Write = 1;

	// Finally, setting up the EPTP
	// Do we want to enable Dirty and Access Enabled bits
	EptPointer->Fields.DirtyAndAceessEnabled = EPT_DIRTYANDACCESS_ENABLED;
	EptPointer->Fields.MemoryType = 6;
	EptPointer->Fields.PageWalkLength = 3;
	EptPointer->Fields.PML4Address = (VirtualToPhysicalAddress(&EptPml4e[0]) / PAGE_SIZE);

	DbgPrint("[*] EPTP: %llx\n", EptPointer);
	return (UINT64)EptPointer;
}