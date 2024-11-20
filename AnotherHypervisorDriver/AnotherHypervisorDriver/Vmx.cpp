#include <ntddk.h>
#include <wdf.h>
#include <wdm.h>
#include "Vmx.h"
#include <intrin.h>
#include "Source.h"
#include "Msr.h"
#include "Memory.h"

extern "C" void inline AsmEnableVmxOperation(void);


void CrEnableVmxOperations(void)
{
	//ULONGLONG Ia32VmxCr0Fixed0 = __read/*msr(IA32_VMX_CR0_FIXED0) & 0xFFFFFFFF;
	//ULONGLONG Ia32VmxCr0Fixed1 = __readmsr(IA32_VMX_CR0_FIXED1) & 0xFFFFFFFF;
	//ULONGLONG Ia32VmxCr4Fixed0 = __readmsr(IA32_VMX_CR4_FIXED0) & 0xFFFFFFFF;
	//ULONGLONG Ia32VmxCr4Fixed1 = __readmsr(IA32_VMX_CR4_FIXED1) & 0xFFFFFFFF;
	//__writecr0((__readcr0() | Ia32VmxCr0Fixed0) & Ia32VmxCr0Fixed1);
	//__writecr4((__readcr4() | Ia32VmxCr4*/Fixed0) & Ia32VmxCr4Fixed1);

	CR0 cr0;
	cr0.value = __readcr0() & 0xFFFFFFFF;
	cr0.bits.PE = 1;
	cr0.bits.NE = 1;
	cr0.bits.PG = 1;
	__writecr0(cr0.value);

	CR4 cr4;
	cr4.value = __readcr4() & 0xFFFFFFFF;
	cr4.fields.vmxe = 1;
	__writecr4(cr4.value);
}


BOOLEAN IsVmxSupported()
{
	// Check for VMX support
	DbgPrint("[*] IsVmxSupported: Checking for VMX support...");
	int cpuInfo[4] = { -1 };
	__cpuid(cpuInfo, 1);
	if ((cpuInfo[2] & (1 << 5)) == 0)
		return FALSE;


	// BIOS lock check
	Ia32FeatureControlMsr FeatureControlMsr;
	FeatureControlMsr.all = __readmsr(MSR_IA32_FEATURE_CONTROL);
	if (FeatureControlMsr.fields.lock == 0)
	{
		DbgPrint("[*] Ia32FeatureControlMsr is unlocked, will enable VMXON and Lock it...");
		FeatureControlMsr.fields.lock = TRUE;
		FeatureControlMsr.fields.enable_vmxon = TRUE;
		__writemsr(MSR_IA32_FEATURE_CONTROL, FeatureControlMsr.all);
	}
	else if (FeatureControlMsr.fields.enable_vmxon == FALSE)
	{
		DbgPrint("[*] Ia32FeatureControlMsr is locked, and VMXON is disabled in BIOS");
		// No way to turn on VMXON, return FALSE
		return FALSE;
	}
	// VMXON must be enabled at this point

	return TRUE;
}


extern VIRTUAL_MACHINE_STATE* VmCpuState = NULL;

ULONG DesiredProcessorCount = 4;

BOOLEAN InitializeVmx()
{
	DbgPrint("[*] InitializeVmx(): Enter");
	if (!IsVmxSupported())
	{
		DbgPrint("[*] VMX is not supported in this machine !");
		return FALSE;
	}

	ULONG VMProcessorCount = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);
	DbgPrint("[*] InitializeVmx(): Number of VM CPUs: %d", VMProcessorCount);

	DbgPrint("[*] InitializeVmx(): Allocating Memory for VmCpuState");
	VmCpuState = (PVIRTUAL_MACHINE_STATE)ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(VIRTUAL_MACHINE_STATE) * DesiredProcessorCount, POOLTAG);
	if (VmCpuState == NULL)
	{
		DbgPrint("[*] InitializeVmx(): Failed to allocate memory for VmCpuState");
		return FALSE;
	}

	KAFFINITY AffinityMask;
	for (UINT32 i = 0; i < DesiredProcessorCount; i++)
	{
		AffinityMask = 0;
		AffinityMask = (1 << i) & 0xffffffff;
		KeSetSystemAffinityThreadEx(AffinityMask);
		ULONG curr = KeGetCurrentProcessorNumber();
		DbgPrint("=====================================================");
		DbgPrint("Enabling VMX Operation in %dth VM processor.", curr);
		CrEnableVmxOperations(); // Change bits in CR0 and CR4
		AllocateVmxonRegion(&VmCpuState[i]);
		AllocateVmcsRegion(&VmCpuState[i]);
		DbgPrint("=====================================================");
	}

	return TRUE;
}


BOOLEAN TeardownVmx()
{
	KAFFINITY AffinityMask;
	for (UINT32 i = 0; i < DesiredProcessorCount; i++)
	{
		AffinityMask = 0;
		AffinityMask = (1 << i) & 0xffffffff;
		KeSetSystemAffinityThreadEx(AffinityMask);
		ULONG curr = KeGetCurrentProcessorNumber();
		__vmx_off();
		DbgPrint("Tearing Down VMX Operation in %dth VM processor.", curr);
		MmFreeContiguousMemory(PhysicalToVirtualAddress(VmCpuState[i].VmxonRegion));
		MmFreeContiguousMemory(PhysicalToVirtualAddress(VmCpuState[i].VmcsRegion));
	}
	return TRUE;
}