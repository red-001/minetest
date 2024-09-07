// Minetest
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "lua_api/l_base.h"

class ModApiTrusted : public ModApiBase
{
private:
	// call_trusted(trusted_function_name, ...) -> ...
	static int l_call_trusted(lua_State *L);

public:
	static void Initialize(lua_State *L, int top);
};
