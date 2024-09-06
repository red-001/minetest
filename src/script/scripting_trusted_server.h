// Minetest
// SPDX-License-Identifier: LGPL-2.1-or-later


#pragma once
#include "cpp_api/s_base.h"
#include "cpp_api/s_modchannels.h"
#include "cpp_api/s_trusted.h"

/*****************************************************************************/
/* Scripting <-> Server Game Interface                                       */
/*****************************************************************************/

class Environment;

class ServerTrustedScripting:
		virtual public ScriptApiBase,
		public ScriptApiTrusted
{
public:
	ServerTrustedScripting(Server* server);

	void loadBuiltin();
	void setEnv(Environment* env)
	{
		ScriptApiBase::setEnv(env);
	}


private:
	void InitializeModApi(lua_State *L, int top);
};
