
local scriptpath = core.get_builtin_path()
local commonpath = scriptpath .. "common" .. DIR_DELIM
local gamepath   = scriptpath .. "game".. DIR_DELIM
local tgamepath   = scriptpath .. "game_trusted".. DIR_DELIM

-- Shared between builtin files, but
-- not exposed to outer context
local builtin_shared = {}

 -- restrict calling back into the untrusted state to just builtin
 -- the Trusted API is designed to only support untrusted -> secure calls
 -- but for convenience we want to be able to call a minimal number of
 -- untrusted functions as an implementation detail.
 --
 -- Need at least to support the auth API in trusted mode
 -- We don't trust mods with this since it's non-ideal design
 -- and mods *should* be written to not need to do such callbacks
local call_untrusted_internal = core.call_untrusted_internal
core.call_untrusted = nil

function builtin_shared.call_core_untrusted(name, ...)
	call_untrusted_internal("core", ...)
end

function builtin_shared.call_auth_untrusted(name, ...)
	call_untrusted_internal("auth", ...)
end

local function wrap_untrusted(name, max_args, max_return_vals)
	local call_core = builtin_shared.call_core_untrusted
	core[name] = function(...)
		return call_core(name, max_args, max_return_vals, ...)
	end
end

-- we cannot use the C implementation directly here
-- it's wrapped by Lua in builtin to handle persistence
core.forceload_block = nil
core.forceload_free_block = nil

wrap_untrusted("forceload_block", 3, 1)
wrap_untrusted("forceload_free_block", 2, 0)

-- calls callbacks
wrap_untrusted("run_priv_callbacks", 4, 0)

-- disable these since users might think a trusted mapgen or async environment exists
core.register_mapgen_script = nil
core.register_async_dofile = nil
core.do_async_callback = nil

-- call the untrusted implementation due to how the callbacks work
core.dynamic_add_media()
wrap_untrusted("dynamic_add_media", 1, 1)

dofile(gamepath .. "constants.lua")
assert(loadfile(commonpath .. "item_s.lua"))(builtin_shared)
assert(loadfile(gamepath .. "item.lua"))(builtin_shared)
assert(loadfile(commonpath .. "register.lua"))(builtin_shared)
assert(loadfile(tgamepath .. "register.lua"))(builtin_shared)

if core.settings:get_bool("profiler.load") then
	profiler = dofile(scriptpath .. "profiler" .. DIR_DELIM .. "init.lua")
end

dofile(commonpath .. "after.lua")
dofile(commonpath .. "metatable.lua")
dofile(commonpath .. "mod_storage.lua")
dofile(gamepath .. "item_entity_spawn.lua")
dofile(gamepath .. "misc_s.lua")
dofile(gamepath .. "misc.lua")
dofile(gamepath .. "privileges.lua")
dofile(gamepath .. "chat_formatter.lua")
dofile(gamepath .. "features.lua")
dofile(gamepath .. "voxelarea.lua")

core.after(0, builtin_shared.cache_content_ids)

profiler = nil
