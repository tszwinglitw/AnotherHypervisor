#include <ntddk.h>
#include <wdf.h>
#include <wdm.h>
#include "Vmx.h"
#include <intrin.h>
#include "Source.h"
#include "Msr.h"
#include "Memory.h"

extern "C" void inline AsmEnableVmxOperation(void);


BOOLEAN IsVmxSupported()
{
	// Check for VMX support
	DbgPrint("[*] IsVmxSupported: Checking for VMX support...");
	int cpuInfo[4] = { -1 };
	__cpuid(cpuInfo, 1);
	if ((cpuInfo[2] & (1 << 5)) == 0)
		return FALSE;


	// BIOS lock check
	DbgPrint("[*] IsVmxSupported: Checking if Ia32FeatureControlMsr is locked...");
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


extern VIRTUAL_MACHINE_STATE* VmGuestState = NULL;

BOOLEAN InitializeVmx()
{
	DbgPrint("[*] InitializeVmx(): Enter");
	if (!IsVmxSupported())
	{
		DbgPrint("[*] VMX is not supported in this machine !");
		return FALSE;
	}

	ULONG ProcessorCount = KeQueryActiveProcessorCount(0);
	DbgPrint("[*] InitializeVmx(): Number of Logical Processors: %d", ProcessorCount);

	DbgPrint("[*] InitializeVmx(): Allocating Memory for VmGuestState");
	VmGuestState = (PVIRTUAL_MACHINE_STATE) ExAllocatePool2(POOL_FLAG_NON_PAGED, sizeof(VIRTUAL_MACHINE_STATE) * ProcessorCount, POOLTAG);
	if (VmGuestState == NULL)
	{
		DbgPrint("[*] InitializeVmx(): Failed to allocate memory for VmGuestState");
		return FALSE;
	}

	KAFFINITY AffinityMask;
	for (UINT32 i = 0; i < KeQueryActiveProcessors(); i++)
	{
		AffinityMask = (1 << i) & 0xffffffff;
		KeSetSystemAffinityThread(AffinityMask);

		DbgPrint("=====================================================");
		DbgPrint("Thread executing in %d th logical processor.", i);
		DbgPrint("Enabling VMX Operation in %d th logical processor.", i);
		AsmEnableVmxOperation();
		AllocateVmxonRegion(&VmGuestState[i]);
		// AllocateVmcsRegion(&VmGuestState[i]);
		// DbgPrint("VMXON Region: %llx \t VMCS Region: %llx", VmGuestState[i].VmxonRegion, VmGuestState[i].VmcsRegion);
		DbgPrint("=====================================================");
	}

	return TRUE;
}