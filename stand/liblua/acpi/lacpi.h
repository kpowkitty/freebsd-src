#ifndef _LACPI_H_
#define _LACPI_H_

#include <lua.h>
#include <sys/linker_set.h>

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

#endif
