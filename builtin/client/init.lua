
local scriptpath = core.get_builtin_path()..DIR_DELIM
local commonpath = scriptpath.."common"..DIR_DELIM
local gamepath = scriptpath.."game"..DIR_DELIM
local clientpath = scriptpath.."client"..DIR_DELIM

dofile(commonpath.."vector.lua")
dofile(commonpath.."run_callbacks.lua")

dofile(gamepath.."constants.lua")
dofile(gamepath.."item.lua")

if core.setting_getbool("profiler.load") then
	profiler = dofile(scriptpath.."profiler"..DIR_DELIM.."init.lua")
end

dofile(gamepath.."detached_inventory.lua")
dofile(gamepath.."features.lua")
dofile(gamepath.."voxelarea.lua")

profiler = nil

--
-- Callback registration
--
core.registered_on_chat_messages = {}

function core.register_on_chat_message(func)
	local t = core.registered_on_chat_messages
	t[#t + 1] = func
	core.callback_origins[func] = {
		mod = core.get_current_modname() or "??",
		name = debug.getinfo(1, "n").name or "??"
	}
end

dofile(clientpath.."chat.lua")