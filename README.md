# Google Summer of Code 2025 @ FreeBSD
---------------
## ACPI Initialization in Loader with Lua Bindings
Intel's Advanced Configuration and Power Interface (ACPI) brought power management out of the BIOS
and into the operating system. We can make it even more powerful by creating an interface
to script it. Currently, the scripting language of choice in FreeBSD (rather than POSIX `sh`, `bash`, or `awk`) 
is Lua. Therefore, this project aims to initialize a portion of ACPI in the FreeBSD bootloader, with respect
to its storage, memory, and stdlib constraints, so we can enumerate and evaluate objects on the device trees to
provide an interface to Lua.

## Milestones
[x] OsdMemory  
[x] `AcpiInitializeSubsystem`  
[x] `AcpiInitializeTables`  
[x] `AcpiEnableSubsystem` in reduced hardware mode, with events enabled  
[x] `AcpiLoadTables`  
[x] `AcpiWalkNamespace`  
[x] `AcpiEvaluateObject`  
[x] ACPICA initialized in loader for amd64  
[ ] Enumerate the ACPI Namespace into human-readable API layer  
[ ] Lua API functionality for evaluating objects  
[ ] Demonstration Lua scripts & man page/handbook update  

**Future goals of this project include arm64 compat.**

## For more information, please consult [my write-up](https://kmpow.com/content/gsoc-writeup)

---------------

## Building
### NOTE: 
To build, one must be on an **amd64 UEFI** FreeBSD image.  

This project is a WIP, and still needs more safety guards.

Unintentionally building this or proceeding without the proper 
safety guards can potentially leave your system in a faulty state. 

In general cases, to recover your system, you need to mount your drive 
on a live iso and replace your drive's `loader.efi` with the live iso image's `loader.efi`. 

### Building
The current working version of this project is found on branch `acpi_init`.

1. Clone the repo & change branches  
```
$ git clone git@github.com:kpowkitty/freebsd-src
$ git checkout acpi_init
```

3. Build the necessary dependencies
```
$ cd stand
$ make libsa
$ make liblua
$ cd efi
$ make libefi
```

4. Build the loader (`-j max_jobs`, where `max_jobs` specifies the number of jobs that `make` can run when you call it)
```
$ cd loader
$ make -j max_jobs
```

5. Install the `loader.efi` image
```
$ make install
# cp /boot/loader.efi /boot/path/to/your/efi/
```

6. Reboot
```
# shutdown -r now
```

At this point, when your system restarts, ACPI has gone through the initialization
sequence in the loader. There is nothing of significance being done with that information yet, though. The next leg of this project will be using this initialization of ACPI to walk the namespace and enumerate its objects into an API layer for Lua.
