-- Minetest: builtin/trusted_game/auth.lua

--
-- Builtin authentication handler and wrapper
--

local builtin_shared = ...
local call_auth_untrusted = builtin_shared.call_auth_untrusted
local call_core_untrusted = builtin_shared.call_core_untrusted

-- Make the auth object private, deny access to mods
local core_auth = core.auth
core.auth = nil

local gamepath = scriptpath.."game".. DIR_DELIM
assert(loadfile(gamepath.."auth_default.lua"))(core_auth)

--
-- Authentication API wrapper
--

local auth_handler_wrapper = {}

local function auth_forward_to_insecure(name, arg_count, expected_return_type)
	auth_handler_wrapper[name] = function(...)
		local ret_val = call_auth_untrusted(name, arg_count, 1, ...)
		
		local actual_return_type = type(ret_val)
		-- filter return type
		if actual_return_type ~= "nil" and actual_return_type ~= "boolean"
			and actual_return_type ~= expected_return_type then
			return false
		
		return ret_val
	end
end

auth_forward_to_insecure("get_auth", 1, "table")
auth_forward_to_insecure("create_auth", 2, "table")
auth_forward_to_insecure("delete_auth", 1, "boolean")
auth_forward_to_insecure("set_privileges", 2, "nil")
auth_forward_to_insecure("reload", 0, "boolean")
auth_forward_to_insecure("record_login", 1, "nil")

auth_handler_wrapper.iterate = function()
	local players_auth = call_core_untrusted("internal_player_auth_iterate", 0, 1)
	if type(players_auth) ~= "table" then
		players_auth = {}
	end
	return pairs(players_auth)
end

function core.get_auth_handler()
	return auth_handler_wrapper
end

core.set_player_password = core_auth.pass("set_password")
core.set_player_privs    = core_auth.pass("set_privileges")
core.remove_player_auth  = core_auth.pass("delete_auth")
core.auth_reload         = core_auth.pass("reload")
