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
//      (c) Copyright 2020-2021 by Andrettin
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

#include "civilization.h"
#include "script/condition/condition.h"

namespace wyrmgus {

class civilization_condition final : public condition
{
public:
	explicit civilization_condition(const std::string &value)
	{
		this->civilization = civilization::get(value);
	}

	virtual bool check(const civilization *civilization) const override
	{
		return civilization == this->civilization;
	}

	virtual bool check(const CPlayer *player, const bool ignore_units) const override
	{
		Q_UNUSED(ignore_units)

		return this->check(player->get_civilization());
	}

	virtual std::string get_string(const size_t indent) const override
	{
		Q_UNUSED(indent)

		return this->civilization->get_name() + " Civilization";
	}

private:
	const wyrmgus::civilization *civilization = nullptr;
};

}
