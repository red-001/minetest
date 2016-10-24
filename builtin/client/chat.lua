-- Minetest: builtin/chat_commands.lua
print('test1')
core.chatcommands = {}
function core.register_chatcommand(cmd, def)
	def = def or {}
	def.params = def.params or ""
	def.description = def.description or ""
	def.mod_origin = core.get_current_modname() or "??"
	core.chatcommands[cmd] = def
end

core.register_on_chat_message(function(message)
	if not (message:sub(1,1) == "/") then
		return false
	end
	core.display_chat_message("issued command: "..message)
	local cmd, param = string.match(message, "^/([^ ]+) *(.*)")
	if not param then
		param = ""
	end
	local cmd_def = core.chatcommands[cmd]
	if cmd_def then
		core.set_last_run_mod(cmd_def.mod_origin)
		local success, message = cmd_def.func(param)
		if message then
			core.display_chat_message(message)
		end
	else
		core.send_chat_message(message)
	end
	return true  -- Handled chat message
end)
