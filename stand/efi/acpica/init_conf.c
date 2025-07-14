#include <efi.h>

#include <stand.h>

#include <acpi.h>

#include "init_acpi.h"

ACPI_STATUS print_node_callback(ACPI_HANDLE Object, UINT32 NestingLevel,
    void *Context, void **ReturnValue) {
	ACPI_BUFFER pathbuf;
	char path[128];
	ACPI_STATUS status;

	pathbuf.Length = sizeof(path);
	pathbuf.Pointer = path;
	
	// Do we want to keep walking if failed? XXX
	status = AcpiGetName(Object, ACPI_FULL_PATHNAME, &pathbuf);
	if (ACPI_FAILURE(status)) {
		printf("ACPI: Failed to get path\n");
		return (AE_OK);
	}

	printf("ACPI Node: \"%s\".\n", path);

	return (AE_OK);
}

/*
 * Confirmation unit test for ACPI initialization in loader.
 */
void init_conf(void) {
	AcpiWalkNamespace(ACPI_TYPE_ANY, ACPI_ROOT_OBJECT, UINT32_MAX,
            print_node_callback, NULL, NULL, NULL);
}
