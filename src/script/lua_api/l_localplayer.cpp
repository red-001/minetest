/*
Minetest
Copyright (C) 2013 celeron55, Perttu Ahola <celeron55@gmail.com>

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

#include "lua_api/l_localplayer.h"
#include "lua_api/l_internal.h"
//#include "lua_api/l_inventory.h"
//#include "lua_api/l_item.h"
#include "common/c_converter.h"
#include "common/c_content.h"
#include "log.h"
//#include "tool.h"
#include "clientobject.h"
#include "content_cao.h"
//#include "server.h"
//#include "hud.h"
//#include "serverscripting.h"


/*
	LocalObjectRef
*/


LocalObjectRef* LocalObjectRef::checkobject(lua_State *L, int narg)
{
	luaL_checktype(L, narg, LUA_TUSERDATA);
	void *ud = luaL_checkudata(L, narg, className);
	if (!ud) luaL_typerror(L, narg, className);
	return *(LocalObjectRef**)ud;  // unbox pointer
}

ClientActiveObject* LocalObjectRef::getobject(LocalObjectRef *ref)
{
	ClientActiveObject *co = ref->m_object;
	return co;
}

GenericCAO* LocalObjectRef::getgenericcao(LocalObjectRef *ref)
{
	ClientActiveObject *obj = getobject(ref);
	/*
	if (obj == NULL)
		return NULL;
	if (obj->getType() != ACTIVEOBJECT_TYPE_PLAYER)
		return NULL;
	*/
	return (GenericCAO*)obj;
}
/*
LocalPlayer *LocalObjectRef::getplayer(LocalObjectRef *ref)
{
	GenericCAO *cao = getplayercao(ref);
	if (cao == NULL)
		return NULL;
	return cao->getPlayer();
}
*/
// Exported functions

// garbage collector
int LocalObjectRef::gc_object(lua_State *L) {
	LocalObjectRef *o = *(LocalObjectRef **)(lua_touserdata(L, 1));
	infostream<<"LocalObjectRef::gc_object: o="<<o<<std::endl;
	delete o;
	return 0;
}

// get_pos(self)
// returns: {x=num, y=num, z=num}
int LocalObjectRef::l_get_pos(lua_State *L)
{
	LocalObjectRef *ref = checkobject(L, 1);
	ClientActiveObject *co = getobject(ref);
	if (co == NULL) return 0;
	v3f pos = co->getPosition();
	lua_newtable(L);
	lua_pushnumber(L, pos.X);
	lua_setfield(L, -2, "x");
	lua_pushnumber(L, pos.Y);
	lua_setfield(L, -2, "y");
	lua_pushnumber(L, pos.Z);
	lua_setfield(L, -2, "z");
	return 1;
}


LocalObjectRef::LocalObjectRef(ClientActiveObject *object):
	m_object(object)
{
	infostream<<"LocalObjectRef created for id="<<m_object->getId()<<std::endl;
}

LocalObjectRef::~LocalObjectRef()
{
	if (m_object)
		infostream<<"LocalObjectRef destructing for id="
				<<m_object->getId()<<std::endl;
	else
		infostream<<"LocalObjectRef destructing for id=unknown"<<std::endl;
}

// Creates an LocalObjectRef and leaves it on top of stack
// Not callable from Lua; all references are created on the C side.
void LocalObjectRef::create(lua_State *L, ClientActiveObject *object)
{
	LocalObjectRef *o = new LocalObjectRef(object);
	//infostream<<"LocalObjectRef::create: o="<<o<<std::endl;
	*(void **)(lua_newuserdata(L, sizeof(void *))) = o;
	luaL_getmetatable(L, className);
	lua_setmetatable(L, -2);
}

void LocalObjectRef::set_null(lua_State *L)
{
	LocalObjectRef *o = checkobject(L, -1);
	o->m_object = NULL;
}

void LocalObjectRef::Register(lua_State *L)
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

	// Cannot be created from Lua
	//lua_register(L, className, create_object);
}

const char LocalObjectRef::className[] = "LocalObjectRef";
const luaL_reg LocalObjectRef::methods[] = {
	// ClientActiveObject
	luamethod(LocalObjectRef, get_pos),
	{0,0}
};
