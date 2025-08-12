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
LAcpiEvaluateObject(lua_State *L)
{
	ACPI_STATUS status;
	ACPI_OBJECT_LIST obj_list;
	ACPI_HANDLE handle = NULL;
	ACPI_OBJECT *objs = NULL;
	UINT32 obj_type = -1;
	UINT32 obj_count = 0;
	ACPI_BUFFER return_buffer = { ACPI_ALLOCATE_BUFFER, NULL };
	const char *pathname;

	if (lua_isuserdata(L, 1)) {
		handle = *(ACPI_HANDLE *)luaL_checkudata(L, 1, "lacpi_node");
	} else if (lua_isstring(L, 1)) {
		pathname = lua_tostring(L, 1);
		if (ACPI_FAILURE(status = AcpiGetHandle(NULL, pathname, &handle))) {
			return luaL_error(L, "AcpiGetHandle failed with status 0x%x", status);
		}
	} else {
		return luaL_error(L, "Expected userdata or string as first argument.");
	}

	if (lua_istable(L, 2)) {
		obj_count = lua_rawlen(L, 2);
		objs = malloc(sizeof(ACPI_OBJECT) * obj_count);
		if (objs == NULL) {
			return luaL_error(L, "Failed to malloc objs.");
		}

		for (int i = 0; i < obj_count; ++i) {
			lua_rawgeti(L, 2, i + 1);
		
			// XXX add all types to switch
			lua_getfield(L, -1, "obj_type");
			obj_type = luaL_checkinteger(L, -1);
			lua_pop(L, 1);
			switch (obj_type) {
				case ACPI_TYPE_INTEGER:
					lua_getfield(L, -1, "value");
					objs[i].Type = ACPI_TYPE_INTEGER;
					objs[i].Integer.Value = luaL_checkinteger(L, -1);
					lua_pop(L, 1);
					break;
				case ACPI_TYPE_STRING:
					lua_getfield(L, -1, "value");
					size_t len;
					const char *str = luaL_checklstring(L, -1, &len);
					objs[i].Type = ACPI_TYPE_STRING;
					objs[i].String.Pointer = (char *)str;
					objs[i].String.Length = (UINT32)len;
					lua_pop(L, 1);
					break;
				default:
					lua_pop(L, 1);
					return luaL_error(L, "Object type not supported.");
			}
			lua_pop(L, 1);
		}

		obj_list.Count = obj_count;
		obj_list.Pointer = objs;
	} else {
		obj_list.Count = 0;
		obj_list.Pointer = NULL;
	}
	
	if (ACPI_FAILURE(status = AcpiEvaluateObject(handle, NULL, (obj_list.Count > 0) ? 
					&obj_list : NULL, &return_buffer))) {
		if (return_buffer.Pointer) {
			AcpiOsFree(return_buffer.Pointer);
		}

		return luaL_error(L, "AcpiEvaluateObject failed with status 0x%x", status);
	}

	// XXX Handle passing the buffer information back to lua
	push_acpi_object(L, return_buffer);

	if (return_buffer.Pointer)
		AcpiOsFree(return_buffer.Pointer);

	return 1;
}

static void
push_acpi_object(lua_state *L, ACPI_OBJECT *obj)
{
	switch (obj->Type) {
		case ACPI_TYPE_INTEGER:
			push_int(L, obj);
			break;
		case ACPI_TYPE_STRING:
			push_str(L, obj);
			break;
		case ACPI_TYPE_BUFFER:
			push_buff(L, obj);
			break;
		case ACPI_TYPE_PACKAGE:
			push_pkg(L, obj);
			break;
		default:
			luaL_error(L, "Unsupported object type %d.", obj->Type);
			break;
	}
}

static void 
push_int(lua_State *L, ACPI_OBJECT *obj)
{
	lua_pushinteger(L, obj->Integer.Value);
}

static void
push_str(lua_State *L, ACPI_OBJECT *obj)
{
	lua_pushlstring(L, obj->String.Pointer, obj->String.Length); 
}

static void
push_buff(lua_State *L, ACPI_OBJECT *obj)
{
	lua_pushlstring(L, (const char*)obj->Buffer.Pointer, obj->Buffer.Length);
}

static void
push_pkg(lua_State *L, ACPI_OBJECT *obj)
{
	lua_newtable(L);
	for (UINT32 i = 0; i < obj->Package.Count; ++i) {
		push_acpi_object(L, &obj->Package.Elements[i]);
		lua_rawseti(L, -2, i + 1);
	}
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
	lacpi_create_mt_gc(L, "lacpi_node", lacpi_node_gc);

	luaL_newlib(L, lacpi_funcs);
	return 1;
}

LUA_ACPI_COMPILE_SET(acpi_util, luaopen_lacpi);
