/*
Minetest
Copyright (C) 2017 Loic Blot <loic.blot@unix-experience.fr>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "lua_api/l_camera.h"
#include "lua_api/l_internal.h"
#include "common/c_converter.h"
#include "camera.h"

void LuaCamera::create(lua_State *L)
{
	LuaCamera *o = new LuaCamera();
	*(void **)(lua_newuserdata(L, sizeof(void *))) = o;
	luaL_getmetatable(L, className);
	lua_setmetatable(L, -2);

	// Keep camera object stack id
	int camera_object = lua_gettop(L);

	lua_getglobal(L, "core");
	lua_getfield(L, -1, "ui");
	luaL_checktype(L, -1, LUA_TTABLE);
	int uitable = lua_gettop(L);

	lua_pushvalue(L, camera_object); // Copy object to top of stack
	lua_setfield(L, uitable, "camera");
}

int LuaCamera::l_get_pos(lua_State *L)
{
	//LuaMinimap *ref = checkobject(L, 1);
	//Minimap *m = getobject(ref);

	//push_v3s16(L, m->getPos());
	lua_pushinteger(L, true);
	return 1;
}

int LuaCamera::l_get_mode(lua_State *L)
{
	LuaCamera *ref = checkobject(L, 1);
	Camera *c = getobject(ref, L);

	CameraMode mode = c->getCameraMode();
	switch(mode) {
		case CAMERA_MODE_FIRST:
			lua_pushstring(L, "first");
			break;
		case CAMERA_MODE_THIRD:
			lua_pushstring(L, "third");
			break;
		case CAMERA_MODE_THIRD_FRONT:
			lua_pushstring(L, "third_front");
			break;
	}

	return 1;
}

int LuaCamera::l_set_mode(lua_State *L)
{
	LuaCamera *ref = checkobject(L, 1);
	Camera *c = getobject(ref, L);

	CameraMode mode;
	std::string set_to = luaL_checkstring(L, 2);
	if(set_to == "first")
		mode = CAMERA_MODE_FIRST;
	else if(set_to == "third")
		mode = CAMERA_MODE_THIRD;
	else if(set_to == "third_front")
		mode = CAMERA_MODE_THIRD_FRONT;
	else {
		errorstream << "Set_mode: Invalid arguments passed by Lua." << std::endl;
		lua_pushboolean(L, false);
		return 1;
	}

	c->setCameraMode(mode);
	lua_pushboolean(L, true);
	return 1;
}

int LuaCamera::l_toggle_mode(lua_State *L)
{
	LuaCamera *ref = checkobject(L, 1);
	Camera *c = getobject(ref, L);
	c->toggleCameraMode();
	return 0;
}

LuaCamera *LuaCamera::checkobject(lua_State *L, int narg)
{
	luaL_checktype(L, narg, LUA_TUSERDATA);

	void *ud = luaL_checkudata(L, narg, className);
	if (!ud)
		luaL_typerror(L, narg, className);

	return *(LuaCamera **)ud;  // unbox pointer
}

Camera* LuaCamera::getobject(LuaCamera *ref, lua_State *L)
{
	Camera *obj = ref->m_camera;
	if(!obj) {
		obj = getClient(L)->getCamera();
		ref->setobject(obj);
	}
	return obj;
}

int LuaCamera::gc_object(lua_State *L) {
	LuaCamera *o = *(LuaCamera **)(lua_touserdata(L, 1));
	delete o;
	return 0;
}

void LuaCamera::Register(lua_State *L)
{
	lua_newtable(L);
	int methodtable = lua_gettop(L);
	luaL_newmetatable(L, className);
	int metatable = lua_gettop(L);

	lua_pushliteral(L, "__metatable");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);  // hide metatable from Lua getmetatable()

	lua_pushliteral(L, "__index");
	lua_pushvalue(L, methodtable);
	lua_settable(L, metatable);

	lua_pushliteral(L, "__gc");
	lua_pushcfunction(L, gc_object);
	lua_settable(L, metatable);

	lua_pop(L, 1);  // drop metatable

	luaL_openlib(L, 0, methods, 0);  // fill methodtable
	lua_pop(L, 1);  // drop methodtable
}

const char LuaCamera::className[] = "Camera";
const luaL_reg LuaCamera::methods[] = {
	luamethod(LuaCamera, get_pos),
	luamethod(LuaCamera, get_mode),
	luamethod(LuaCamera, set_mode),
	luamethod(LuaCamera, toggle_mode),
	{0,0}
};
