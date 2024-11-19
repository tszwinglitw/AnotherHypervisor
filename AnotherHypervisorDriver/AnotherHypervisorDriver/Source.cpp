#include <ntddk.h>
#include <wdf.h>
#include <wdm.h>
#include <intrin.h>
#include "Source.h"
#include "Vmx.h"
#include "Msr.h"



// Declaration
VOID MyDriverUnload(PDRIVER_OBJECT DriverObject);
NTSTATUS MyDriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath);
NTSTATUS UnsupportedHandler(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS CreateHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS IoctlHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS ReadHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS WriteHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);
NTSTATUS CloseHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);




NTSTATUS UnsupportedHandler(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	DbgPrint("[*] This is an unsupported operation.\n");
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_NOT_SUPPORTED;
}


NTSTATUS MyDriverEntry(_In_ PDRIVER_OBJECT     DriverObject,
	_In_ PUNICODE_STRING    RegistryPath)
{

	UNREFERENCED_PARAMETER(RegistryPath);

	NTSTATUS       NtStatus = STATUS_SUCCESS;
	PDEVICE_OBJECT DeviceObject = NULL;
	UNICODE_STRING DriverName, DosDeviceName;

	DbgPrint("[*] DriverEntry Called.");

	RtlInitUnicodeString(&DriverName, L"\\Device\\AnotherHypervisor");
	RtlInitUnicodeString(&DosDeviceName, L"\\DosDevices\\AnotherHypervisor");

	NtStatus = IoCreateDevice(DriverObject, 0, &DriverName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &DeviceObject);

	if (NtStatus != STATUS_SUCCESS)
		goto MyDriverEntry_Exit;

	DriverObject->DriverUnload = MyDriverUnload;
	DeviceObject->Flags |= IO_TYPE_DEVICE;
	DeviceObject->Flags &= (~DO_DEVICE_INITIALIZING);
	IoCreateSymbolicLink(&DosDeviceName, &DriverName);

	for (size_t i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++)
		DriverObject->MajorFunction[i] = UnsupportedHandler;

	DriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseHandler;
	DriverObject->MajorFunction[IRP_MJ_CREATE] = CreateHandler;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IoctlHandler;
	DriverObject->MajorFunction[IRP_MJ_READ] = ReadHandler;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = WriteHandler;


MyDriverEntry_Exit:
	return NtStatus;
}


VOID
MyDriverUnload(PDRIVER_OBJECT DriverObject)
{
	UNICODE_STRING DosDeviceName;

	DbgPrint("[*] MyDriverUnload Called !");

	RtlInitUnicodeString(&DosDeviceName, L"\\DosDevices\\AnotherHypervisor");

	IoDeleteSymbolicLink(&DosDeviceName);
	IoDeleteDevice(DriverObject->DeviceObject);
}




NTSTATUS
CreateHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	DbgPrint("[*] CreateHandler: VMX Operation Enabled !");


	if (!InitializeVmx())
	{
		DbgPrint("[*] InitializeVmx failed !");
		Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
		return STATUS_UNSUCCESSFUL;
	}



	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	/* What is IoCompleteRequest ?
	* The IoCompleteRequest routine completes processing of an I/O request and releases the caller's I/O resources. */

	return STATUS_SUCCESS;
}

NTSTATUS
IoctlHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	DbgPrint("[*] IoctlHandler");



	PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
	NTSTATUS NtStatus = STATUS_SUCCESS;
	UINT64 InBufLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;
	UINT64 OutBufLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;

	if (!InBufLength || !OutBufLength)
	{
		NtStatus = STATUS_INVALID_PARAMETER;
		goto EndIoctlHandler;
	}


	switch (IrpStack->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_SIOCTL_METHOD_BUFFERED:
		break;
	case IOCTL_SIOCTL_METHOD_NEITHER:
		break;
	case IOCTL_SIOCTL_METHOD_IN_DIRECT:
		break;
	case IOCTL_SIOCTL_METHOD_OUT_DIRECT:
		break;
	default:
		// The specified I/O control code is unrecognized by this driver.
		NtStatus = STATUS_INVALID_DEVICE_REQUEST;
		DbgPrint("ERROR: unrecognized IOCTL %x\n", IrpStack->Parameters.DeviceIoControl.IoControlCode);
		break;
	}

EndIoctlHandler:

	Irp->IoStatus.Status = NtStatus;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return NtStatus;
}

NTSTATUS
ReadHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	DbgPrint("[*] ReadHandler: Not implemented yet :( !");

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS
WriteHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	DbgPrint("[*] WriteHandler: Not implemented yet :( !");

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS
CloseHandler(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	DbgPrint("[*] CloseHandler");
	TeardownVmx();

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}