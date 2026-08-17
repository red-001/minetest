// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>
// Copyright (C) 2017 numzero, Lobachevskiy Vitaliy <numzer0@yandex.ru>

#pragma once

#include "irr_v2d.h"
#include <SColor.h>
#include <memory>

class IrrlichtDevice;
class ShadowRenderer;
class Client;
class Hud;

/// Abstract interface of a rendering core. Draws the game frame.
class RenderingCore
{
public:
	virtual ~RenderingCore() = default;

	/// Draw one frame into the active window.
	virtual void draw(video::SColor _skycolor, bool _show_hud,
			bool _draw_wield_tool, bool _draw_crosshair) = 0;

	virtual v2u32 getVirtualSize() const = 0;

	virtual ShadowRenderer *get_shadow_renderer() { return nullptr; };
};