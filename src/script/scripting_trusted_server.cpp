// Minetest
// SPDX-License-Identifier: LGPL-2.1-or-later


#include "scripting_trusted_server.h"
#include "server.h"
#include "log.h"
#include "settings.h"
#include "filesys.h"
#include "cpp_api/s_internal.h"
#include "lua_api/l_areastore.h"
#include "lua_api/l_auth.h"
#include "lua_api/l_base.h"
#include "lua_api/l_craft.h"
#include "lua_api/l_env.h"
#include "lua_api/l_inventory.h"
#include "lua_api/l_item.h"
#include "lua_api/l_itemstackmeta.h"
#include "lua_api/l_mapgen.h"
#include "lua_api/l_modchannels.h"
#include "lua_api/l_nodemeta.h"
#include "lua_api/l_nodetimer.h"
#include "lua_api/l_noise.h"
#include "lua_api/l_object.h"
#include "lua_api/l_playermeta.h"
#include "lua_api/l_particles.h"
#include "lua_api/l_rollback.h"
#include "lua_api/l_server.h"
#include "lua_api/l_util.h"
#include "lua_api/l_vmanip.h"
#include "lua_api/l_settings.h"
#include "lua_api/l_http.h"
#include "lua_api/l_storage.h"

extern "C" {
#include <lualib.h>
}

ServerTrustedScripting::ServerTrustedScripting(Server* server):
		ScriptApiBase(ScriptingType::TrustedServer)
{
	setGameDef(server);
	
	SCRIPTAPI_PRECHECKHEADER
	
	lua_getglobal(L, "core");
	int top = lua_gettop(L);

	// Initialize our lua_api modules
	InitializeModApi(L, top);
	lua_pop(L, 1);

	// Push builtin initialization type
	lua_pushstring(L, "trusted_game");
	lua_setglobal(L, "INIT");

	infostream << "SCRIPTAPI: Initialized trusted game modules" << std::endl;
}

void ServerTrustedScripting::loadBuiltin()
{
	loadMod(Server::getBuiltinLuaPath() + DIR_DELIM "init.lua", BUILTIN_MOD_NAME);
	checkSetByBuiltin();
}

void ServerTrustedScripting::InitializeModApi(lua_State *L, int top)
{
	// Register reference classes (userdata)
	InvRef::Register(L);
	ItemStackMetaRef::Register(L);
	LuaAreaStore::Register(L);
	LuaItemStack::Register(L);
	LuaPerlinNoise::Register(L);
	LuaPerlinNoiseMap::Register(L);
	LuaPseudoRandom::Register(L);
	LuaPcgRandom::Register(L);
	LuaRaycast::Register(L);
	LuaSecureRandom::Register(L);
	LuaVoxelManip::Register(L);
	NodeMetaRef::Register(L);
	NodeTimerRef::Register(L);
	ObjectRef::Register(L);
	PlayerMetaRef::Register(L);
	LuaSettings::Register(L);
	StorageRef::Register(L);

	// Initialize mod api modules
	ModApiAuth::Initialize(L, top);
	ModApiCraft::InitializeAsync(L, top);
	ModApiEnv::Initialize(L, top);
	ModApiInventory::InitializeTrusted(L, top);
	ModApiItem::InitializeAsync(L, top);
	ModApiMapgen::InitializeEmerge(L, top);
	ModApiParticles::Initialize(L, top);
	ModApiRollback::Initialize(L, top);
	ModApiServer::Initialize(L, top);
	ModApiUtil::Initialize(L, top);
	ModApiHttp::Initialize(L, top);
	ModApiStorage::Initialize(L, top);
}
