// Minetest
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "cpp_api/s_trusted.h"
#include "cpp_api/s_internal.h"
#include "common/c_converter.h"

void ScriptApiTrusted::runTrustedFunction(std::string_view function, std::vector<PackedValue> arguments, std::vector<PackedValue>& return_values)
{
	SCRIPTAPI_PRECHECKHEADER
	SANITY_CHECK(arguments.size() <= std::numeric_limits<int>::max());

	int error_handler = PUSH_ERROR_HANDLER(L);

	// Get trusted functions table
	lua_getglobal(L, "core");
	lua_getfield(L, -1, "trusted_functions");
	lua_remove(L, -2); // Remove core

	// get the trustlet
	lua_getfield(L, -1, function.data());
	lua_remove(L, -2); // Remove trusted_functions

	if (!lua_isfunction(L, -1))
		throw LuaError("trusted function not found!");

	callPacked(arguments, return_values, error_handler);
}

void ScriptApiTrusted::step(float dtime)
{
	SCRIPTAPI_PRECHECKHEADER

	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_globalsteps");

	lua_pushnumber(L, dtime);
	runCallbacks(1, RUN_CALLBACKS_MODE_FIRST);
}
