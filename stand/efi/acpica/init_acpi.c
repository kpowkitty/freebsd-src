/*
 * Copyright (c) 2025 Kayla Powell <kpowkitty@FreeBSD.org>
 * Copyright (c) 2000 Takanori Watanabe <takawata@jp.freebsd.org>
 * Copyright (c) 2000 Mitsuru IWASAKI <iwasaki@jp.freebsd.org>
 * Copyright (c) 2000, 2001 Michael Smith
 * Copyright (c) 2000 BSDi
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <machine/_inttypes.h>

#include <efi.h>

#include <stand.h>


#include <acpi.h>
#include <accommon.h>
#include <acoutput.h>

#include <init_acpi.h>

#define _COMPONENT ACPI_LOADER
ACPI_MODULE_NAME("init_acpi");

/* Holds the description of the acpi0 device. */
static char acpi_desc[ACPI_OEM_ID_SIZE + ACPI_OEM_TABLE_ID_SIZE + 2];

/* Holds the current ACPICA version. */
static char acpi_ca_version[12];

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

/*
 * Intialize the ACPI subsystem and tables.
 */
ACPI_STATUS
acpi_Startup(void)
{
    ACPI_STATUS status;

    ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

    /* Initialize the ACPICA subsystem. */
    if (ACPI_FAILURE(status = AcpiInitializeSubsystem())) {
        printf("ACPI: Could not initialize Subsystem: %s\n",
            AcpiFormatException(status));
        return_VALUE (status);
    }

    /* Initialize the ACPICA tables. */
    if (ACPI_FAILURE(status = AcpiInitializeTables(NULL, 2, TRUE))) {
        printf("ACPI: Table initialisation failed: %s\n",
            AcpiFormatException(status));
        return_VALUE (status);
    }

    return_VALUE (status);
}

/*
 * Detect ACPI and perform early initialisation.
 */
int
acpi_identify(void)
{
    ACPI_PHYSICAL_ADDRESS paddr;
	char oemid[ACPI_OEM_ID_SIZE + 1];
	char oemtableid[ACPI_OEM_TABLE_ID_SIZE + 1];

    ACPI_FUNCTION_TRACE((char *)(uintptr_t)__func__);

    /* Initialize root tables. */
    if (ACPI_FAILURE(acpi_Startup())) {
        printf("ACPI: Try disabling either ACPI or apic support.\n");
        return (ENXIO);
    }

	/*
	 * XXX - Can we simply get the physical address of it and traverse it?
	 * Find out why the driver does the below and decide if it is necessary.
	 */
	if ((paddr = AcpiOsGetRootPointer()) == 0) {
		return (ENXIO);
	}

    printf("Successfully got paddr: 0x%llx\n.", (unsigned long long)paddr);

	/*
	if ((paddr = AcpiOsGetRootPointer()) == 0 ||
        (rsdp = AcpiOsMapMemory(paddr, sizeof(ACPI_TABLE_RSDP))) == NULL)
        return (ENXIO);
    if (rsdp->Revision > 1 && rsdp->XsdtPhysicalAddress != 0)
        paddr = (ACPI_PHYSICAL_ADDRESS)rsdp->XsdtPhysicalAddress;
    else
        paddr = (ACPI_PHYSICAL_ADDRESS)rsdp->RsdtPhysicalAddress;
    AcpiOsUnmapMemory(rsdp, sizeof(ACPI_TABLE_RSDP));

    if ((rsdt = AcpiOsMapMemory(paddr, sizeof(ACPI_TABLE_HEADER))) == NULL)
        return (ENXIO);
	
	Initialize acpi_desc and acpi_ca_version. 
	memcpy(oemid, rsdt->OemId, ACPI_OEM_ID_SIZE);
	oemid[ACPI_OEM_ID_SIZE] = '\0';
	memcpy(oemtableid, rsdt->OemTableId, ACPI_OEM_TABLE_ID_SIZE);
	oemtableid[ACPI_OEM_TABLE_ID_SIZE] = '\0';

	// To be implemented
	//trim_trailing_spaces(oemid);
	//trim_trailing_spaces(oemtableid);

	snprintf(acpi_desc, sizeof(acpi_desc), "%s %s", oemid, oemtableid);

    snprintf(acpi_ca_version, sizeof(acpi_ca_version), "%x", ACPI_CA_VERSION);

	printf("acpi_desc: \"%s\".", acpi_desc);
	printf("acpi_ca_version: %x", acpi_ca_version);
    
	AcpiOsUnmapMemory(rsdt, sizeof(ACPI_TABLE_HEADER));
	*/
    return (0);
}
