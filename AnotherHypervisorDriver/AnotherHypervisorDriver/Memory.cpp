
#include "Memory.h"
#include "Source.h"
#include <ntddk.h>
#include <wdf.h>
#include <intrin.h>
#include "Msr.h"


#define VMXON_REGION_SIZE 4096
#define VMCS_REGION_SIZE 4096
#define ALIGNMENT_PAGE_SIZE 4096


ULONGLONG VirtualToPhysicalAddress(PVOID Va)
{
    return MmGetPhysicalAddress(Va).QuadPart;
}


PVOID64 PhysicalToVirtualAddress(LONGLONG Pa)
{
    PHYSICAL_ADDRESS PhysicalAddr;
    PhysicalAddr.QuadPart = Pa;
    return MmGetVirtualForPhysical(PhysicalAddr);
}


BOOLEAN AllocateVmcsRegion(IN VIRTUAL_MACHINE_STATE* VmGuestState)
{
    if (KeGetCurrentIrql() > DISPATCH_LEVEL)
    {
        DbgPrint("[*] AllocateVmcsRegion: Current IRQL is higher than DISPATCH_LEVEL\n");
        return FALSE;
    }

    PHYSICAL_ADDRESS MaxPhysicalAddress = { 0 };
    MaxPhysicalAddress.QuadPart = MAXULONG64;

    PVOID Buffer = MmAllocateContiguousMemory(VMCS_REGION_SIZE, MaxPhysicalAddress); // Allocating a 4-KByte Contigous Memory region

    if (Buffer == NULL)
    {
        DbgPrint("[*] Error: Couldn't Allocate Buffer for VMCS Region.");
        return FALSE;
    }
    if (((ULONGLONG)Buffer % ALIGNMENT_PAGE_SIZE) != 0)
    {
        DbgPrint("[*] Error: Allocated buffer is not page aligned.");
        MmFreeContiguousMemory(Buffer);
        return FALSE;
    }

    UINT64 PhysicalBuffer = VirtualToPhysicalAddress(Buffer);

    // Zero-Out memory
    RtlSecureZeroMemory(Buffer, VMCS_REGION_SIZE);
    VOID* AlignedPhysicalBuffer = (BYTE*)((ULONG_PTR)(PhysicalBuffer));
    VOID* AlignedVirtualBuffer = (BYTE*)((ULONG_PTR)(Buffer));

    DbgPrint("[*] Virtual allocated buffer for VMCS at %llx", Buffer);
    DbgPrint("[*] Virtual aligned allocated buffer for VMCS at %llx", AlignedVirtualBuffer);
    DbgPrint("[*] Aligned physical buffer allocated for VMCS at %llx", AlignedPhysicalBuffer);

    // IA32_VMX_BASIC_MSR RevisionId
    // Write the Revision Identifier to the VMCS region

    Ia32VmxBasicMsr VmxBasicMsr = { 0 };

    VmxBasicMsr.all = __readmsr(MSR_IA32_VMX_BASIC);

    DbgPrint("[*] MSR_IA32_VMX_BASIC Revision Identifier %llx", VmxBasicMsr.fields.revision_identifier);

    // Changing Revision Identifier
    // We use the Virtual Address of the Aligned Buffer to write the Revision Identifier
    *(UINT64*)AlignedVirtualBuffer = VmxBasicMsr.fields.revision_identifier;

    UINT8 Status = __vmx_vmptrld((unsigned long long*) & AlignedPhysicalBuffer);
    if (Status)
    {
        DbgPrint("[*] VMCS failed with status %d\n", Status);
        return FALSE;
    }

    VmGuestState->VmcsRegion = (UINT64)AlignedPhysicalBuffer;

    return TRUE;
}


BOOLEAN AllocateVmxonRegion(IN VIRTUAL_MACHINE_STATE* VmGuestState)
{
    DbgPrint("[*] AllocateVmxonRegion: Enter\n");
	if (KeGetCurrentIrql() > DISPATCH_LEVEL)
	{
		DbgPrint("[*] AllocateVmxonRegion: Current IRQL is higher than DISPATCH_LEVEL\n");
		return FALSE;
	}

    return TRUE;
	PHYSICAL_ADDRESS MaxPhysicalAddress = { 0 };
	MaxPhysicalAddress.QuadPart = MAXULONG64;

    PVOID Buffer = MmAllocateContiguousMemory(VMXON_REGION_SIZE, MaxPhysicalAddress); // Allocating a 4-KByte Contigous Memory region

    if (Buffer == NULL)
    {
        DbgPrint("[*] Error: Couldn't Allocate Buffer for VMXON Region.");
        return FALSE; 
    }
    if (((ULONGLONG) Buffer % ALIGNMENT_PAGE_SIZE) != 0)
	{
		DbgPrint("[*] Error: Allocated buffer is not page aligned.");
		// MmFreeContiguousMemory(Buffer);
		return FALSE; 
	}

    UINT64 PhysicalBuffer = VirtualToPhysicalAddress(Buffer);

	UNREFERENCED_PARAMETER(VmGuestState);
    UNREFERENCED_PARAMETER(PhysicalBuffer);



    // Zero-Out memory
    RtlSecureZeroMemory(Buffer, VMXON_REGION_SIZE);
    VOID* AlignedPhysicalBuffer = (BYTE*)((ULONG_PTR)(PhysicalBuffer));
    VOID* AlignedVirtualBuffer = (BYTE*)((ULONG_PTR)(Buffer));

    DbgPrint("[*] Virtual allocated buffer for VMXON at %llx", Buffer);
    DbgPrint("[*] Virtual aligned allocated buffer for VMXON at %llx", AlignedVirtualBuffer);
    DbgPrint("[*] Aligned physical buffer allocated for VMXON at %llx", AlignedPhysicalBuffer);

    // IA32_VMX_BASIC_MSR RevisionId
	// Write the Revision Identifier to the VMXON region

    Ia32VmxBasicMsr VmxBasicMsr = { 0 };

    VmxBasicMsr.all = __readmsr(MSR_IA32_VMX_BASIC);

    DbgPrint("[*] MSR_IA32_VMX_BASIC Revision Identifier %llx", VmxBasicMsr.fields.revision_identifier);

    // Changing Revision Identifier
	// We use the Virtual Address of the Aligned Buffer to write the Revision Identifier
    *(UINT64*)AlignedVirtualBuffer = VmxBasicMsr.fields.revision_identifier;

	// But we need to write the Physical Address of the Aligned Buffer to the VMXON Region
    // https://learn.microsoft.com/en-us/cpp/intrinsics/vmx-on?view=msvc-170
    UINT8 Status = __vmx_on((unsigned long long *) &AlignedPhysicalBuffer);
    if (Status)
    {
        DbgPrint("[*] VMXON failed with status %d\n", Status);
        return FALSE;
    }

    VmGuestState->VmxonRegion = (UINT64) AlignedPhysicalBuffer;

	return TRUE;

}