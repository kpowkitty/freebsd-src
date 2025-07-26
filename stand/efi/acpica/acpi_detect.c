#include <machine/_inttypes.h>

#include <efi.h>

#include <acpi.h>

#include "acpi_detect.h"

/* For ACPI rsdp discovery. */
EFI_GUID acpi = ACPI_TABLE_GUID;
EFI_GUID acpi20 = ACPI_20_TABLE_GUID;
ACPI_TABLE_RSDP *rsdp;

void
acpi_detect(void)
{
	char buf[24];
	int revision;

	feature_enable(FEATURE_EARLY_ACPI);
	if ((rsdp = efi_get_table(&acpi20)) == NULL)
		if ((rsdp = efi_get_table(&acpi)) == NULL)
			return;

	sprintf(buf, "0x%016"PRIxPTR, (uintptr_t)rsdp);
	setenv("acpi.rsdp", buf, 1);
	revision = rsdp->Revision;
	if (revision == 0)
		revision = 1;
	sprintf(buf, "%d", revision);
	setenv("acpi.revision", buf, 1);
	strncpy(buf, rsdp->OemId, sizeof(rsdp->OemId));
	buf[sizeof(rsdp->OemId)] = '\0';
	setenv("acpi.oem", buf, 1);
	sprintf(buf, "0x%016x", rsdp->RsdtPhysicalAddress);
	setenv("acpi.rsdt", buf, 1);
	if (revision >= 2) {
		/* XXX extended checksum? */
		sprintf(buf, "0x%016llx",
		    (unsigned long long)rsdp->XsdtPhysicalAddress);
		setenv("acpi.xsdt", buf, 1);
		sprintf(buf, "%d", rsdp->Length);
		setenv("acpi.xsdt_length", buf, 1);
	}
}
