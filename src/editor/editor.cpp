//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
//      (c) Copyright 2002-2022 by Lutz Sammer, Jimmy Salmon and Andrettin
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.

/**
**  @page EditorModule Module - Editor
**
**  This is a very simple editor for the Stratagus engine.
**
**  @section Missing Missing features
**
**    @li Edit upgrade section
**    @li Edit allow section
**    @li Edit .cm files
**    @li Upgraded unit-types should be shown different on map
**    @li Good keyboard bindings
**    @li Script support
**    @li Commandline support
**    @li Cut&Paste
**    @li More random map functions.
*/

#include "stratagus.h"

#include "editor.h"

#include "game/game.h"
#include "map/map.h"
#include "map/map_info.h"
#include "player/player.h"
#include "util/event_loop.h"
#include "util/path_util.h"
#include "video/video.h"

CEditor::CEditor() : SelectedPlayer(PlayerNumNeutral), State(EditorStateType::EditorSelecting)
{
}

/**
**  Start the editor
**
**  @param filename  Map to load, null to create a new map
*/
boost::asio::awaitable<void> CEditor::start(const std::filesystem::path &filepath)
{
	std::string nc, rc;

	if (!filepath.empty()) {
		CurrentMapPath = filepath;
	} else {
		// new map, choose some default values
		CurrentMapPath.clear();
		// Map.Info.Description.clear();
		// Map.Info.MapWidth = 64;
		// Map.Info.MapHeight = 64;
	}

	//Wyrmgus start
	CleanPlayers(); //clean players, as they could not have been cleansed after a scenario
	//Wyrmgus end

	// Run the editor.
	co_await EditorMainLoop();

	// Clear screen
	Video.ClearScreen();

	CEditor::get()->TerrainEditable = true;

	CEditor::get()->ShownTileTypes.clear();
	CleanGame();
	CleanPlayers();
}

boost::asio::awaitable<void> CEditor::start_new(const std::string &name, const QSize &map_size)
{
	auto map_info = make_qunique<wyrmgus::map_info>();
	map_info->set_name(name);
	map_info->set_map_size(map_size);
	map_info->set_presentation_filepath("new_map");

	CMap::get()->set_info(std::move(map_info));

	co_await this->start("");
}

void CEditor::start_async(const QString &filepath)
{
	event_loop::get()->co_spawn([this, filepath]() -> boost::asio::awaitable<void> {
		co_await this->start(path::from_qstring(filepath));
	});
}

void CEditor::start_new_async(const QString &name, const int map_width, const int map_height)
{
	event_loop::get()->co_spawn([this, name, map_width, map_height]() -> boost::asio::awaitable<void> {
		co_await this->start_new(name.toStdString(), QSize(map_width, map_height));
	});
}

void CEditor::set_running_async(const bool running)
{
	event_loop::get()->post([this, running]() {
		this->set_running(running);
	});
}
