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
//      (c) Copyright 2022 by Andrettin
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

#pragma once

namespace wyrmgus {

enum class trigger_target {
	player, //checked for each player
	neutral_player, //checked for the neutral player
	player_or_neutral_player //checked for each player, including the neutral one
};

inline trigger_target string_to_trigger_target(const std::string &str)
{
	if (str == "player") {
		return trigger_target::player;
	} else if (str == "neutral_player") {
		return trigger_target::neutral_player;
	} else if (str == "player_or_neutral_player") {
		return trigger_target::player_or_neutral_player;
	}

	throw std::runtime_error("Invalid trigger target: \"" + str + "\".");
}

}

Q_DECLARE_METATYPE(wyrmgus::trigger_target)
