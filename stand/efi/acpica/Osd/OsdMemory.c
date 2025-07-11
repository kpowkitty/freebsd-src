/*-
 * Copyright (c) 2025 Kayla Powell
 * Copyright (c) 2000 Mitsaru Iwasaki
 * Copyright (c) 2000 Michael Smith
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

/*
 * 6.2 : Memory Management
 */

#include <efi.h>

#include <acpi.h>

#include <init_acpi.h>

EFI_GUID acpi = ACPI_TABLE_GUID;
EFI_GUID acpi20 = ACPI_20_TABLE_GUID;

void *
AcpiOsAllocate(ACPI_SIZE Size)
{
    return (malloc(Size));
}

void
AcpiOsFree(void *Memory)
{
    free(Memory);
}

void *
AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS PhysicalAddress, ACPI_SIZE Length)
{

	/* Originally, this mapped physical address space to virtual address
	 * space. Instead of going through and removing all calls to this,
	 * we opt to change in place. To have it retain its original
	 * functionality, we need to have it return the physical address, as we 
	 * simply operate in physical address space. */

	return (void *)(PhysicalAddress);
}

void
AcpiOsUnmapMemory(void *LogicalAddress, ACPI_SIZE Length)
{
	/* No-op as we never mapped any memory. */
}

/* We may or may not need this... Commenting out for now. 
ACPI_STATUS
AcpiOsGetPhysicalAddress(void *LogicalAddress,
    ACPI_PHYSICAL_ADDRESS *PhysicalAddress)
{
     We can't necessarily do this, so cop out. 
    return (AE_BAD_ADDRESS);
}

BOOLEAN
AcpiOsReadable (void *Pointer, ACPI_SIZE Length)
{
    return (TRUE);
}

BOOLEAN
AcpiOsWritable (void *Pointer, ACPI_SIZE Length)
{
    return (TRUE);
}

ACPI_STATUS
AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 *Value, UINT32 Width)
{
    void	*LogicalAddress;

    LogicalAddress = pmap_mapdev(Address, Width / 8);
    if (LogicalAddress == NULL)
	return (AE_NOT_EXIST);

    switch (Width) {
    case 8:
	*Value = *(volatile uint8_t *)LogicalAddress;
	break;
    case 16:
	*Value = *(volatile uint16_t *)LogicalAddress;
	break;
    case 32:
	*Value = *(volatile uint32_t *)LogicalAddress;
	break;
    case 64:
	*Value = *(volatile uint64_t *)LogicalAddress;
	break;
    }

    pmap_unmapdev(LogicalAddress, Width / 8);

    return (AE_OK);
}

ACPI_STATUS
AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS Address, UINT64 Value, UINT32 Width)
{
    void	*LogicalAddress;

    LogicalAddress = pmap_mapdev(Address, Width / 8);
    if (LogicalAddress == NULL)
	return (AE_NOT_EXIST);

    switch (Width) {
    case 8:
	*(volatile uint8_t *)LogicalAddress = Value;
	break;
    case 16:
	*(volatile uint16_t *)LogicalAddress = Value;
	break;
    case 32:
	*(volatile uint32_t *)LogicalAddress = Value;
	break;
    case 64:
	*(volatile uint64_t *)LogicalAddress = Value;
	break;
    }

    pmap_unmapdev(LogicalAddress, Width / 8);

    return (AE_OK);
}
*/

ACPI_PHYSICAL_ADDRESS
AcpiOsGetRootPointer (
    void)
{
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
	if (!rsdp) {
		return (0);
	}

	return (ACPI_PHYSICAL_ADDRESS)(uintptr_t)(rsdp);
=======
	return (rsdp->XsdtPhysicalAddress);
>>>>>>> 46d29d6804 (ACPI_DEBUG_OUTPUT initialized)
=======
	return (rsdp->XsdtPhysicalAddress);
>>>>>>> 46d29d6804 (ACPI_DEBUG_OUTPUT initialized)
=======
	return (rsdp->XsdtPhysicalAddress);
>>>>>>> 46d29d6804 (ACPI_DEBUG_OUTPUT initialized)
}
