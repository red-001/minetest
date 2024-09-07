-- Minetest: builtin/auth.lua

--
-- Builtin authentication handler
--

-- Make the auth object private, deny access to mods
local core_auth = core.auth
core.auth = nil

local gamepath = scriptpath.."game".. DIR_DELIM
assert(loadfile(gamepath.."auth_default.lua"))(core_auth)

core.register_on_prejoinplayer(function(name, ip)
	if core.registered_auth_handler ~= nil then
		return -- Don't do anything if custom auth handler registered
	end
	local auth_entry = core_auth.read(name)
	if auth_entry ~= nil then
		return
	end

	local name_lower = name:lower()
	for k in core.builtin_auth_handler.iterate() do
		if k:lower() == name_lower then
			return string.format("\nCannot create new player called '%s'. "..
					"Another account called '%s' is already registered. "..
					"Please check the spelling if it's your account "..
					"or use a different nickname.", name, k)
		end
	end
end)

--
-- Authentication API
--

function core.register_authentication_handler(handler)
	if core.registered_auth_handler then
		error("Add-on authentication handler already registered by "..core.registered_auth_handler_modname)
	end
	core.registered_auth_handler = handler
	core.registered_auth_handler_modname = core.get_current_modname()
	handler.mod_origin = core.registered_auth_handler_modname
end

function core.get_auth_handler()
	return core.registered_auth_handler or core.builtin_auth_handler
end

local auth_pass = core_auth.pass

-- undocumented, called by trusted environment
function core.internal_player_auth_iterate()
	local auth_handler = core.get_auth_handler()
	local players_auth = {}
	for k,v in auth_handler.iterate() do
		players_auth[k] = v
	end
	return players_auth
end

core.set_player_password = auth_pass("set_password")
core.set_player_privs    = auth_pass("set_privileges")
core.remove_player_auth  = auth_pass("delete_auth")
core.auth_reload         = auth_pass("reload")


local record_login = auth_pass("record_login")
core.register_on_joinplayer(function(player)
	record_login(player:get_player_name())
end)
