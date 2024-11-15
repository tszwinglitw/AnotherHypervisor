#include <ntddk.h>
#include <wdf.h>
#include <wdm.h>

extern "C" void inline MainAsm(void);
extern "C" void inline MainAsm2(void);

VOID Unload(IN WDFDRIVER Driver)
{
    UNREFERENCED_PARAMETER(Driver);
}



VOID
MyDriverUnload(PDRIVER_OBJECT DriverObject);


NTSTATUS MyDriverEntry(_In_ PDRIVER_OBJECT     DriverObject,
	_In_ PUNICODE_STRING    RegistryPath)
{

	UNREFERENCED_PARAMETER(RegistryPath);

    NTSTATUS       NtStatus = STATUS_SUCCESS;
    PDEVICE_OBJECT DeviceObject = NULL;
    UNICODE_STRING DriverName, DosDeviceName;

    DbgPrint("DriverEntry Called.");

    RtlInitUnicodeString(&DriverName, L"\\Device\\MyHypervisor");
    RtlInitUnicodeString(&DosDeviceName, L"\\DosDevices\\MyHypervisor");

    NtStatus = IoCreateDevice(DriverObject, 0, &DriverName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);

    if (NtStatus == STATUS_SUCCESS)
    {
        DriverObject->DriverUnload = MyDriverUnload;
        DeviceObject->Flags |= IO_TYPE_DEVICE;
        DeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);
        IoCreateSymbolicLink(&DosDeviceName, &DriverName);
    }
    return NtStatus;
}


VOID
MyDriverUnload(PDRIVER_OBJECT DriverObject)
{
    UNICODE_STRING DosDeviceName;

    DbgPrint("DrvUnload Called !");

    RtlInitUnicodeString(&DosDeviceName, L"\\DosDevices\\MyHypervisor");

    IoDeleteSymbolicLink(&DosDeviceName);
    IoDeleteDevice(DriverObject->DeviceObject);
}