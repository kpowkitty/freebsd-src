#ifndef INIT_CONF
#define INIT_CONF

#include <efi.h>
#include <efilib.h>

ACPI_STATUS print_node_callback(ACPI_HANDLE Object, UINT32 NestingLevel,
    void *Context, void **ReturnValue);
void init_conf(void);

#endif
