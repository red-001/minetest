// Minetest
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "lua_api/l_trusted_internal.h"
#include "lua_api/l_internal.h"
#include "common/c_converter.h"
#include "cpp_api/s_trusted.h"
#include "server.h"
#include "scripting_server.h"
#include <algorithm>

// call_untrusted_internal(table_name, func_name, max_args, max_return_values, ...) -> ...
int ModApiTrustedHelpersInternal::l_call_untrusted_internal(lua_State* L) {
	NO_MAP_LOCK_REQUIRED;

	Server* server = getServer(L);
	SANITY_CHECK(server);

	ServerScripting* untrusted_scripting = server->getScriptIface();
	if (!untrusted_scripting)
		throw LuaError("No server scripting found, called too early or too late");

	const auto table_name = readParam<std::string_view>(L, 1);
	const auto function_name = readParam<std::string_view>(L, 2);
	const int max_args = luaL_checkinteger(L, 3);
	const int max_return_values = luaL_checkinteger(L, 4);

	const int argument_count = std::min(lua_gettop(L) - 5, max_args);

	std::vector<PackedValue> arguments;
	std::vector<PackedValue> return_values;
	arguments.reserve(argument_count);
	for (int i = 5; i < argument_count; i++)
		arguments.push_back(script_pack(L, i, /*safe=*/true));

	untrusted_scripting->callInternalFunction(table_name, function_name, arguments, return_values);

	const int return_value_count = std::min(static_cast<int>(return_values.size()), max_return_values);

	for (int i = 0; i < return_value_count; i++)
		script_unpack(L, &return_values[i], /*safe=*/true);

	return return_value_count;
}

void ModApiTrustedHelpersInternal::Initialize(lua_State* L, int top)
{
	API_FCT(call_untrusted_internal);
}
