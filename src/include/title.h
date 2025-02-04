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
/**@name title.h - The title screen headerfile. */
//
//      (c) Copyright 2007 by Jimmy Salmon
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
	class font;
	class renderer;
}

enum {
	TitleFlagCenter = 1 << 0   /// Center Text
};

class TitleScreenLabel final
{
public:
	std::string Text;
	wyrmgus::font *Font = nullptr;
	int Xofs = 0;
	int Yofs = 0;
	int Flags = 0;
};

class TitleScreen final
{
public:
	[[nodiscard]]
	boost::asio::awaitable<void> ShowTitleImage(std::vector<std::function<void(renderer *)>> &render_commands) const;

private:
	void ShowLabels(std::vector<std::function<void(renderer *)>> &render_commands) const;

public:
	std::string File;
	bool StretchImage = true;
	int Timeout = 0;
	int Iterations = 0;
	int Editor = 0;
	std::vector<TitleScreenLabel> Labels;
};

extern std::vector<TitleScreen> TitleScreens;          /// File for title screen

[[nodiscard]]
extern boost::asio::awaitable<void> ShowTitleScreens(std::vector<std::function<void(renderer *)>> &render_commands);
