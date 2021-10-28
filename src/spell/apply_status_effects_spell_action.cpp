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
//      (c) Copyright 1998-2021 by Vladi Belperchinov-Shabanski, Lutz Sammer,
//                                 Jimmy Salmon, Joris Dauphin and Andrettin
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

#include "stratagus.h"

#include "spell/apply_status_effects_spell_action.h"

//Wyrmgus start
#include "commands.h"
//Wyrmgus end
#include "spell/status_effect.h"
#include "unit/unit.h"
#include "util/string_conversion_util.h"
#include "util/string_util.h"

namespace wyrmgus {

void apply_status_effects_spell_action::process_sml_property(const sml_property &property)
{
	const std::string &key = property.get_key();
	const std::string &value = property.get_value();

	const status_effect status_effect = string_to_status_effect(key);

	this->status_effect_cycles[status_effect] = std::stoi(value);
}

int apply_status_effects_spell_action::Cast(CUnit &caster, const spell &, CUnit *target, const Vec2i &goal_pos, int z, int modifier)
{
	Q_UNUSED(caster)
	Q_UNUSED(goal_pos)
	Q_UNUSED(z)

	if (target == nullptr) {
		return 1;
	}

	for (const auto &[status_effect, cycles] : this->status_effect_cycles) {
		if (cycles == 0) {
			//remove status effect
			target->remove_status_effect(status_effect);
			continue;
		}

		target->apply_status_effect(status_effect, cycles * modifier / 100);

		if (status_effect == status_effect::stun) {
			//if the target has become stunned, stop it
			CommandStopUnit(*target);
		}
	}

	return 1;
}

}
