#include <lua.h>
#include <lauxlib.h>
#include <lacpi.h>
#include "lacpi_object.h"
#include "lacpi_utils.h"
#include "lacpi.h"

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
	
	node->pathname = strdup(pathname);
	if (node->pathname == NULL) {
		return luaL_error(L, "Failed to strdup pathname.");
	}

	node->handle = handle;
	
	luaL_getmetatable(L, "lacpi_node");
	lua_setmetatable(L, -2);
	
	return 1;
}

/*
 * Lua stack expectations:
 *
 * 1 = userdata ACPI handle ("lacpi_node") or nil
 * 2 = relative pathname string from handle or nil
 * 3 = absolute pathname string or nil
 * 4 = optional table of ACPI objects (ACPI_OBJECT_LIST) to pass as arguments
 *
 * Either a handle (arg 1) or a pathname (arg 3) must be provided.
 * Only if 1 is provided, 2 may also be provided, but not required.
 * Any other arguments not being passed must be nil.
 *
 * Returns: ACPI object or table of ACPI objects.
 */
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
	char *pathname = NULL;

	if (lua_isuserdata(L, 1)) {
		handle = *(ACPI_HANDLE *)luaL_checkudata(L, 1, "lacpi_node");
	}
	
	const char *rel_path = lua_isstring(L, 2) ? lua_tostring(L, 2) : NULL;
	const char *abs_path = lua_isstring(L, 3) ? lua_tostring(L, 3) : NULL;

	pathname = rel_path ? strdup(rel_path) : strdup(abs_path);

	if (handle == NULL && pathname == NULL) {
		return luaL_error(L, 
		    "At least one of handle or pathname must be provided");
	}

	if (lua_istable(L, 4)) {
		obj_count = lua_rawlen(L, 4);
		objs = malloc(sizeof(ACPI_OBJECT) * obj_count);
		if (objs == NULL) {
			return luaL_error(L, "Failed to malloc objs.");
		}

		for (int i = 0; i < obj_count; ++i) {
			lua_rawgeti(L, 4, i + 1);
		
			lua_getfield(L, -1, "obj_type");
			obj_type = lua_int_to_uint32(L, -1, 
			    "Invalid ACPI Object type");
			lua_pop(L, 1);
			build_acpi_obj(L, &objs[i], obj_type);
			lua_pop(L, 1);
		}

		obj_list.Count = obj_count;
		obj_list.Pointer = objs;
	} else {
		obj_list.Count = 0;
		obj_list.Pointer = NULL;
	}
	
	if (ACPI_FAILURE(status = AcpiEvaluateObject(handle, pathname, 
	    (obj_list.Count > 0) ? &obj_list : NULL, &return_buffer))) {
		if (return_buffer.Pointer) {
			AcpiOsFree(return_buffer.Pointer);
		}


		free_acpi_objs(objs, obj_count);

		return luaL_error(L, 
		    "AcpiEvaluateObject failed with status 0x%x", status);
	}

	if (return_buffer.Pointer != NULL) {
		ACPI_OBJECT *ret_obj = (ACPI_OBJECT *)return_buffer.Pointer;

		if (ret_obj->Type == ACPI_TYPE_PACKAGE) {
			lua_newtable(L);
			for (UINT32 i = 0; i < ret_obj->Package.Count; ++i) {
				push_acpi_obj(L, &ret_obj->Package.Elements[i]);
				lua_rawseti(L, -2, i + 1);
			}
		} else {
			push_acpi_obj(L, (ACPI_OBJECT *)return_buffer.Pointer);
		}

		AcpiOsFree(return_buffer.Pointer);
		
		free_acpi_objs(objs, obj_count);
	}

	return 1;
}

static int
lacpi_node_gc(lua_State *L)
{
	struct lacpi_node *node = (struct lacpi_node *)luaL_checkudata(L, 1, 
	    "lacpi_node");
	if (node->pathname) {
		free((void *)node->pathname);
		node->pathname = NULL;
	}

	return 0;
}

void 
lacpi_node_register_mt(lua_State *L)
{
	lacpi_create_mt_gc(L, "lacpi_node", lacpi_node_gc);
}

static const 
luaL_Reg lacpi_object_funcs[] = {
	{ "get_handle", LAcpiGetHandle },
	{ "evaluate", LAcpiEvaluateObject },
	{ NULL, NULL }
};

int
luaopen_lacpi_object(lua_State *L)
{
	luaL_newlib(L, lacpi_object_funcs);
	return 1;
}

void
lacpi_object_interp_ref(void)
{
}
