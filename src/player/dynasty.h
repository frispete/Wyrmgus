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
//      (c) Copyright 2020-2022 by Andrettin
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
#include "database/detailed_data_entry.h"

class CPlayer;
class CUpgrade;

namespace wyrmgus {

class character;
class faction;
class icon;

template <typename scope_type>
class and_condition;

class dynasty final : public detailed_data_entry, public data_type<dynasty>
{
	Q_OBJECT

	Q_PROPERTY(CUpgrade* upgrade READ get_upgrade WRITE set_upgrade)
	Q_PROPERTY(wyrmgus::icon* icon MEMBER icon NOTIFY changed)
	Q_PROPERTY(std::vector<wyrmgus::faction *> factions READ get_factions)
	Q_PROPERTY(QVariantList dynastic_tree_characters READ get_dynastic_tree_characters NOTIFY changed)

public:
	static constexpr const char *class_identifier = "dynasty";
	static constexpr const char property_class_identifier[] = "wyrmgus::dynasty*";
	static constexpr const char *database_folder = "dynasties";

	static dynasty *add(const std::string &identifier, const wyrmgus::data_module *data_module)
	{
		dynasty *dynasty = data_type::add(identifier, data_module);
		dynasty->index = dynasty::get_all().size() - 1;
		return dynasty;
	}

	explicit dynasty(const std::string &identifier);
	~dynasty();

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void check() const override;

	virtual std::string get_encyclopedia_text() const override;

	int get_index() const
	{
		return this->index;
	}

	CUpgrade *get_upgrade() const
	{
		return this->upgrade;
	}

	void set_upgrade(CUpgrade *upgrade);

	wyrmgus::icon *get_icon() const
	{
		return this->icon;
	}

	const std::vector<faction *> &get_factions() const
	{
		return this->factions;
	}

	Q_INVOKABLE void add_faction(faction *faction);
	Q_INVOKABLE void remove_faction(faction *faction);

	const std::unique_ptr<and_condition<CPlayer>> &get_preconditions() const
	{
		return this->preconditions;
	}

	const std::unique_ptr<and_condition<CPlayer>> &get_conditions() const
	{
		return this->conditions;
	}

	const std::vector<const character *> &get_characters() const
	{
		return this->characters;
	}

	QVariantList get_dynastic_tree_characters() const;

	void add_character(const character *character)
	{
		this->characters.push_back(character);
	}

signals:
	void changed();

private:
	int index = -1;
	CUpgrade *upgrade = nullptr; //dynasty upgrade applied when the dynasty is set
	wyrmgus::icon *icon = nullptr;
	std::vector<faction *> factions; //to which factions is this dynasty available
	std::unique_ptr<and_condition<CPlayer>> preconditions;
	std::unique_ptr<and_condition<CPlayer>> conditions;
	std::vector<const character *> characters;
};

}
