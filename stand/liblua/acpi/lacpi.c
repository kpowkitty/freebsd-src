#include "lacpi.h"

void
lacpi_interp_ref(void)
{
	lacpi_object_interp_ref(void);
}

static void
lua_acpi_bindings(lua_State *L)
{
	struct lua_acpi_module **mod;

	SET_FOREACH(mod, lua_acpi_modules) {
		(*mod)->init(L);
		lua_setglobal(L, (*mod)->mod_name);
	}
}

void
lua_acpi_register_hook(void)
{
	if (acpi_is_initialized()) {
		acpi_lua_register = lua_acpi_bindings;
	}
}
