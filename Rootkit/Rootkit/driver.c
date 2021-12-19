
#include <ntddk.h>
#include <wdf.h>
#include "intrin.h"
#include <stdio.h>
#include "driver.h"


#define DriverName "Rootkit"

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD KmdfHelloWorldEvtDeviceAdd;

const WCHAR wcgDeviceNameBuffder[] = L"\\Device\\MyDevice";

// https://cpp.hotexamples.com/examples/-/-/IoGetDeviceObjectPointer/cpp-iogetdeviceobjectpointer-function-examples.html
// https://medium.com/@ophirharpaz/kdnet-tutorial-for-noobs-68669778bbd4
// https://docs.microsoft.com/en-us/windows-hardware/drivers/debugger/debug-universal-drivers--kernel-mode-#connectto
PDEVICE_OBJECT pxgRootkitDevice; // Global pointer to our device object

// https://stackoverflow.com/questions/54787115/how-to-debug-a-windows-kernel-driver-properly
// https://www.youtube.com/watch?v=hGKObMOrYZQ


NTSTATUS MyOpen(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);
        
    return STATUS_SUCCESS;
}

NTSTATUS MyClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);
    return STATUS_SUCCESS;
}

NTSTATUS MyRead(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    return STATUS_SUCCESS;
}

NTSTATUS MyWrite(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    return STATUS_SUCCESS;
}

NTSTATUS MyIoControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    PIO_STACK_LOCATION  IrpSp;
    ULONG               FunctionCode;


    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    FunctionCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;
    switch (FunctionCode)
    {

    }

    return STATUS_SUCCESS;
}

NTSTATUS OnStubDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    UNREFERENCED_PARAMETER(DeviceObject);
    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


VOID OnUnload(_In_ PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
    PFILE_OBJECT FileObject;
    UNICODE_STRING DeviceNameUnicodeString;
    NTSTATUS status = STATUS_SUCCESS;

    // Set up our name and symbolic link.
    RtlInitUnicodeString(&DeviceNameUnicodeString, wcgDeviceNameBuffder);

    status = IoGetDeviceObjectPointer(&DeviceNameUnicodeString, GENERIC_ALL, &FileObject, &pxgRootkitDevice);
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "%s:OnUnload:%p:%p:%X:%wZ\n", DriverName, FileObject, pxgRootkitDevice, status, &DeviceNameUnicodeString));
    if (NT_SUCCESS(status)) {
        IoDeleteDevice(pxgRootkitDevice);
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "%s:OnUnload: DeviceObject deleted\n", DriverName));
    }
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT     DriverObject, _In_ PUNICODE_STRING    RegistryPath)
{
    UNREFERENCED_PARAMETER(DriverObject);
    UNREFERENCED_PARAMETER(RegistryPath);
    IDTINFO   idt_info;
    IDTENTRY* idt_entries;
    uint64_t ui64TotalEntries;
    uint64_t ui64TempIndex;
    PFILE_OBJECT FileObject;
    char acTempPrintBuffer[255];

    UNICODE_STRING DeviceNameUnicodeString;

    // NTSTATUS variable to record success or failure
    NTSTATUS status = STATUS_SUCCESS;

    // Allocate the driver configuration object
    WDF_DRIVER_CONFIG config;

    // Print "Hello World" for DriverEntry
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "%s:DriverEntry:\n", DriverName));

    // Initialize the driver configuration object to register the
    // entry point for the EvtDeviceAdd callback, KmdfHelloWorldEvtDeviceAdd
    WDF_DRIVER_CONFIG_INIT(&config, KmdfHelloWorldEvtDeviceAdd);

    // Set up our name and symbolic link.
    RtlInitUnicodeString(&DeviceNameUnicodeString, wcgDeviceNameBuffder);


    // Create device if doesn't exist
    status = IoGetDeviceObjectPointer(&DeviceNameUnicodeString, GENERIC_ALL, &FileObject, &pxgRootkitDevice);
    if (!NT_SUCCESS(status)) {
        // status = IoCreateDevice(DriverObject, 0, &DeviceNameUnicodeString, 0x00001234, 0, FALSE, &pxgRootkitDevice);
    }
    status = STATUS_SUCCESS;

    // Finally, create the driver object
    /*
    status = WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &config, WDF_NO_HANDLE);
    */


    for (ui64TempIndex = 0; ui64TempIndex < IRP_MJ_MAXIMUM_FUNCTION; ui64TempIndex++)
    {
        DriverObject->MajorFunction[ui64TempIndex] = OnStubDispatch;        // For each Major Function(Interface number) we assign a function
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = MyOpen;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = MyClose;
    DriverObject->MajorFunction[IRP_MJ_READ] = MyRead;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = MyWrite;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MyIoControl;

    // Unload routine
    DriverObject->DriverUnload = OnUnload;

    // Load Idt Info
    __sidt(&idt_info);

    idt_entries = (IDTENTRY*)idt_info.ui64IdtBase;
    ui64TotalEntries =  idt_info.ui16IdtLimit/sizeof(IDTENTRY);
    for (ui64TempIndex = 0; ui64TempIndex <= ui64TotalEntries; ui64TempIndex++)
    {
        IDTENTRY* pIdtEntry = &idt_entries[ui64TempIndex];
        uint64_t ui64Address = (pIdtEntry->ui16LowOffset) | (((uint64_t)pIdtEntry->ui16MiddeOffset) << 16) | (((uint64_t)pIdtEntry->ui16MiddeOffset) << 32);

        _snprintf(acTempPrintBuffer, 253, "%s:DriverEntry: IDT[%llu]# 0x%08llX\n", DriverName, ui64TempIndex, ui64Address);
        //KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, _t));
    }

    return status;
}

NTSTATUS KmdfHelloWorldEvtDeviceAdd(_In_ WDFDRIVER Driver, _Inout_ PWDFDEVICE_INIT DeviceInit)
{
    UNREFERENCED_PARAMETER(Driver);
    UNREFERENCED_PARAMETER(DeviceInit);
    NTSTATUS status = STATUS_SUCCESS;

    // Print "Hello World"
    KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "%s:KmdfHelloWorldEvtDeviceAdd:\n",DriverName));

    // Create the device object
    /* WDFDEVICE hDevice;
    status = WdfDeviceCreate(&DeviceInit, WDF_NO_OBJECT_ATTRIBUTES, &hDevice);*/
    return status;
}

