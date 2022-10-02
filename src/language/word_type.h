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
//      (c) Copyright 2015-2022 by Andrettin
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

#include "util/enum_converter.h"

namespace wyrmgus {

enum class word_type {
	none = -1,
	noun,
	verb,
	adjective,
	pronoun,
	adverb,
	conjunction,
	adposition,
	article,
	numeral,
	affix
};

extern template class enum_converter<word_type>;

inline std::string word_type_to_name(const word_type type)
{
	switch (type) {
		case word_type::none:
			return "None";
		case word_type::noun:
			return "Noun";
		case word_type::verb:
			return "Verb";
		case word_type::adjective:
			return "Adjective";
		case word_type::pronoun:
			return "Pronoun";
		case word_type::adverb:
			return "Adverb";
		case word_type::conjunction:
			return "Conjunction";
		case word_type::adposition:
			return "Adposition";
		case word_type::article:
			return "Article";
		case word_type::numeral:
			return "Numeral";
		case word_type::affix:
			return "Affix";
		default:
			break;
	}

	throw std::runtime_error("Invalid word type: \"" + std::to_string(static_cast<int>(type)) + "\".");
}

}

Q_DECLARE_METATYPE(wyrmgus::word_type)
