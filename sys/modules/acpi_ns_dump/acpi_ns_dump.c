#include <sys/param.h>
#include <sys/module.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <dev/acpica/acpivar.h>
#include <contrib/dev/acpica/include/acpi.h>

static ACPI_STATUS
dump_acpi_node(ACPI_HANDLE handle, UINT32 level, void *context, void **retval)
{
    ACPI_BUFFER path;
    ACPI_DEVICE_INFO *info;
    ACPI_STATUS status;

    path.Length = ACPI_ALLOCATE_BUFFER;
    path.Pointer = NULL;

    // Get full path of node
    status = AcpiGetName(handle, ACPI_FULL_PATHNAME, &path);
    if (ACPI_FAILURE(status)) {
        printf("acpidump: Failed to get path\n");
        return AE_OK;
    }

    // Get device info
    status = AcpiGetObjectInfo(handle, &info);
    if (ACPI_SUCCESS(status)) {
        printf("acpidump: %s", (char *)path.Pointer);
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

static int
acpi_ns_dump_event(module_t mod, int event, void *arg)
{
    switch (event) {
    case MOD_LOAD:
        printf("ACPI Namespace Dump Module Loaded\n");
        AcpiWalkNamespace(ACPI_TYPE_ANY, ACPI_ROOT_OBJECT, ACPI_UINT32_MAX,
                          dump_acpi_node, NULL, NULL, NULL);
        break;
    case MOD_UNLOAD:
        printf("ACPI Namespace Dump Module Unloaded\n");
        break;
    default:
        return EOPNOTSUPP;
    }
    return 0;
}

DEV_MODULE(acpi_ns_dump, acpi_ns_dump_event, NULL);
MODULE_DEPEND(acpi_ns_dump, acpi, 1, 1, 1);

