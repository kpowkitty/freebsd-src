#include <init_acpi.h>
#include <lutils.h>
#include <lua.h>
#include <lauxlib.h>
#include <contrib/dev/acpica/include/acpi.h>
#include "lacpi.h"
#include "lacpi_object.h"

/*
 * Reference set for all lacpi modules.
 */
void
lacpi_interp_ref(void)
{
	lacpi_object_interp_ref();
}

/*
 * Unpacks all lacpi modules.
 */
static void
lua_acpi_bindings(lua_State *L)
{
	struct lua_acpi_module **mod;

	SET_FOREACH(mod, lua_acpi_modules) {
		(*mod)->init(L);
		lua_setglobal(L, (*mod)->mod_name);
	}
}

/*
 * Function hook for lacpi modules.
 */
void
lua_acpi_register_hook(void)
{
	if (acpi_is_initialized()) {
		lua_acpi_register = lua_acpi_bindings;
	}
}

int
luaopen_lacpi(lua_State *L)
{
	lua_newtable(L);

	luaopen_lacpi_object(L);
	lua_setfield(L, -2, "object");

	return 1;
}

LUA_ACPI_COMPILE_SET(lacpi, luaopen_lacpi);
