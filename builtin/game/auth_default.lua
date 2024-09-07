local core_auth = ...

core.builtin_auth_handler = {
	get_auth = function(name)
		assert(type(name) == "string")
		local auth_entry = core_auth.read(name)
		-- If no such auth found, return nil
		if not auth_entry then
			return nil
		end
		-- Figure out what privileges the player should have.
		-- Take a copy of the privilege table
		local privileges = {}
		for priv, _ in pairs(auth_entry.privileges) do
			privileges[priv] = true
		end
		-- If singleplayer, give all privileges except those marked as give_to_singleplayer = false
		if core.is_singleplayer() then
			for priv, def in pairs(core.registered_privileges) do
				if def.give_to_singleplayer then
					privileges[priv] = true
				end
			end
		-- For the admin, give everything
		elseif name == core.settings:get("name") then
			for priv, def in pairs(core.registered_privileges) do
				if def.give_to_admin then
					privileges[priv] = true
				end
			end
		end
		-- All done
		return {
			password = auth_entry.password,
			privileges = privileges,
			last_login = auth_entry.last_login,
		}
	end,
	create_auth = function(name, password)
		assert(type(name) == "string")
		assert(type(password) == "string")
		core.log('info', "Built-in authentication handler adding player '"..name.."'")
		return core_auth.create({
			name = name,
			password = password,
			privileges = core.string_to_privs(core.settings:get("default_privs")),
			last_login = -1,  -- Defer login time calculation until record_login (called by on_joinplayer)
		})
	end,
	delete_auth = function(name)
		assert(type(name) == "string")
		local auth_entry = core_auth.read(name)
		if not auth_entry then
			return false
		end
		core.log('info', "Built-in authentication handler deleting player '"..name.."'")
		return core_auth.delete(name)
	end,
	set_password = function(name, password)
		assert(type(name) == "string")
		assert(type(password) == "string")
		local auth_entry = core_auth.read(name)
		if not auth_entry then
			core.builtin_auth_handler.create_auth(name, password)
		else
			core.log('info', "Built-in authentication handler setting password of player '"..name.."'")
			auth_entry.password = password
			core_auth.save(auth_entry)
		end
		return true
	end,
	set_privileges = function(name, privileges)
		assert(type(name) == "string")
		assert(type(privileges) == "table")
		local auth_entry = core_auth.read(name)
		if not auth_entry then
			auth_entry = core.builtin_auth_handler.create_auth(name,
				core.get_password_hash(name,
					core.settings:get("default_password")))
		end

		local prev_privs = auth_entry.privileges
		auth_entry.privileges = privileges

		core_auth.save(auth_entry)

		for priv, value in pairs(privileges) do
			-- Warnings for improper API usage
			if value == false then
				core.log('deprecated', "`false` value given to `minetest.set_player_privs`, "..
						"this is almost certainly a bug, "..
						"granting a privilege rather than revoking it")
			elseif value ~= true then
				core.log('deprecated', "non-`true` value given to `minetest.set_player_privs`")
			end
			-- Run grant callbacks
			if prev_privs[priv] == nil then
				core.run_priv_callbacks(name, priv, nil, "grant")
			end
		end

		-- Run revoke callbacks
		for priv, _ in pairs(prev_privs) do
			if privileges[priv] == nil then
				core.run_priv_callbacks(name, priv, nil, "revoke")
			end
		end
		core.notify_authentication_modified(name)
	end,
	reload = function()
		core_auth.reload()
		return true
	end,
	record_login = function(name)
		assert(type(name) == "string")
		local auth_entry = core_auth.read(name)
		assert(auth_entry)
		auth_entry.last_login = os.time()
		core_auth.save(auth_entry)
	end,
	iterate = function()
		local names = {}
		local nameslist = core_auth.list_names()
		for k,v in pairs(nameslist) do
			names[v] = true
		end
		return pairs(names)
	end,
}

function core.change_player_privs(name, changes)
	local privs = core.get_player_privs(name)
	for priv, change in pairs(changes) do
		if change == true then
			privs[priv] = true
		elseif change == false then
			privs[priv] = nil
		else
			error("non-bool value given to `minetest.change_player_privs`")
		end
	end
	core.set_player_privs(name, privs)
end

function core_auth.pass(name)
	return function(...)
		local auth_handler = core.get_auth_handler()
		if auth_handler[name] then
			return auth_handler[name](...)
		end
		return false
	end
end