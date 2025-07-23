SRCTOP?= /usr/src
KMOD    = acpi_ns_dump
SRCS    = acpi_ns_dump.c

CFLAGS+= -I${SRCTOP}/sys
CFLAGS+= -I${SRCTOP}/sys/contrib/dev/acpica/include
CFLAGS+= -I${SRCTOP}/sys/dev/acpica
CFLAGS+= -I/usr/obj/usr/src/amd64.amd64/sys/GENERIC

.include <bsd.kmod.mk>
