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
//      (c) Copyright 2018-2022 by Andrettin
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

#include "database/data_type.h"
#include "database/named_data_entry.h"

namespace wyrmgus {

class resource_icon;

class season final : public named_data_entry, public data_type<season>
{
	Q_OBJECT

	Q_PROPERTY(wyrmgus::resource_icon* icon MEMBER icon NOTIFY changed)

public:
	static constexpr const char *class_identifier = "season";
	static constexpr const char property_class_identifier[] = "wyrmgus::season*";
	static constexpr const char *database_folder = "seasons";

	explicit season(const std::string &identifier) : named_data_entry(identifier)
	{
	}

	virtual void check() const override
	{
		if (this->get_icon() == nullptr) {
			throw std::runtime_error("Season \"" + this->get_identifier() + "\" has no icon.");
		}
	}

	const resource_icon *get_icon() const
	{
		return this->icon;
	}

signals:
	void changed();

private:
	resource_icon *icon = nullptr;
};

}