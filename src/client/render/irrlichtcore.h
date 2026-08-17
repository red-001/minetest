// Luanti
// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright (C) 2010-2013 celeron55, Perttu Ahola <celeron55@gmail.com>
// Copyright (C) 2017 numzero, Lobachevskiy Vitaliy <numzer0@yandex.ru>

#pragma once

#include "core.h"
#include "irr_v2d.h"

class ShadowRenderer;
class Client;
class Hud;
class RenderPipeline;

/// Rendering core that uses the built-in Irrlicht renderer.
class IrrlichtRenderingCore : public RenderingCore
{
	IrrlichtDevice *device;
	Client *client;
	Hud *hud;
	std::unique_ptr<ShadowRenderer> shadow_renderer;

	std::unique_ptr<RenderPipeline> pipeline;

	v2f virtual_size_scale;
	v2u32 virtual_size { 0, 0 };

public:
	IrrlichtRenderingCore(IrrlichtDevice *device, Client *client, Hud *hud,
			std::unique_ptr<ShadowRenderer> shadow_renderer,
			std::unique_ptr<RenderPipeline> pipeline,
			v2f virtual_size_scale);
	IrrlichtRenderingCore(const IrrlichtRenderingCore &) = delete;
	IrrlichtRenderingCore(IrrlichtRenderingCore &&) = delete;
	~IrrlichtRenderingCore() override;

	IrrlichtRenderingCore &operator=(const IrrlichtRenderingCore &) = delete;
	IrrlichtRenderingCore &operator=(IrrlichtRenderingCore &&) = delete;

	void draw(video::SColor _skycolor, bool _show_hud,
			bool _draw_wield_tool, bool _draw_crosshair) override;

	v2u32 getVirtualSize() const override;

	ShadowRenderer *get_shadow_renderer() override { return shadow_renderer.get(); };
};