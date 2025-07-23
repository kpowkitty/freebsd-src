#ifndef ACPI_NS_DUMP
#define ACPI_NS_DUMP

#include <efi.h>
#include <efilib.h>

ACPI_STATUS dump_acpi_node(ACPI_HANDLE handle, UINT32 level,
    void *context, void **retval);
void acpi_ns_dump(void);

#endif
