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
/**@name replay.h - The replay header file. */
//
//      (c) Copyright 1998-2007 by Lutz Sammer and Jimmy Salmon
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

enum ReplayType {
	ReplayNone,          /// No replay
	ReplaySinglePlayer,  /// Single player replay
	ReplayMultiPlayer    /// Multi player replay
};                       /// Replay types

class CFile;
class CUnit;

extern bool CommandLogDisabled;    /// True, if command log is off
extern ReplayType ReplayGameType;  /// Replay game type

/// Log commands into file
extern void CommandLog(const char *action, const CUnit *unit, int flush,
					   int x, int y, const CUnit *dest, const char *value, int num);
/// Replay user commands from log each cycle, single player games
extern void SinglePlayerReplayEachCycle();
/// Replay user commands from log each cycle, multiplayer games
extern void MultiPlayerReplayEachCycle();
/// Load replay
extern int LoadReplay(const std::filesystem::path &filepath);
/// End logging
extern void EndReplayLog();
/// Clean replay
extern void CleanReplayLog();
/// Save the replay list to file
extern void SaveReplayList(CFile &file);

[[nodiscard]]
extern boost::asio::awaitable<void> StartReplay(const std::filesystem::path &filepath, const bool reveal);

/// Register ccl functions related to network
extern void ReplayCclRegister();
