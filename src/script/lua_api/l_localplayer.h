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

#ifndef L_OBJECT_H_
#define L_OBJECT_H_

#include "lua_api/l_base.h"
#include "irrlichttypes.h"

class ClientActiveObject;
class GenericCAO;
class LocalPlayer;

/*
	LocalObjectRef
*/

class LocalObjectRef : public ModApiBase {
private:
	ClientActiveObject *m_object;

	static const char className[];
	static const luaL_reg methods[];
public:
	static LocalObjectRef *checkobject(lua_State *L, int narg);

	static ClientActiveObject* getobject(LocalObjectRef *ref);
private:
	static GenericCAO* getgenericcao(LocalObjectRef *ref);

	static LocalPlayer *getplayer(LocalObjectRef *ref);

	// Exported functions

	// garbage collector
	static int gc_object(lua_State *L);

	// get_pos(self)
	// returns: {x=num, y=num, z=num}
	static int l_get_pos(lua_State *L);



public:
	LocalObjectRef(ClientActiveObject *object);

	~LocalObjectRef();

	// Creates an LocalObjectRef and leaves it on top of stack
	// Not callable from Lua; all references are created on the C side.
	static void create(lua_State *L, ClientActiveObject *object);

	static void set_null(lua_State *L);

	static void Register(lua_State *L);
};

#endif /* L_OBJECT_H_ */
