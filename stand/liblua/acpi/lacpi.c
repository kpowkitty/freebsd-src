#include <lua.h>
#include <lauxlib.h>

#include <contrib/dev/acpica/include/acpi.h>

#include <lacpi.h>

static int LAcpiGetHandle(lua_State *L)
{
	const char *pathname = luaL_checkstring(L, 1);
	ACPI_HANDLE handle;
	ACPI_STATUS status;

	if (ACPI_SUCCESS(status = AcpiGetHandle(NULL, pathname, &handle))) {
		lua_pushlightuserdata(L, handle);
		return 1;
	}
	
	return luaL_error(L, "AcpiGetHandle failed with status 0x%x", status);
}

static const luaL_Reg lacpi_funcs[] = {
	{ "get_handle", LAcpiGetHandle },
	{ NULL, NULL }
};

int
luaopen_lacpi(lua_State *L)
{
	luaL_newlib(L, lacpi_funcs);
	return 1;
}

LUA_ACPI_COMPILE_SET(acpi_util, luaopen_lacpi);
