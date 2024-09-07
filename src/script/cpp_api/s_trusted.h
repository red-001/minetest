// Minetest
// SPDX-License-Identifier: LGPL-2.1-or-later

#pragma once

#include "common/c_packer.h"
#include "cpp_api/s_base.h"
#include "util/string.h"
#include "gui/guiMainMenu.h"

class ScriptApiTrusted : virtual public ScriptApiBase {
public:
	void runTrustedFunction(std::string_view function, std::vector<PackedValue> arguments, std::vector<PackedValue> &return_values);
	void step(float dtime);
	void onModsLoaded();
	void onShutdown();
};
