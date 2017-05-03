-- Minetest: builtin/client/chatcommands.lua

local function rgb_to_hex(rgb)
	local hexadecimal = '#'

	for key, value in pairs(rgb) do
		local hex = ''

		while(value > 0)do
			local index = math.fmod(value, 16) + 1
			value = math.floor(value / 16)
			hex = string.sub('0123456789ABCDEF', index, index) .. hex			
		end

		if(string.len(hex) == 0)then
			hex = '00'

		elseif(string.len(hex) == 1)then
			hex = '0' .. hex
		end

		hexadecimal = hexadecimal .. hex
	end

	return hexadecimal
end

local function color_from_hue(hue)
	local h = hue / 60
	local c = 255
	local x = (1 - math.abs(h%2 - 1)) * 255

  	local i = math.floor(h);
  	if (i == 0) then
		return rgb_to_hex({c, x, 0})
  	elseif (i == 1) then 
		return rgb_to_hex({x, c, 0})
  	elseif (i == 2) then 
		return rgb_to_hex({0, c, x})
	elseif (i == 3) then
		return rgb_to_hex({0, x, c});
	elseif (i == 4) then
		return rgb_to_hex({x, 0, c});
	else 
		return rgb_to_hex({c, 0, x});
	end
end

--[[
function rgb_to_hex(red, green, blue)
{
  var h = ((red << 16) | (green << 8) | (blue)).toString(16);
  // add the beginning zeros
  while (h.length < 6) h = '0' + h;
  return '#' + h;
}
--]]
core.register_on_sending_chat_messages(function(message)
	if message:sub(1,2) == ".." then
		return false
	end

	local first_char = message:sub(1,1)
	if first_char == "/" or first_char == "." then
		core.display_chat_message(core.gettext("issued command: ") .. message)
	end

	if first_char ~= "." then
		return false
	end

	local cmd, param = string.match(message, "^%.([^ ]+) *(.*)")
 	param = param or ""

	if not cmd then
		core.display_chat_message(core.gettext("-!- Empty command"))
		return true
	end

	local cmd_def = core.registered_chatcommands[cmd]
	if cmd_def then
		core.set_last_run_mod(cmd_def.mod_origin)
		local _, message = cmd_def.func(param)
		if message then
			core.display_chat_message(message)
		end
	else
		core.display_chat_message(core.gettext("-!- Invalid command: ") .. cmd)
	end

	return true
end)

core.register_chatcommand("list_players", {
	description = core.gettext("List online players"),
	func = function(param)
		local players = table.concat(core.get_player_names(), ", ")
		core.display_chat_message(core.gettext("Online players: ") .. players)
	end
})

core.register_chatcommand("disconnect", {
	description = core.gettext("Exit to main menu"),
	func = function(param)
		core.disconnect()
	end,
})

core.register_chatcommand("set_colour", {
	description = core.gettext("Change chat colour"),
	func = function(param)
		core.set_setting("chat_colour", param)
	end,
})

core.register_chatcommand("rainbow", {
	description = core.gettext("rainbow text"),
	func = function(param)
	if (param:len() == 0) then
		return false, "string too short"
	end
	step = 360 / param:len()
 	local hue = 0
      -- iterate the whole 360 degrees
	local output = ""
      	for i=1, param:len() do
        	output = output  .. core.get_color_escape_sequence(color_from_hue(hue)) ..  param:sub(i,i)
        	hue = hue + step
	end
	core.send_message(output)
end,
})

