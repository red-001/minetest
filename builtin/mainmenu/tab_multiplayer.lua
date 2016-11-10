--Minetest
--Copyright (C) 2014 sapier
--
--This program is free software; you can redistribute it and/or modify
--it under the terms of the GNU Lesser General Public License as published by
--the Free Software Foundation; either version 2.1 of the License, or
--(at your option) any later version.
--
--This program is distributed in the hope that it will be useful,
--but WITHOUT ANY WARRANTY; without even the implied warranty of
--MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
--GNU Lesser General Public License for more details.
--
--You should have received a copy of the GNU Lesser General Public License along
--with this program; if not, write to the Free Software Foundation, Inc.,
--51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

--------------------------------------------------------------------------------
local function get_formspec(tabview, name, tabdata)
	-- Update the cached supported proto info,
	-- it may have changed after a change by the settings menu.
	common_update_cached_supp_proto()
	local fav_selected = nil
	if menudata.search_result then
		fav_selected = menudata.search_result[tabdata.fav_selected]
	else
		fav_selected = menudata.favorites[tabdata.fav_selected]
	end
	
	if not tabdata.search_for then 
		tabdata.search_for = ""
	end

	local retval =
		"label[7.75,-0.15;" .. fgettext("Address / Port") .. "]" ..
		"label[7.75,1.05;" .. fgettext("Name / Password") .. "]" ..
		"field[8,0.75;3.3,0.5;te_address;;" ..
			core.formspec_escape(core.setting_get("address")) .. "]" ..
		"field[11.15,0.75;1.4,0.5;te_port;;" ..
			core.formspec_escape(core.setting_get("remote_port")) .. "]" ..
		"button[10.1,4.9;2,0.5;btn_mp_connect;" .. fgettext("Connect") .. "]" ..
		"field[8,1.95;2.95,0.5;te_name;;" ..
			core.formspec_escape(core.setting_get("name")) .. "]" ..
		"pwdfield[10.78,1.95;1.77,0.5;te_pwd;]" ..
		"box[7.73,2.35;4.3,2.28;#999999]"..
		"field[0.15,0.25;5,0.27;te_search;;"..core.formspec_escape(tabdata.search_for).."]"..
		"button[4.8,0;3,0.1;btn_mp_search;" .. fgettext("Search") .. "]"
		
	if tabdata.fav_selected and fav_selected then
		if gamedata.fav then
			retval = retval .. "button[7.85,4.9;2.3,0.5;btn_delete_favorite;" ..
				fgettext("Del. Favorite") .. "]"
		end
		if fav_selected.description then
			retval = retval .. "textarea[8.1,2.4;4.26,2.6;;" ..
				core.formspec_escape((gamedata.serverdescription or ""), true) .. ";]"
		end
	end

	--favourites
	retval = retval .. "tablecolumns[" ..
		image_column(fgettext("Favorite"), "favorite") .. ";" ..
		"color,span=3;" ..
		"text,align=right;" ..                -- clients
		"text,align=center,padding=0.25;" ..  -- "/"
		"text,align=right,padding=0.25;" ..   -- clients_max
		image_column(fgettext("Creative mode"), "creative") .. ",padding=1;" ..
		image_column(fgettext("Damage enabled"), "damage") .. ",padding=0.25;" ..
		image_column(fgettext("PvP enabled"), "pvp") .. ",padding=0.25;" ..
		"color,span=1;" ..
		"text,padding=1]" ..
		"table[-0.15,0.4;7.75,5.35;favourites;"
		
	if menudata.search_result then
		for i = 1, #menudata.search_result do
			local favs = core.get_favorites("local")
			local server = menudata.search_result[i]
			
			for fav_id = 1, #favs do
				if server.address == favs[fav_id].address and
						server.port == favs[fav_id].port then
					server.is_favorite = true
				end
			end
			
			if i ~= 1 then
				retval = retval .. ","
			end
			
			retval = retval .. render_serverlist_row(server, server.is_favorite)
		end
	elseif #menudata.favorites > 0 then
		local favs = core.get_favorites("local")
		if #favs > 0 then
			for i = 1, #favs do
			for j = 1, #menudata.favorites do
				if menudata.favorites[j].address == favs[i].address and
						menudata.favorites[j].port == favs[i].port then
					table.insert(menudata.favorites, i, table.remove(menudata.favorites, j))
				end
			end
				if favs[i].address ~= menudata.favorites[i].address then
					table.insert(menudata.favorites, i, favs[i])
				end
			end
		end
		retval = retval .. render_serverlist_row(menudata.favorites[1], (#favs > 0))
		for i = 2, #menudata.favorites do
			retval = retval .. "," .. render_serverlist_row(menudata.favorites[i], (i <= #favs))
		end
	end

	if tabdata.fav_selected then
		retval = retval .. ";" .. tabdata.fav_selected .. "]"
	else
		retval = retval .. ";0]"
	end

	return retval
end

local function check_filters(filters, server)
		return  (filters.pvp == nil or filters.pvp == server.pvp) and
				(filters.creative == nil or filters.creative == server.creative) and
				(filters.version  == nil or filters.version  == server.version) and
				(filters.damage   == nil or filters.damage   == server.damage)
end

--------------------------------------------------------------------------------
local function main_button_handler(tabview, fields, name, tabdata)
	local serverlist = menudata.search_result or menudata.favorites
	
	if fields.te_name then
		gamedata.playername = fields.te_name
		core.setting_set("name", fields.te_name)
	end

	if fields.favourites then
		local event = core.explode_table_event(fields.favourites)
		local fav = serverlist[event.row]

		if event.type == "DCL" then
			if event.row <= #serverlist then
				if menudata.favorites_is_public and
						not is_server_protocol_compat_or_error(
							fav.proto_min, fav.proto_max) then
					return true
				end

				gamedata.address    = fav.address
				gamedata.port       = fav.port
				gamedata.playername = fields.te_name
				gamedata.selected_world = 0

				if fields.te_pwd then
					gamedata.password = fields.te_pwd
				end

				gamedata.servername        = fav.name
				gamedata.serverdescription = fav.description

				if gamedata.address and gamedata.port then
					core.setting_set("address", gamedata.address)
					core.setting_set("remote_port", gamedata.port)
					core.start()
				end
			end
			return true
		end

		if event.type == "CHG" then
			if event.row <= #serverlist then
				gamedata.fav = false
				local favs = core.get_favorites("local")
				local address = fav.address
				local port    = fav.port
				gamedata.serverdescription = fav.description

				for i = 1, #favs do
					if fav.address == favs[i].address and
							fav.port == favs[i].port then
						gamedata.fav = true
					end
				end

				if address and port then
					core.setting_set("address", address)
					core.setting_set("remote_port", port)
				end
				tabdata.fav_selected = event.row
			end
			return true
		end
	end

	if fields.key_up or fields.key_down then
		local fav_idx = core.get_table_index("favourites")
		local fav = serverlist[fav_idx]

		if fav_idx then
			if fields.key_up and fav_idx > 1 then
				fav_idx = fav_idx - 1
			elseif fields.key_down and fav_idx < #menudata.favorites then
				fav_idx = fav_idx + 1
			end
		else
			fav_idx = 1
		end

		if not menudata.favorites or not fav then
			tabdata.fav_selected = 0
			return true
		end

		local address = fav.address
		local port    = fav.port
		gamedata.serverdescription = fav.description
		if address and port then
			core.setting_set("address", address)
			core.setting_set("remote_port", port)
		end

		tabdata.fav_selected = fav_idx
		return true
	end

	if fields.btn_delete_favorite then
		local current_favourite = core.get_table_index("favourites")
		if not current_favourite then return end

		core.delete_favorite(current_favourite)
		asyncOnlineFavourites()
		tabdata.fav_selected = nil

		core.setting_set("address", "")
		core.setting_set("remote_port", "30000")
		return true
	end

	if fields.btn_mp_search or fields.key_enter_field == "te_search" then
		tabdata.fav_selected = 1
		local input = fields.te_search:lower()
		tabdata.search_for = fields.te_search
		
		if input == "" then 
			menudata.search_result = nil
			return true 
		end
		
		if #menudata.favorites < 2 then 
			return true 
		end
		
		menudata.search_result = {}
		
		-- setup the filter and keyword list
		local filters = {}
		local keywords = {}
		for word in input:gmatch("%S+") do
			local index = word:find(":")
			if index then
				local filter = word:sub(1,index-1)
				local setting = word:sub(index+1,-1)

				if filter == "pvp" then
					filters.pvp = core.is_yes(setting)
				elseif filter == "creative" then
					filters.creative = core.is_yes(setting)
				elseif filter == "version" then
					if setting ~= "" then
						filters.version = setting
					end
				elseif filter == "damage" then
					filters.damage = core.is_yes(setting)
				else
					table.insert(keywords, word)
				end
				
			else
				table.insert(keywords, word)
			end
		end
		
		if #keywords < 1 then
			local search_result = {}
			for i = 1, #menudata.favorites do
				local server = menudata.favorites[i]
				if check_filters(filters, server) then
					table.insert(search_result, server)
				end
			end
			menudata.search_result = search_result
			if #search_result > 0 then
				local first_server = search_result[1]
				core.setting_set("address",     first_server.address)
				core.setting_set("remote_port", first_server.port)
			end
			return true
		end
					
		-- Search the serverlist
		local search_result = {}
		for i = 1, #menudata.favorites do
			local server = menudata.favorites[i]
			if check_filters(filters, server) then
				local found = 0
				
				for k = 1, #keywords do
					local keyword = keywords[k]
					if server.name then
						local name = server.name:lower()
						local _, count = name:gsub(keyword, keyword)
						found = found + count * 2
					end
			
					if server.description then
						local desc = server.description:lower()
						local _, count = desc:gsub(keyword, keyword)
						found = found + count 
					end
				end
				if found > 0 then
					local points = (#menudata.favorites - i) / 5 + found
					server.points = points
					table.insert(search_result, server)
				end
			end
		end
		if #search_result > 0 then
			-- Order the results by relevance
			
			local function compare(a,b)
				return a.points > b.points
			end
			table.sort(search_result,compare)
			menudata.search_result = search_result
			local first_server = search_result[1]
			core.setting_set("address",     first_server.address)
			core.setting_set("remote_port", first_server.port)
		end
		return true
	end

	if (fields.btn_mp_connect or fields.key_enter) and fields.te_address and fields.te_port then
		gamedata.playername = fields.te_name
		gamedata.password   = fields.te_pwd
		gamedata.address    = fields.te_address
		gamedata.port       = fields.te_port
		gamedata.selected_world = 0
		local fav_idx = core.get_table_index("favourites")
		local fav = serverlist[fav_idx]

		if fav_idx and fav_idx <= #serverlist and
				fav.address == fields.te_address and
				fav.port    == fields.te_port then

			gamedata.servername        = fav.name
			gamedata.serverdescription = fav.description

			if menudata.favorites_is_public and
					not is_server_protocol_compat_or_error(
						fav.proto_min, fav.proto_max) then
				return true
			end
		else
			gamedata.servername        = ""
			gamedata.serverdescription = ""
		end

		core.setting_set("address",     fields.te_address)
		core.setting_set("remote_port", fields.te_port)

		core.start()
		return true
	end
	return false
end

local function on_change(type, old_tab, new_tab)
	if type == "LEAVE" then return end
	asyncOnlineFavourites()
end

--------------------------------------------------------------------------------
return {
	name = "multiplayer",
	caption = fgettext("Client"),
	cbf_formspec = get_formspec,
	cbf_button_handler = main_button_handler,
	on_change = on_change
}
