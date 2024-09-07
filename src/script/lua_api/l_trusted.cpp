// Minetest
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lua_api/l_trusted.h"
#include "lua_api/l_internal.h"
#include "common/c_converter.h"
#include "cpp_api/s_trusted.h"

// call_trusted(trusted_function_name, ...) -> ...
int ModApiTrusted::l_call_trusted(lua_State *L)
{
	NO_MAP_LOCK_REQUIRED;

	ScriptApiTrusted* trusted_api = getTrustedAPI(L);
	if (!trusted_api)
		throw LuaError("Trusted API not found, internal Minetest error!");

	auto function_name = readParam<std::string_view>(L, 1);

	std::vector<PackedValue> arguments;
	std::vector<PackedValue> return_values;
	arguments.reserve(lua_gettop(L) - 1);
	for (int i = 2; i < lua_gettop(L); i++)
		arguments.push_back(script_pack(L, i, /*safe=*/true));

	trusted_api->runTrustedFunction(function_name, std::move(arguments), return_values);

	for (PackedValue& value : return_values)
		script_unpack(L, &value, /*safe=*/true);

	return return_values.size();
}

void ModApiTrusted::Initialize(lua_State *L, int top)
{
	API_FCT(call_trusted);
}
