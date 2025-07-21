#include <efi.h>

#include <stand.h>

#include <acpi.h>

#include "init_acpi.h"

ACPI_STATUS dump_acpi_node(ACPI_HANDLE handle, UINT32 level,
    void *context, void **retval) 
{
    ACPI_BUFFER path;
    ACPI_DEVICE_INFO *info;
    ACPI_STATUS status;

    path.Length = ACPI_ALLOCATE_BUFFER;
    path.Pointer = NULL;

    // Get full path of node
    status = AcpiGetName(handle, ACPI_FULL_PATHNAME, &path);
    if (ACPI_FAILURE(status)) {
        printf("ACPI: Failed to get path\n");
        return AE_OK;
    }

    // Get device info
    status = AcpiGetObjectInfo(handle, &info);
    if (ACPI_SUCCESS(status)) {
        printf("ACPI Node: %s", (char *)path.Pointer);
        if (info->Valid & ACPI_VALID_HID)
            printf(" HID=%s", info->HardwareId.String);
        if (info->Valid & ACPI_VALID_UID)
            printf(" UID=%s", info->UniqueId.String);
        printf("\n");
        ACPI_FREE(info);
    }

    ACPI_FREE(path.Pointer);
    return AE_OK;
}

/*
 * Confirmation unit test for ACPI initialization in loader.
 */
void acpi_dump(void) 
{
	AcpiWalkNamespace(ACPI_TYPE_ANY, ACPI_ROOT_OBJECT, UINT32_MAX,
            dump_acpi_node, NULL, NULL, NULL);
}
