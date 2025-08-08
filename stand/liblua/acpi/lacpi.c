#include <lua.h>
#include <lauxlib.h>

#include <contrib/dev/acpica/include/acpi.h>

#include <lacpi.h>

/*
 * Retrieves opaque pointer (ACPI_HANDLE).
 *
 * A string containing a valid ACPI pathname must be on 
 * top of the Lua stack.
 * Passes back a lacpi_node object. 
 */
static int 
LAcpiGetHandle(lua_State *L)
{
	const char *pathname = luaL_checkstring(L, 1);
	ACPI_HANDLE handle;
	ACPI_STATUS status;

	if (ACPI_FAILURE(status = AcpiGetHandle(NULL, pathname, &handle))) {
		return luaL_error(L, "AcpiGetHandle failed with status 0x%x", status);
	}
	

	struct lacpi_node *node = (struct lacpi_node *)lua_newuserdata(L, sizeof(struct lacpi_node));
	
	node->name = strdup(pathname);
	if (node->name == NULL) {
		return luaL_error(L, "Failed to strdup pathname.");
	}

	node->handle = handle;
	
	luaL_getmetatable(L, "lacpi_node");
	lua_setmetatable(L, -2);
	
	return 1;
}

static int
lacpi_node_gc(lua_State *L)
{
	struct lacpi_node *node = (struct lacpi_node *)lua_checkudata(L, 1, "lacpi_node");
	if (node->name) {
		free(node->name);
		node->name = NULL;
	}

	return 0;
}

static const 
luaL_Reg lacpi_funcs[] = {
	{ "get_handle", LAcpiGetHandle },
	{ NULL, NULL }
};

int
luaopen_lacpi(lua_State *L)
{
	luaL_newmetatable(L, "lacpi_node");

	lua_pushcfunction(L, lacpi_node_gc);
	lua_setfield(L, -2, "__gc");

	lua_pop(L, 1);


	luaL_newlib(L, lacpi_funcs);
	return 1;
}

LUA_ACPI_COMPILE_SET(acpi_util, luaopen_lacpi);
