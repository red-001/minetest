/*
Minetest
Copyright (C) 2017 nerzhul, Loic Blot <loic.blot@unix-experience.fr>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <cassert>
#include <json/json.h>
#include "database-files.h"
#include "content_sao.h"
#include "remoteplayer.h"
#include "settings.h"
#include "porting.h"
#include "filesys.h"
#include "util/auth.h"
#include "util/base64.h"
#include "filesys.h"

// !!! WARNING !!!
// This backend is intended to be used on Minetest 0.4.16 only for the transition backend
// for player files

void PlayerDatabaseFiles::serialize(std::ostringstream &os, RemotePlayer *player)
{
	// Utilize a Settings object for storing values
	Settings args;
	args.setS32("version", 1);
	args.set("name", player->getName());

	sanity_check(player->getPlayerSAO());
	args.setS32("hp", player->getPlayerSAO()->getHP());
	args.setV3F("position", player->getPlayerSAO()->getBasePosition());
	args.setFloat("pitch", player->getPlayerSAO()->getPitch());
	args.setFloat("yaw", player->getPlayerSAO()->getYaw());
	args.setS32("breath", player->getPlayerSAO()->getBreath());

	std::string extended_attrs;
	player->serializeExtraAttributes(extended_attrs);
	args.set("extended_attributes", extended_attrs);

	args.writeLines(os);

	os << "PlayerArgsEnd\n";

	player->inventory.serialize(os);
}

void PlayerDatabaseFiles::savePlayer(RemotePlayer *player)
{
	std::string savedir = m_savedir + DIR_DELIM;
	std::string path = savedir + player->getName();
	bool path_found = false;
	RemotePlayer testplayer("", NULL);

	for (u32 i = 0; i < PLAYER_FILE_ALTERNATE_TRIES && !path_found; i++) {
		if (!fs::PathExists(path)) {
			path_found = true;
			continue;
		}

		// Open and deserialize file to check player name
		std::ifstream is(path.c_str(), std::ios_base::binary);
		if (!is.good()) {
			errorstream << "Failed to open " << path << std::endl;
			return;
		}

		testplayer.deSerialize(is, path, NULL);
		is.close();
		if (strcmp(testplayer.getName(), player->getName()) == 0) {
			path_found = true;
			continue;
		}

		path = savedir + player->getName() + itos(i);
	}

	if (!path_found) {
		errorstream << "Didn't find free file for player " << player->getName()
				<< std::endl;
		return;
	}

	// Open and serialize file
	std::ostringstream ss(std::ios_base::binary);
	serialize(ss, player);
	if (!fs::safeWriteToFile(path, ss.str())) {
		infostream << "Failed to write " << path << std::endl;
	}
	player->setModified(false);
}

bool PlayerDatabaseFiles::removePlayer(const std::string &name)
{
	std::string players_path = m_savedir + DIR_DELIM;
	std::string path = players_path + name;

	RemotePlayer temp_player("", NULL);
	for (u32 i = 0; i < PLAYER_FILE_ALTERNATE_TRIES; i++) {
		// Open file and deserialize
		std::ifstream is(path.c_str(), std::ios_base::binary);
		if (!is.good())
			continue;

		temp_player.deSerialize(is, path, NULL);
		is.close();

		if (temp_player.getName() == name) {
			fs::DeleteSingleFileOrEmptyDirectory(path);
			return true;
		}

		path = players_path + name + itos(i);
	}

	return false;
}

bool PlayerDatabaseFiles::loadPlayer(RemotePlayer *player, PlayerSAO *sao)
{
	std::string players_path = m_savedir + DIR_DELIM;
	std::string path = players_path + player->getName();

	const std::string player_to_load = player->getName();
	for (u32 i = 0; i < PLAYER_FILE_ALTERNATE_TRIES; i++) {
		// Open file and deserialize
		std::ifstream is(path.c_str(), std::ios_base::binary);
		if (!is.good())
			continue;

		player->deSerialize(is, path, sao);
		is.close();

		if (player->getName() == player_to_load)
			return true;

		path = players_path + player_to_load + itos(i);
	}

	infostream << "Player file for player " << player_to_load << " not found" << std::endl;
	return false;
}

void PlayerDatabaseFiles::listPlayers(std::vector<std::string> &res)
{
	std::vector<fs::DirListNode> files = fs::GetDirListing(m_savedir);
	// list files into players directory
	for (std::vector<fs::DirListNode>::const_iterator it = files.begin(); it !=
		files.end(); ++it) {
		// Ignore directories
		if (it->dir)
			continue;

		const std::string &filename = it->name;
		std::string full_path = m_savedir + DIR_DELIM + filename;
		std::ifstream is(full_path.c_str(), std::ios_base::binary);
		if (!is.good())
			continue;

		RemotePlayer player(filename.c_str(), NULL);
		// Null env & dummy peer_id
		PlayerSAO playerSAO(NULL, &player, 15789, false);

		player.deSerialize(is, "", &playerSAO);
		is.close();

		res.emplace_back(player.getName());
	}
}

AuthDatabaseFiles::AuthDatabaseFiles(const std::string &world_path) :
	m_savedir(world_path + DIR_DELIM + "auth.txt")
{
	loadDatabase();
}

AuthData* AuthDatabaseFiles::getPlayerAuth(const std::string &player_name)
{
	auto it = m_auth_data.find(player_name);
	if (it == m_auth_data.end()) {
		return nullptr;
	}
	return &it->second;
}

// stub for now
void AuthDatabaseFiles::createPlayerAuth(const std::string &name, const std::string &hash, const std::string &salt)
{
	AuthData new_player_auth;
	new_player_auth.user_last_login = time(0);
	new_player_auth.is_srp =  true;
	new_player_auth.password_hash = base64_encode(hash);
	new_player_auth.password_salt = base64_encode(salt);
	m_auth_data[name] = new_player_auth;
	saveDatabase();
}

bool AuthDatabaseFiles::playerAuthUpdated(const std::string &player_name)
{
	saveDatabase();
	return true;
}

void AuthDatabaseFiles::record_login(const std::string player_name)
{
	AuthData *player_auth = getPlayerAuth(player_name);
	if (player_auth)
		player_auth->user_last_login = time(0);
	saveDatabase();
}
void AuthDatabaseFiles::reload()
{
	loadDatabase();
}

bool AuthDatabaseFiles::removePlayer(const std::string &player_name)
{
	auto it = m_auth_data.find(player_name);
	if (it == m_auth_data.end()) {
		return false;
	}
	m_auth_data.erase(it);
	saveDatabase();
	return true;
}

std::unordered_set<std::string> AuthDatabaseFiles::listPlayers()
{
	std::unordered_set<std::string> player_list;
	for (auto const &it : m_auth_data)
		player_list.insert(it.first);
	return player_list;
}

void AuthDatabaseFiles::loadDatabase()
{
	std::ifstream auth_file(m_savedir);
	if (auth_file.is_open()) {
		std::string line;
		while (getline(auth_file, line)) {
			std::vector<std::string> components = str_split(line, ':');
			if (components.size() > 3) {
				AuthData user_auth;
				std::string name = components[0];
				std::string login_data = components[1];
				std::string privs_string = components[2];
				if (components.size() == 4)
					user_auth.user_last_login = std::stol(components[3]);
				user_auth.player_privs = str_split_set(privs_string, ',');
				if (split_srp_verifier_and_salt(login_data, &user_auth.password_hash,
						&user_auth.password_salt)) {
					user_auth.is_srp = true;
				} else if (!login_data.empty() && base64_is_valid(login_data)) {
					user_auth.is_srp = false;
					user_auth.password_hash = login_data;
				} else {
					errorstream << "Skipping loading Auth for user: '" << name << "', invalid password data!" << std::endl;
					continue;
				}
				m_auth_data[name] = user_auth;
			} else {
				errorstream << "Skipping invalid line in Auth file" << std::endl;
			}
		}
		auth_file.close();
	} else {
		errorstream << "Unable to open Auth file" << std::endl;
	}
}

void AuthDatabaseFiles::saveDatabase()
{
	std::stringstream data;
	for (auto const &it : m_auth_data) {
		AuthData player_auth = it.second;
		std::string privs = str_concat_set(player_auth.player_privs, ',');
		std::string login_data;
		if (player_auth.is_srp)
			login_data = encode_srp_verifier(base64_decode(player_auth.password_hash),
					base64_decode(player_auth.password_salt));
		else
			login_data = player_auth.password_hash;
		data << it.first << ":" << login_data << ":"
				<< privs << ":"
				<< player_auth.user_last_login << std::endl;
	}
	fs::safeWriteToFile(m_savedir, data.str());
}
