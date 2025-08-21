#pragma once

#include <lua.h>
#include <sys/linker_set.h>

#include <contrib/dev/acpica/include/acpi.h>

typedef int (*lua_module_init_fn)(lua_State *L);

struct lua_acpi_module {
	const char *mod_name;
	lua_module_init_fn init;
};

SET_DECLARE(lua_acpi_modules, struct lua_acpi_module);

#define LUA_ACPI_COMPILE_SET(name, initfn)			\
	static struct lua_acpi_module lua_##name =		\
	{							\
		.mod_name = #name,				\
		.init = initfn					\
	};							\
	DATA_SET(lua_acpi_modules, lua_##name)

extern struct lacpi_handle {
	const char*	pathname;
	ACPI_HANDLE 	handle;
};
