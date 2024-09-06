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

	int args_start = lua_gettop(L);

	for (PackedValue &value : arguments)
		script_unpack(L, &value, /*safe=*/true);

	int res = lua_pcall(L, static_cast<int>(arguments.size()), LUA_MULTRET, error_handler);

	if (res)
		scriptError(res, __func__);


	int args_end = lua_gettop(L);
	return_values.reserve(args_end - args_start);

	for (int i = args_start; i <= args_end; i++)
		return_values.push_back(script_pack(L, i, /*safe=*/true));
}

void ScriptApiTrusted::step(float dtime)
{
	SCRIPTAPI_PRECHECKHEADER

	lua_getglobal(L, "core");
	lua_getfield(L, -1, "registered_globalsteps");

	lua_pushnumber(L, dtime);
	runCallbacks(1, RUN_CALLBACKS_MODE_FIRST);
}
