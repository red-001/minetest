/*
Minetest
Copyright (C) 2017 Loic Blot <loic.blot@unix-experience.fr>
Copyright (C) 2017 red-001 <red-001@outlook.ie>

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

#ifndef L_CAMERA_H_
#define L_CAMERA_H_

#include "l_base.h"

class Camera;

class LuaCamera : public ModApiBase {
private:

	static const char className[];
	static const luaL_reg methods[];

	// garbage collector
	static int gc_object(lua_State *L);

	static int l_get_pos(lua_State *L);
	static int l_get_mode(lua_State *L);
	static int l_set_mode(lua_State *L);
	static int l_toggle_mode(lua_State *L);

	Camera *m_camera;
public:
	LuaCamera() {
		m_camera = NULL;
	}
	~LuaCamera() {}

	static void create(lua_State *L);

	static LuaCamera *checkobject(lua_State *L, int narg);
	static Camera* getobject(LuaCamera *ref, lua_State *L);

	void setobject(Camera *obj)
	{
		m_camera = obj;
	}

	static void Register(lua_State *L);
};

#endif // L_CAMERA_H_
