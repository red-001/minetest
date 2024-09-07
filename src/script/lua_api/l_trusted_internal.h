// Minetest
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "lua_api/l_base.h"

class ModApiTrustedHelpersInternal : public ModApiBase
{
private:
	// call_untrusted_internal(trusted_function_name, ...) -> ...
	static int l_call_untrusted_internal(lua_State* L);

public:
	static void Initialize(lua_State* L, int top);
};
