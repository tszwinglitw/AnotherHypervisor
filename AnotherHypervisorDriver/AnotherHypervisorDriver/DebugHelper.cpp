#include <ntddk.h>
#include <wdf.h>

VOID PrintIrpInfo(PIRP Irp)
{
	PIO_STACK_LOCATION  IrpStack;
	IrpStack = IoGetCurrentIrpStackLocation(Irp);

	PAGED_CODE();

	DbgPrint("\tIrp->AssociatedIrp.SystemBuffer = 0x%p\n", Irp->AssociatedIrp.SystemBuffer);
	DbgPrint("\tIrp->UserBuffer = 0x%p\n", Irp->UserBuffer);
	DbgPrint("\tIrpStack->Parameters.DeviceIoControl.Type3InputBuffer = 0x%p\n", IrpStack->Parameters.DeviceIoControl.Type3InputBuffer);
	DbgPrint("\tIrpStack->Parameters.DeviceIoControl.InputBufferLength = %d\n", IrpStack->Parameters.DeviceIoControl.InputBufferLength);
	DbgPrint("\tIrpStack->Parameters.DeviceIoControl.OutputBufferLength = %d\n", IrpStack->Parameters.DeviceIoControl.OutputBufferLength);

	return;
}