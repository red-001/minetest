-- Minetest: builtin/trusted_game/register.lua

local builtin_shared = ...

local make_registration = builtin_shared.make_registration
local make_registration_reverse = builtin_shared.make_registration_reverse

core.registered_globalsteps, core.register_globalstep = make_registration()
core.registered_on_mods_loaded, core.register_on_mods_loaded = make_registration()
core.registered_on_shutdown, core.register_on_shutdown = make_registration()
