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
/**@name player.cpp - The players. */
//
//      (c) Copyright 1998-2015 by Lutz Sammer, Jimmy Salmon, Nehal Mistry
//		and Andrettin
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
//

//@{

/*----------------------------------------------------------------------------
-- Includes
----------------------------------------------------------------------------*/

#include <stdarg.h>

#include "stratagus.h"

#include "player.h"

#include "action/action_upgradeto.h"
#include "actions.h"
#include "ai.h"
//Wyrmgus start
#include "commands.h" //for faction setting
//Wyrmgus end
#include "iolib.h"
#include "map.h"
#include "network.h"
#include "netconnect.h"
#include "sound.h"
#include "translate.h"
#include "unitsound.h"
#include "unittype.h"
#include "unit.h"
//Wyrmgus start
#include "upgrade.h"
//Wyrmgus end
#include "ui.h"
#include "video.h"

/*----------------------------------------------------------------------------
--  Documentation
----------------------------------------------------------------------------*/

/**
**  @class CPlayer player.h
**
**  \#include "player.h"
**
**  This structure contains all information about a player in game.
**
**  The player structure members:
**
**  CPlayer::Player
**
**    This is the unique slot number. It is not possible that two
**    players have the same slot number at the same time. The slot
**    numbers are reused in the future. This means if a player is
**    defeated, a new player can join using this slot. Currently
**    #PlayerMax (16) players are supported. This member is used to
**    access bit fields.
**    Slot #PlayerNumNeutral (15) is reserved for the neutral units
**    like gold-mines or critters.
**
**    @note Should call this member Slot?
**
**  CPlayer::Name
**
**    Name of the player used for displays and network game.
**    It is restricted to 15 characters plus final zero.
**
**  CPlayer::Type
**
**    Type of the player. This field is setup from the level (map).
**    We support currently #PlayerNeutral,
**    #PlayerNobody, #PlayerComputer, #PlayerPerson,
**    #PlayerRescuePassive and #PlayerRescueActive.
**    @see #PlayerTypes.
**
**  CPlayer::RaceName
**
**    Name of the race to which the player belongs, used to select
**    the user interface and the AI.
**    We have 'orc', 'human', 'alliance' or 'mythical'. Should
**    only be used during configuration and not during runtime.
**
**  CPlayer::Race
**
**    Race number of the player. This field is setup from the level
**    map. This number is mapped with #PlayerRaces to the symbolic
**    name CPlayer::RaceName.
**
**  CPlayer::AiName
**
**    AI name for computer. This field is setup
**    from the map. Used to select the AI for the computer
**    player.
**
**  CPlayer::Team
**
**    Team of player. Selected during network game setup. All players
**    of the same team are allied and enemy to all other teams.
**    @note It is planned to show the team on the map.
**
**  CPlayer::Enemy
**
**    A bit field which contains the enemies of this player.
**    If CPlayer::Enemy & (1<<CPlayer::Player) != 0 its an enemy.
**    Setup during startup using the CPlayer::Team, can later be
**    changed with diplomacy. CPlayer::Enemy and CPlayer::Allied
**    are combined, if none bit is set, the player is neutral.
**    @note You can be allied to a player, which sees you as enemy.
**
**  CPlayer::Allied
**
**    A bit field which contains the allies of this player.
**    If CPlayer::Allied & (1<<CPlayer::Player) != 0 its an allied.
**    Setup during startup using the Player:Team, can later be
**    changed with diplomacy. CPlayer::Enemy and CPlayer::Allied
**    are combined, if none bit is set, the player is neutral.
**    @note You can be allied to a player, which sees you as enemy.
**
**  CPlayer::SharedVision
**
**    A bit field which contains shared vision for this player.
**    Shared vision only works when it's activated both ways. Really.
**
**  CPlayer::StartX CPlayer::StartY
**
**    The tile map coordinates of the player start position. 0,0 is
**    the upper left on the map. This members are setup from the
**    map and only important for the game start.
**    Ignored if game starts with level settings. Used to place
**    the initial workers if you play with 1 or 3 workers.
**
**  CPlayer::Resources[::MaxCosts]
**
**    How many resources the player owns. Needed for building
**    units and structures.
**    @see _costs_, TimeCost, GoldCost, WoodCost, OilCost, MaxCosts.
**
**  CPlayer::MaxResources[::MaxCosts]
**
**    How many resources the player can store at the moment.
**
**  CPlayer::Incomes[::MaxCosts]
**
**    Income of the resources, when they are delivered at a store.
**    @see _costs_, TimeCost, GoldCost, WoodCost, OilCost, MaxCosts.
**
**  CPlayer::LastResources[::MaxCosts]
**
**    Keeps track of resources in time (used for calculating
**    CPlayer::Revenue, see below)
**
**  CPlayer::Revenue[::MaxCosts]
**
**    Production of resources per minute (or estimates)
**    Used just as information (statistics) for the player...
**
**  CPlayer::UnitTypesCount[::UnitTypeMax]
**
**    Total count for each different unit type. Used by the AI and
**    for dependencies checks. The addition of all counts should
**    be CPlayer::TotalNumUnits.
**    @note Should not use the maximum number of unit-types here,
**    only the real number of unit-types used.
**
**  CPlayer::AiEnabled
**
**    If the player is controlled by the computer and this flag is
**    true, than the player is handled by the AI on this local
**    computer.
**
**    @note Currently the AI is calculated parallel on all computers
**    in a network play. It is planned to change this.
**
**  CPlayer::Ai
**
**    AI structure pointer. Please look at #PlayerAi for more
**    information.
**
**  CPlayer::Units
**
**    A table of all (CPlayer::TotalNumUnits) units of the player.
**
**  CPlayer::TotalNumUnits
**
**    Total number of units (incl. buildings) in the CPlayer::Units
**    table.
**
**  CPlayer::Demand
**
**    Total unit demand, used to demand limit.
**    A player can only build up to CPlayer::Food units and not more
**    than CPlayer::FoodUnitLimit units.
**
**    @note that CPlayer::NumFoodUnits > CPlayer::Food, when enough
**    farms are destroyed.
**
**  CPlayer::NumBuildings
**
**    Total number buildings, units that don't need food.
**
**  CPlayer::Food
**
**    Number of food available/produced. Player can't train more
**    CPlayer::NumFoodUnits than this.
**    @note that all limits are always checked.
**
**  CPlayer::FoodUnitLimit
**
**    Number of food units allowed. Player can't train more
**    CPlayer::NumFoodUnits than this.
**    @note that all limits are always checked.
**
**  CPlayer::BuildingLimit
**
**    Number of buildings allowed.  Player can't build more
**    CPlayer::NumBuildings than this.
**    @note that all limits are always checked.
**
**  CPlayer::TotalUnitLimit
**
**    Number of total units allowed. Player can't have more
**    CPlayer::NumFoodUnits+CPlayer::NumBuildings=CPlayer::TotalNumUnits
**    this.
**    @note that all limits are always checked.
**
**  CPlayer::Score
**
**    Total number of points. You can get points for killing units,
**    destroying buildings ...
**
**  CPlayer::TotalUnits
**
**    Total number of units made.
**
**  CPlayer::TotalBuildings
**
**    Total number of buildings made.
**
**  CPlayer::TotalResources[::MaxCosts]
**
**    Total number of resources collected.
**    @see _costs_, TimeCost, GoldCost, WoodCost, OilCost, MaxCosts.
**
**  CPlayer::TotalRazings
**
**    Total number of buildings destroyed.
**
**  CPlayer::TotalKills
**
**    Total number of kills.
**
**  CPlayer::Color
**
**    Color of units of this player on the minimap. Index number
**    into the global palette.
**
**  CPlayer::UnitColors
**
**    Unit colors of this player. Contains the hardware dependent
**    pixel values for the player colors (palette index #208-#211).
**    Setup from the global palette.
**    @note Index #208-#211 are various SHADES of the team color
**    (#208 is brightest shade, #211 is darkest shade) .... these
**    numbers are NOT red=#208, blue=#209, etc
**
**  CPlayer::Allow
**
**    Contains which unit-types and upgrades are allowed for the
**    player. Possible values are:
**    @li  'A' -- allowed,
**    @li  'F' -- forbidden,
**    @li  'R' -- acquired, perhaps other values
**    @li  'Q' -- acquired but forbidden (does it make sense?:))
**    @li  'E' -- enabled, allowed by level but currently forbidden
**    @see CAllow
**
**  CPlayer::UpgradeTimers
**
**    Timer for the upgrades. One timer for all possible upgrades.
**    Initial 0 counted up by the upgrade action, until it reaches
**    the upgrade time.
**    @see _upgrade_timers_
**    @note it is planned to combine research for faster upgrades.
*/

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

int NumPlayers;                  /// How many player slots used
CPlayer Players[PlayerMax];       /// All players in play
CPlayer *ThisPlayer;              /// Player on this computer
PlayerRace PlayerRaces;          /// Player races

bool NoRescueCheck;               /// Disable rescue check

/**
**  Colors used for minimap.
*/
//Wyrmgus start
//std::vector<CColor> PlayerColorsRGB[PlayerMax];
//std::vector<IntColor> PlayerColors[PlayerMax];

//std::string PlayerColorNames[PlayerMax];
std::vector<CColor> PlayerColorsRGB[PlayerColorMax];
std::vector<IntColor> PlayerColors[PlayerColorMax];

std::string PlayerColorNames[PlayerColorMax];
//Wyrmgus end

/**
**  Which indexes to replace with player color
*/
int PlayerColorIndexStart;
int PlayerColorIndexCount;

//Wyrmgus start
std::map<std::string, int> CivilizationStringToIndex;
std::map<std::string, int> FactionStringToIndex[MAX_RACES];
//Wyrmgus end

/*----------------------------------------------------------------------------
--  Functions
----------------------------------------------------------------------------*/

/**
**  Clean up the PlayerRaces names.
*/
void PlayerRace::Clean()
{
	//Wyrmgus start
	if (this->Count > 0) { //don't clean the languages if first defining the civilizations
		for (size_t i = 0; i < this->Languages.size(); ++i) {
			for (size_t j = 0; j < this->Languages[i]->LanguageWords.size(); ++j) {
				delete this->Languages[i]->LanguageWords[j];
			}
			this->Languages[i]->LanguageWords.clear();
			
			for (size_t j = 0; j < 2; ++j) {
				this->Languages[i]->NameTranslations[j].clear();
			}
		}
	}
	//Wyrmgus end
	for (unsigned int i = 0; i != this->Count; ++i) {
		this->Name[i].clear();
		this->Display[i].clear();
		this->Visible[i] = false;
		//Wyrmgus start
		for (int j = 0; j < UnitTypeClassMax; ++j) {
			this->CivilizationClassUnitTypes[i][j] = -1;
			this->CivilizationClassUpgrades[i][j] = -1;
		}
		for (int j = 0; j < FactionMax; ++j) {
			for (int k = 0; k < UnitTypeClassMax; ++k) {
				FactionClassUnitTypes[i][j][k] = -1;
				FactionClassUpgrades[i][j][k] = -1;
			}
		}
		this->Playable[i] = false;
		this->Species[i].clear();
		this->DefaultColor[i].clear();
		this->ParentCivilization[i] = -1;
		this->CivilizationLanguage[i] = -1;
		for (unsigned int j = 0; j < FactionMax; ++j) {
			if (this->Factions[i][j]) {
				delete this->Factions[i][j];
			}
		}
		for (unsigned int j = 0; j < PersonalNameMax; ++j) {
			this->PersonalNames[i][j].clear();
			this->PersonalNamePrefixes[i][j].clear();
			this->PersonalNameSuffixes[i][j].clear();
			this->ProvinceNames[i][j].clear();
			this->ProvinceNamePrefixes[i][j].clear();
			this->ProvinceNameSuffixes[i][j].clear();
			this->SettlementNames[i][j].clear();
			this->SettlementNamePrefixes[i][j].clear();
			this->SettlementNameSuffixes[i][j].clear();
		}
		//clear deities
		for (size_t j = 0; j < this->Deities[i].size(); ++j) {
			delete this->Deities[i][j];
		}
		this->Deities[i].clear();
		//Wyrmgus end
	}
	this->Count = 0;
}

int PlayerRace::GetRaceIndexByName(const char *raceName) const
{
	//Wyrmgus start
	/*
	for (unsigned int i = 0; i != this->Count; ++i) {
		if (this->Name[i].compare(raceName) == 0) {
			return i;
		}
	}
	return -1;
	*/
	std::string civilization_name(raceName);
	
	if (civilization_name.empty()) {
		return -1;
	}
	
	return CivilizationStringToIndex[civilization_name];
	//Wyrmgus end
}

//Wyrmgus start
int PlayerRace::GetFactionIndexByName(const int civilization, const std::string faction_name) const
{
	if (faction_name.empty()) {
		return -1;
	}
	
	if (civilization == -1) { //if the civilization is -1, then search all civilizations for this faction
		for (int i = 0; i < MAX_RACES; ++i) {
			int civilization_faction_index = PlayerRaces.GetFactionIndexByName(i, faction_name);
			if (civilization_faction_index != -1) {
				return civilization_faction_index;
			}
		}
		return -1; //return -1 if found nothing
	}
	
	if (FactionStringToIndex[civilization].find(faction_name) != FactionStringToIndex[civilization].end()) {
		return FactionStringToIndex[civilization][faction_name];
	} else {
		return -1;
	}		
}

int PlayerRace::GetDeityIndexByName(const int civilization, std::string deity_name) const
{
	for (size_t i = 0; i < this->Deities[civilization].size(); ++i) {
		if (deity_name == this->Deities[civilization][i]->Name) {
			return i;
		}
	}
	return -1;
}

int PlayerRace::GetLanguageIndexByIdent(std::string language_ident) const
{
	for (size_t i = 0; i < this->Languages.size(); ++i) {
		if (language_ident == this->Languages[i]->Ident) {
			return i;
		}
	}
	return -1;
}

int PlayerRace::GetCivilizationClassUnitType(int civilization, int class_id)
{
	if (civilization == -1 || class_id == -1) {
		return -1;
	}
	
	if (CivilizationClassUnitTypes[civilization][class_id] != -1) {
		return CivilizationClassUnitTypes[civilization][class_id];
	}
	
	if (PlayerRaces.ParentCivilization[civilization] != -1) {
		return GetCivilizationClassUnitType(PlayerRaces.ParentCivilization[civilization], class_id);
	}
	
	return -1;
}

int PlayerRace::GetCivilizationClassUpgrade(int civilization, int class_id)
{
	if (civilization == -1 || class_id == -1) {
		return -1;
	}
	
	if (CivilizationClassUpgrades[civilization][class_id] != -1) {
		return CivilizationClassUpgrades[civilization][class_id];
	}
	
	if (PlayerRaces.ParentCivilization[civilization] != -1) {
		return GetCivilizationClassUpgrade(PlayerRaces.ParentCivilization[civilization], class_id);
	}
	
	return -1;
}

int PlayerRace::GetFactionClassUnitType(int civilization, int faction, int class_id)
{
	if (civilization == -1 || class_id == -1) {
		return -1;
	}
	
	if (faction != -1) {
		if (FactionClassUnitTypes[civilization][faction][class_id] != -1) {
			return FactionClassUnitTypes[civilization][faction][class_id];
		}
		
		if (PlayerRaces.Factions[civilization][faction]->ParentFaction != -1) {
			return GetFactionClassUnitType(civilization, PlayerRaces.Factions[civilization][faction]->ParentFaction, class_id);
		}
	}
	
	return GetCivilizationClassUnitType(civilization, class_id);
}

int PlayerRace::GetFactionClassUpgrade(int civilization, int faction, int class_id)
{
	if (civilization == -1 || class_id == -1) {
		return -1;
	}
	
	if (faction != -1) {
		if (FactionClassUpgrades[civilization][faction][class_id] != -1) {
			return FactionClassUpgrades[civilization][faction][class_id];
		}
		
		if (PlayerRaces.Factions[civilization][faction]->ParentFaction != -1) {
			return GetFactionClassUpgrade(civilization, PlayerRaces.Factions[civilization][faction]->ParentFaction, class_id);
		}
	}
	
	return GetCivilizationClassUpgrade(civilization, class_id);
}

int PlayerRace::GetCivilizationLanguage(int civilization)
{
	if (civilization == -1) {
		return -1;
	}
	
	if (CivilizationLanguage[civilization] != -1) {
		return CivilizationLanguage[civilization];
	}
	
	if (PlayerRaces.ParentCivilization[civilization] != -1) {
		return GetCivilizationLanguage(PlayerRaces.ParentCivilization[civilization]);
	}
	
	return -1;
}

int PlayerRace::GetFactionLanguage(int civilization, int faction)
{
	if (civilization == -1) {
		return -1;
	}
	
	if (faction != -1) {
		if (Factions[civilization][faction]->Language != -1) {
			return Factions[civilization][faction]->Language;
		}
		
		if (PlayerRaces.Factions[civilization][faction]->ParentFaction != -1) {
			return GetFactionLanguage(civilization, PlayerRaces.Factions[civilization][faction]->ParentFaction);
		}
	}
	
	return GetCivilizationLanguage(civilization);
}

/**
**  "Translate" (that is, adapt) a proper name from one culture (civilization) to another.
*/
std::string PlayerRace::TranslateName(std::string name, int language)
{
	std::string new_name;
	
	if (language == -1) {
		return new_name;
	}

	// try to translate the entire name, as a particular translation for it may exist
	if (PlayerRaces.Languages[language]->NameTranslations[0].size() > 0) {
		for (size_t i = 0; i < PlayerRaces.Languages[language]->NameTranslations[0].size(); ++i) {
			std::string name_to_be_translated = TransliterateText(PlayerRaces.Languages[language]->NameTranslations[0][i]);
			if (name_to_be_translated == name) {
				std::string name_translation = TransliterateText(PlayerRaces.Languages[language]->NameTranslations[1][i]);
				new_name = name_translation;
				return new_name;
			}
		}
	}
	
	//if adapting the entire name failed, try to match prefixes and suffixes
	if (PlayerRaces.Languages[language]->NameTranslations[0].size() > 0) {
		for (size_t i = 0; i < PlayerRaces.Languages[language]->NameTranslations[0].size(); ++i) {
			std::string prefix_to_be_translated = TransliterateText(PlayerRaces.Languages[language]->NameTranslations[0][i]);
			if (prefix_to_be_translated == name.substr(0, prefix_to_be_translated.size())) {
				for (size_t j = 0; j < PlayerRaces.Languages[language]->NameTranslations[0].size(); ++j) {
					std::string suffix_to_be_translated = TransliterateText(PlayerRaces.Languages[language]->NameTranslations[0][j]);
					suffix_to_be_translated[0] = tolower(suffix_to_be_translated[0]);
					if (suffix_to_be_translated == name.substr(prefix_to_be_translated.size(), name.size() - prefix_to_be_translated.size())) {
						std::string prefix_translation = TransliterateText(PlayerRaces.Languages[language]->NameTranslations[1][i]);
						std::string suffix_translation = TransliterateText(PlayerRaces.Languages[language]->NameTranslations[1][j]);
						suffix_translation[0] = tolower(suffix_translation[0]);
						if (prefix_translation.substr(prefix_translation.size() - 2, 2) == "gs" && suffix_translation.substr(0, 1) == "g") { //if the last two characters of the prefix are "gs", and the first character of the suffix is "g", then remove the final "s" from the prefix (as in "Königgrätz")
							prefix_translation = FindAndReplaceStringEnding(prefix_translation, "gs", "g");
						}
						if (prefix_translation.substr(prefix_translation.size() - 1, 1) == "s" && suffix_translation.substr(0, 1) == "s") { //if the prefix ends in "s" and the suffix begins in "s" as well, then remove the final "s" from the prefix (as in "Josefstadt", "Kronstadt" and "Leopoldstadt")
							prefix_translation = FindAndReplaceStringEnding(prefix_translation, "s", "");
						}
						new_name = prefix_translation;
						new_name += suffix_translation;
						return new_name;
					}
				}
			}
		}
	}
	
	return new_name;
}

LanguageWord *PlayerRace::GetLanguageWord(const std::string word, int language, int word_type) const
{
	for (size_t i = 0; i < this->Languages[language]->LanguageWords.size(); ++i) {
		if (this->Languages[language]->LanguageWords[i]->Word == word && this->Languages[language]->LanguageWords[i]->Type == word_type) {
			return this->Languages[language]->LanguageWords[i];
		}
	}

	return NULL;
}
//Wyrmgus end

/**
**  Init players.
*/
void InitPlayers()
{
	for (int p = 0; p < PlayerMax; ++p) {
		Players[p].Index = p;
		if (!Players[p].Type) {
			Players[p].Type = PlayerNobody;
		}
		//Wyrmgus start
//		for (int x = 0; x < PlayerColorIndexCount; ++x) {
//			PlayerColors[p][x] = Video.MapRGB(TheScreen->format, PlayerColorsRGB[p][x]);
//		}
		//Wyrmgus end
	}
	//Wyrmgus start
	for (int p = 0; p < PlayerColorMax; ++p) {
		for (int x = 0; x < PlayerColorIndexCount; ++x) {
			PlayerColors[p][x] = Video.MapRGB(TheScreen->format, PlayerColorsRGB[p][x]);
		}
	}
	//Wyrmgus end
}

/**
**  Clean up players.
*/
void CleanPlayers()
{
	ThisPlayer = NULL;
	for (unsigned int i = 0; i < PlayerMax; ++i) {
		Players[i].Clear();
	}
	NumPlayers = 0;
	NoRescueCheck = false;
}

void FreePlayerColors()
{
	for (int i = 0; i < PlayerMax; ++i) {
		Players[i].UnitColors.Colors.clear();
		//Wyrmgus start
//		PlayerColorsRGB[i].clear();
//		PlayerColors[i].clear();
		//Wyrmgus end
	}
	//Wyrmgus start
	for (int i = 0; i < PlayerColorMax; ++i) {
		PlayerColorsRGB[i].clear();
		PlayerColors[i].clear();
	}
	//Wyrmgus end
}

/**
**  Save state of players to file.
**
**  @param file  Output file.
**
**  @note FIXME: Not completely saved.
*/
void SavePlayers(CFile &file)
{
	file.printf("\n--------------------------------------------\n");
	file.printf("--- MODULE: players\n\n");

	//  Dump all players
	for (int i = 0; i < NumPlayers; ++i) {
		Players[i].Save(file);
	}

	file.printf("SetThisPlayer(%d)\n\n", ThisPlayer->Index);
}


void CPlayer::Save(CFile &file) const
{
	const CPlayer &p = *this;
	file.printf("Player(%d,\n", this->Index);
	//Wyrmgus start
	file.printf(" \"race\", \"%s\",", PlayerRaces.Name[p.Race].c_str());
	if (p.Faction != -1) {
		file.printf(" \"faction\", %d,", p.Faction);
	}
	for (int i = 0; i < PlayerColorMax; ++i) {
		if (PlayerColors[i][0] == this->Color) {
			file.printf(" \"color\", %d,", i);
			break;
		}
	}
	//Wyrmgus end
	file.printf("  \"name\", \"%s\",\n", p.Name.c_str());
	file.printf("  \"type\", ");
	switch (p.Type) {
		case PlayerNeutral:       file.printf("\"neutral\",");         break;
		case PlayerNobody:        file.printf("\"nobody\",");          break;
		case PlayerComputer:      file.printf("\"computer\",");        break;
		case PlayerPerson:        file.printf("\"person\",");          break;
		case PlayerRescuePassive: file.printf("\"rescue-passive\","); break;
		case PlayerRescueActive:  file.printf("\"rescue-active\","); break;
		default:                  file.printf("%d,", p.Type); break;
	}
	//Wyrmgus start
//	file.printf(" \"race\", \"%s\",", PlayerRaces.Name[p.Race].c_str());
	//Wyrmgus end
	file.printf(" \"ai-name\", \"%s\",\n", p.AiName.c_str());
	file.printf("  \"team\", %d,", p.Team);

	file.printf(" \"enemy\", \"");
	for (int j = 0; j < PlayerMax; ++j) {
		file.printf("%c", (p.Enemy & (1 << j)) ? 'X' : '_');
	}
	file.printf("\", \"allied\", \"");
	for (int j = 0; j < PlayerMax; ++j) {
		file.printf("%c", (p.Allied & (1 << j)) ? 'X' : '_');
	}
	file.printf("\", \"shared-vision\", \"");
	for (int j = 0; j < PlayerMax; ++j) {
		file.printf("%c", (p.SharedVision & (1 << j)) ? 'X' : '_');
	}
	file.printf("\",\n  \"start\", {%d, %d},\n", p.StartPos.x, p.StartPos.y);

	// Resources
	file.printf("  \"resources\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		file.printf("\"%s\", %d, ", DefaultResourceNames[j].c_str(), p.Resources[j]);
	}
	// Stored Resources
	file.printf("},\n  \"stored-resources\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		file.printf("\"%s\", %d, ", DefaultResourceNames[j].c_str(), p.StoredResources[j]);
	}
	// Max Resources
	file.printf("},\n  \"max-resources\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		file.printf("\"%s\", %d, ", DefaultResourceNames[j].c_str(), p.MaxResources[j]);
	}
	// Last Resources
	file.printf("},\n  \"last-resources\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		file.printf("\"%s\", %d, ", DefaultResourceNames[j].c_str(), p.LastResources[j]);
	}
	// Incomes
	file.printf("},\n  \"incomes\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		if (j) {
			if (j == MaxCosts / 2) {
				file.printf("\n ");
			} else {
				file.printf(" ");
			}
		}
		file.printf("\"%s\", %d,", DefaultResourceNames[j].c_str(), p.Incomes[j]);
	}
	// Revenue
	file.printf("},\n  \"revenue\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		if (j) {
			if (j == MaxCosts / 2) {
				file.printf("\n ");
			} else {
				file.printf(" ");
			}
		}
		file.printf("\"%s\", %d,", DefaultResourceNames[j].c_str(), p.Revenue[j]);
	}

	// UnitTypesCount done by load units.

	file.printf("},\n  \"%s\",\n", p.AiEnabled ? "ai-enabled" : "ai-disabled");

	// Ai done by load ais.
	// Units done by load units.
	// TotalNumUnits done by load units.
	// NumBuildings done by load units.
	
	//Wyrmgus start
	if (p.Revealed) {
		file.printf(" \"revealed\",");
	}
	//Wyrmgus end
	
	file.printf(" \"supply\", %d,", p.Supply);
	file.printf(" \"unit-limit\", %d,", p.UnitLimit);
	file.printf(" \"building-limit\", %d,", p.BuildingLimit);
	file.printf(" \"total-unit-limit\", %d,", p.TotalUnitLimit);

	file.printf("\n  \"score\", %d,", p.Score);
	file.printf("\n  \"total-units\", %d,", p.TotalUnits);
	file.printf("\n  \"total-buildings\", %d,", p.TotalBuildings);
	file.printf("\n  \"total-resources\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		if (j) {
			file.printf(" ");
		}
		file.printf("%d,", p.TotalResources[j]);
	}
	file.printf("},");
	file.printf("\n  \"total-razings\", %d,", p.TotalRazings);
	file.printf("\n  \"total-kills\", %d,", p.TotalKills);
	//Wyrmgus start
	for (size_t i = 0; i < UnitTypes.size(); ++i) {
		if (p.UnitTypeKills[i] != 0) {
			file.printf("\n  \"unit-type-kills\", \"%s\", %d,", UnitTypes[i]->Ident.c_str(), p.UnitTypeKills[i]);
		}
	}
	//Wyrmgus end
	if (p.LostTownHallTimer != 0) {
		file.printf("\n  \"lost-town-hall-timer\", %d,", p.LostTownHallTimer);
	}
	//Wyrmgus end

	file.printf("\n  \"speed-resource-harvest\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		if (j) {
			file.printf(" ");
		}
		file.printf("%d,", p.SpeedResourcesHarvest[j]);
	}
	file.printf("},");
	file.printf("\n  \"speed-resource-return\", {");
	for (int j = 0; j < MaxCosts; ++j) {
		if (j) {
			file.printf(" ");
		}
		file.printf("%d,", p.SpeedResourcesReturn[j]);
	}
	file.printf("},");
	file.printf("\n  \"speed-build\", %d,", p.SpeedBuild);
	file.printf("\n  \"speed-train\", %d,", p.SpeedTrain);
	file.printf("\n  \"speed-upgrade\", %d,", p.SpeedUpgrade);
	file.printf("\n  \"speed-research\", %d,", p.SpeedResearch);

	//Wyrmgus start
	/*
	Uint8 r, g, b;

	SDL_GetRGB(p.Color, TheScreen->format, &r, &g, &b);
	file.printf("\n  \"color\", { %d, %d, %d },", r, g, b);
	*/
	//Wyrmgus end

	// UnitColors done by init code.
	// Allow saved by allow.

	file.printf("\n  \"timers\", {");
	for (int j = 0; j < UpgradeMax; ++j) {
		if (j) {
			file.printf(" ,");
		}
		file.printf("%d", p.UpgradeTimers.Upgrades[j]);
	}
	file.printf("}");

	file.printf(")\n\n");

	DebugPrint("FIXME: must save unit-stats?\n");
}

/**
**  Create a new player.
**
**  @param type  Player type (Computer,Human,...).
*/
void CreatePlayer(int type)
{
	if (NumPlayers == PlayerMax) { // already done for bigmaps!
		return;
	}
	CPlayer &player = Players[NumPlayers];
	player.Index = NumPlayers;

	player.Init(type);
}

void CPlayer::Init(/* PlayerTypes */ int type)
{
	std::vector<CUnit *>().swap(this->Units);
	std::vector<CUnit *>().swap(this->FreeWorkers);
	//Wyrmgus start
	std::vector<CUnit *>().swap(this->LevelUpUnits);
	//Wyrmgus end

	//  Take first slot for person on this computer,
	//  fill other with computer players.
	if (type == PlayerPerson && !NetPlayers) {
		if (!ThisPlayer) {
			ThisPlayer = this;
		} else {
			type = PlayerComputer;
		}
	}
	if (NetPlayers && NumPlayers == NetLocalPlayerNumber) {
		ThisPlayer = &Players[NetLocalPlayerNumber];
	}

	if (NumPlayers == PlayerMax) {
		static int already_warned;

		if (!already_warned) {
			DebugPrint("Too many players\n");
			already_warned = 1;
		}
		return;
	}

	//  Make simple teams:
	//  All person players are enemies.
	int team;
	switch (type) {
		case PlayerNeutral:
		case PlayerNobody:
		default:
			team = 0;
			this->SetName("Neutral");
			break;
		case PlayerComputer:
			team = 1;
			this->SetName("Computer");
			break;
		case PlayerPerson:
			team = 2 + NumPlayers;
			this->SetName("Person");
			break;
		case PlayerRescuePassive:
		case PlayerRescueActive:
			// FIXME: correct for multiplayer games?
			this->SetName("Computer");
			team = 2 + NumPlayers;
			break;
	}
	DebugPrint("CreatePlayer name %s\n" _C_ this->Name.c_str());

	this->Type = type;
	this->Race = 0;
	//Wyrmgus start
	this->Faction = -1;
	//Wyrmgus end
	this->Team = team;
	this->Enemy = 0;
	this->Allied = 0;
	this->AiName = "ai-passive";

	//  Calculate enemy/allied mask.
	for (int i = 0; i < NumPlayers; ++i) {
		switch (type) {
			case PlayerNeutral:
			case PlayerNobody:
			default:
				break;
			case PlayerComputer:
				// Computer allied with computer and enemy of all persons.
				//Wyrmgus start
				/*
				if (Players[i].Type == PlayerComputer) {
					this->Allied |= (1 << i);
					Players[i].Allied |= (1 << NumPlayers);
				} else if (Players[i].Type == PlayerPerson || Players[i].Type == PlayerRescueActive) {
				*/
				// make computer players be hostile to each other by default
				if (Players[i].Type == PlayerComputer || Players[i].Type == PlayerPerson || Players[i].Type == PlayerRescueActive) {
				//Wyrmgus end
					this->Enemy |= (1 << i);
					Players[i].Enemy |= (1 << NumPlayers);
				}
				break;
			case PlayerPerson:
				// Humans are enemy of all?
				if (Players[i].Type == PlayerComputer || Players[i].Type == PlayerPerson) {
					this->Enemy |= (1 << i);
					Players[i].Enemy |= (1 << NumPlayers);
				} else if (Players[i].Type == PlayerRescueActive || Players[i].Type == PlayerRescuePassive) {
					this->Allied |= (1 << i);
					Players[i].Allied |= (1 << NumPlayers);
				}
				break;
			case PlayerRescuePassive:
				// Rescue passive are allied with persons
				if (Players[i].Type == PlayerPerson) {
					this->Allied |= (1 << i);
					Players[i].Allied |= (1 << NumPlayers);
				}
				break;
			case PlayerRescueActive:
				// Rescue active are allied with persons and enemies of computer
				if (Players[i].Type == PlayerComputer) {
					this->Enemy |= (1 << i);
					Players[i].Enemy |= (1 << NumPlayers);
				} else if (Players[i].Type == PlayerPerson) {
					this->Allied |= (1 << i);
					Players[i].Allied |= (1 << NumPlayers);
				}
				break;
		}
	}

	//  Initial default incomes.
	for (int i = 0; i < MaxCosts; ++i) {
		this->Incomes[i] = DefaultIncomes[i];
	}

	//  Initial max resource amounts.
	for (int i = 0; i < MaxCosts; ++i) {
		this->MaxResources[i] = DefaultResourceMaxAmounts[i];
	}

	memset(this->UnitTypesCount, 0, sizeof(this->UnitTypesCount));
	memset(this->UnitTypesAiActiveCount, 0, sizeof(this->UnitTypesAiActiveCount));
	//Wyrmgus start
	memset(this->UnitTypesNonHeroCount, 0, sizeof(this->UnitTypesNonHeroCount));
	this->Heroes.clear();
	//Wyrmgus end

	this->Supply = 0;
	this->Demand = 0;
	this->NumBuildings = 0;
	this->Score = 0;
	//Wyrmgus start
	this->LostTownHallTimer = 0;
	//Wyrmgus end

	this->Color = PlayerColors[NumPlayers][0];

	if (Players[NumPlayers].Type == PlayerComputer || Players[NumPlayers].Type == PlayerRescueActive) {
		this->AiEnabled = true;
	} else {
		this->AiEnabled = false;
	}
	//Wyrmgus start
	this->Revealed = false;
	//Wyrmgus end
	++NumPlayers;
}

/**
**  Change player name.
**
**  @param name    New name.
*/
void CPlayer::SetName(const std::string &name)
{
	Name = name;
}

//Wyrmgus start
/**
**  Change player faction.
**
**  @param faction    New faction.
*/
void CPlayer::SetFaction(const std::string faction_name)
{
	if (this->Faction != -1) {
		if (!PlayerRaces.Factions[this->Race][this->Faction]->FactionUpgrade.empty()) {
			UpgradeLost(*this, CUpgrade::Get(PlayerRaces.Factions[this->Race][this->Faction]->FactionUpgrade)->ID);
		}
	}
	
	if (faction_name.empty()) {
		this->Faction = -1;
		return;
	}
	
	int faction = PlayerRaces.GetFactionIndexByName(this->Race, faction_name);
	int old_language = PlayerRaces.GetFactionLanguage(this->Race, this->Faction);
	int new_language = PlayerRaces.GetFactionLanguage(this->Race, faction);
	
	if (!IsNetworkGame()) { //only set the faction's name as the player's name if this is a single player game
		this->SetName(faction_name);
	}
	this->Faction = faction;
	if (this->Faction != -1) {
		int color = -1;
		for (size_t i = 0; i < PlayerRaces.Factions[this->Race][faction]->Colors.size(); ++i) {
			if (!IsPlayerColorUsed(PlayerRaces.Factions[this->Race][faction]->Colors[i])) {
				color = PlayerRaces.Factions[this->Race][faction]->Colors[i];
				break;
			}
		}
		if (color == -1) { //if all of the faction's colors are used, get a unused player color
			for (int i = 0; i < PlayerColorMax; ++i) {
				if (!IsPlayerColorUsed(i)) {
					color = i;
					break;
				}
			}
		}
		
		if (color != -1) {
			this->Color = PlayerColors[color][0];
			this->UnitColors.Colors = PlayerColorsRGB[color];
		}
	
		if (!PlayerRaces.Factions[this->Race][this->Faction]->FactionUpgrade.empty()) {
			UpgradeAcquire(*this, CUpgrade::Get(PlayerRaces.Factions[this->Race][this->Faction]->FactionUpgrade));
		}
	} else {
		fprintf(stderr, "Invalid faction \"%s\" tried to be set for player %d of civilization \"%s\".\n", faction_name.c_str(), this->Index, PlayerRaces.Name[this->Race].c_str());
	}
	
	if (new_language != old_language) { //if the language changed, update the names of this player's units
		for (int i = 0; i < this->GetUnitCount(); ++i) {
			CUnit &unit = this->GetUnit(i);
			if (!unit.Character) {
				unit.UpdatePersonalName();
			}
		}
	}
}

/**
**  Change player faction to a randomly chosen one.
**
**  @param faction    New faction.
*/
void CPlayer::SetRandomFaction()
{
	// set random one from the civilization's factions
	int faction_count = 0;
	int local_factions[FactionMax];
	
	// first search for valid factions in the current faction's "develops to" list, and only if that fails search in all factions of the civilization
	if (this->Faction != -1) {
		for (size_t i = 0; i < PlayerRaces.Factions[this->Race][this->Faction]->DevelopsTo.size(); ++i) {
			int faction_id = PlayerRaces.GetFactionIndexByName(this->Race, PlayerRaces.Factions[this->Race][this->Faction]->DevelopsTo[i]);
			if (faction_id != -1) {
				bool faction_used = false;
				for (int j = 0; j < PlayerMax; ++j) {
					if (this->Index != j && Players[j].Type != PlayerNobody && Players[j].Name == PlayerRaces.Factions[this->Race][faction_id]->Name) {
						faction_used = true;
					}		
				}
				if (
					!faction_used
					&& ((PlayerRaces.Factions[this->Race][faction_id]->Type == "tribe" && !this->HasUpgradeClass("writing")) || ((PlayerRaces.Factions[this->Race][faction_id]->Type == "polity" && this->HasUpgradeClass("writing"))))
					&& PlayerRaces.Factions[this->Race][faction_id]->Playable
				) {
					local_factions[faction_count] = faction_id;
					faction_count += 1;
				}
			}
		}
	}
	
	if (faction_count == 0) {
		for (int i = 0; i < FactionMax; ++i) {
			if (PlayerRaces.Factions[this->Race][i] && !PlayerRaces.Factions[this->Race][i]->Name.empty()) {
				bool faction_used = false;
				for (int j = 0; j < PlayerMax; ++j) {
					if (this->Index != j && Players[j].Type != PlayerNobody && Players[j].Name == PlayerRaces.Factions[this->Race][i]->Name) {
						faction_used = true;
					}		
				}
				if (
					!faction_used
					&& ((PlayerRaces.Factions[this->Race][i]->Type == "tribe" && !this->HasUpgradeClass("writing")) || ((PlayerRaces.Factions[this->Race][i]->Type == "polity" && this->HasUpgradeClass("writing"))))
					&& PlayerRaces.Factions[this->Race][i]->Playable
				) {
					local_factions[faction_count] = i;
					faction_count += 1;
				}
			} else {
				break;
			}
		}
	}
	
	if (faction_count > 0) {
		int chosen_faction = local_factions[SyncRand(faction_count)];
		this->SetFaction(PlayerRaces.Factions[this->Race][chosen_faction]->Name);
	}
}

bool CPlayer::IsPlayerColorUsed(int color)
{
	bool color_used = false;
	for (int i = 0; i < PlayerMax; ++i) {
		if (this->Index != i && Players[i].Faction != -1 && Players[i].Type != PlayerNobody && Players[i].Color == PlayerColors[color][0]) {
			color_used = true;
		}		
	}
	return color_used;
}

bool CPlayer::HasUpgradeClass(std::string upgrade_class_name)
{
	if (this->Race == -1 || this->Faction == -1 || upgrade_class_name.empty()) {
		return false;
	}
	
	int upgrade_id = PlayerRaces.GetFactionClassUpgrade(this->Race, this->Faction, GetUpgradeClassIndexByName(upgrade_class_name));
	
	if (upgrade_id != -1 && this->Allow.Upgrades[upgrade_id] == 'R') {
		return true;
	}

	return false;
}
//Wyrmgus end

/**
**  Clear all player data excepts members which don't change.
**
**  The fields that are not cleared are
**  UnitLimit, BuildingLimit, TotalUnitLimit and Allow.
*/
void CPlayer::Clear()
{
	Index = 0;
	Name.clear();
	Type = 0;
	Race = 0;
	//Wyrmgus start
	Faction = -1;
	//Wyrmgus end
	AiName.clear();
	Team = 0;
	Enemy = 0;
	Allied = 0;
	SharedVision = 0;
	StartPos.x = 0;
	StartPos.y = 0;
	memset(Resources, 0, sizeof(Resources));
	memset(StoredResources, 0, sizeof(StoredResources));
	memset(MaxResources, 0, sizeof(MaxResources));
	memset(LastResources, 0, sizeof(LastResources));
	memset(Incomes, 0, sizeof(Incomes));
	memset(Revenue, 0, sizeof(Revenue));
	memset(UnitTypesCount, 0, sizeof(UnitTypesCount));
	memset(UnitTypesAiActiveCount, 0, sizeof(UnitTypesAiActiveCount));
	//Wyrmgus start
	memset(UnitTypesNonHeroCount, 0, sizeof(UnitTypesNonHeroCount));
	this->Heroes.clear();
	//Wyrmgus end
	AiEnabled = false;
	//Wyrmgus start
	Revealed = false;
	//Wyrmgus end
	Ai = 0;
	this->Units.resize(0);
	this->FreeWorkers.resize(0);
	//Wyrmgus start
	this->LevelUpUnits.resize(0);
	//Wyrmgus end
	NumBuildings = 0;
	Supply = 0;
	Demand = 0;
	// FIXME: can't clear limits since it's initialized already
	//	UnitLimit = 0;
	//	BuildingLimit = 0;
	//	TotalUnitLimit = 0;
	Score = 0;
	TotalUnits = 0;
	TotalBuildings = 0;
	memset(TotalResources, 0, sizeof(TotalResources));
	TotalRazings = 0;
	TotalKills = 0;
	//Wyrmgus start
	memset(UnitTypeKills, 0, sizeof(UnitTypeKills));
	LostTownHallTimer = 0;
	//Wyrmgus end
	Color = 0;
	UpgradeTimers.Clear();
	for (int i = 0; i < MaxCosts; ++i) {
		SpeedResourcesHarvest[i] = SPEEDUP_FACTOR;
		SpeedResourcesReturn[i] = SPEEDUP_FACTOR;
	}
	SpeedBuild = SPEEDUP_FACTOR;
	SpeedTrain = SPEEDUP_FACTOR;
	SpeedUpgrade = SPEEDUP_FACTOR;
	SpeedResearch = SPEEDUP_FACTOR;
}


void CPlayer::AddUnit(CUnit &unit)
{
	Assert(unit.Player != this);
	Assert(unit.PlayerSlot == static_cast<size_t>(-1));
	unit.PlayerSlot = this->Units.size();
	this->Units.push_back(&unit);
	unit.Player = this;
	Assert(this->Units[unit.PlayerSlot] == &unit);
}

void CPlayer::RemoveUnit(CUnit &unit)
{
	Assert(unit.Player == this);
	Assert(this->Units[unit.PlayerSlot] == &unit);

	//	unit.Player = NULL; // we can remove dying unit...
	CUnit *last = this->Units.back();

	this->Units[unit.PlayerSlot] = last;
	last->PlayerSlot = unit.PlayerSlot;
	this->Units.pop_back();
	unit.PlayerSlot = static_cast<size_t>(-1);
	Assert(last == &unit || this->Units[last->PlayerSlot] == last);
}

void CPlayer::UpdateFreeWorkers()
{
	FreeWorkers.clear();
	if (FreeWorkers.capacity() != 0) {
		// Just calling FreeWorkers.clear() is not always appropriate.
		// Certain paths may leave FreeWorkers in an invalid state, so
		// it's safer to re-initialize.
		std::vector<CUnit*>().swap(FreeWorkers);
	}
	const int nunits = this->GetUnitCount();

	for (int i = 0; i < nunits; ++i) {
		CUnit &unit = this->GetUnit(i);
		if (unit.IsAlive() && unit.Type->BoolFlag[HARVESTER_INDEX].value && unit.Type->ResInfo && !unit.Removed) {
			if (unit.CurrentAction() == UnitActionStill) {
				FreeWorkers.push_back(&unit);
			}
		}
	}
}

//Wyrmgus start
void CPlayer::UpdateLevelUpUnits()
{
	LevelUpUnits.clear();
	if (LevelUpUnits.capacity() != 0) {
		// Just calling LevelUpUnits.clear() is not always appropriate.
		// Certain paths may leave LevelUpUnits in an invalid state, so
		// it's safer to re-initialize.
		std::vector<CUnit*>().swap(LevelUpUnits);
	}
	const int nunits = this->GetUnitCount();

	for (int i = 0; i < nunits; ++i) {
		CUnit &unit = this->GetUnit(i);
		if (unit.IsAlive() && unit.Variable[LEVELUP_INDEX].Value >= 1 && !unit.Removed) {
			LevelUpUnits.push_back(&unit);
		}
	}
}
//Wyrmgus end

std::vector<CUnit *>::const_iterator CPlayer::UnitBegin() const
{
	return Units.begin();
}

std::vector<CUnit *>::iterator CPlayer::UnitBegin()
{
	return Units.begin();
}

std::vector<CUnit *>::const_iterator CPlayer::UnitEnd() const
{
	return Units.end();
}

std::vector<CUnit *>::iterator CPlayer::UnitEnd()
{
	return Units.end();
}

CUnit &CPlayer::GetUnit(int index) const
{
	return *Units[index];
}

int CPlayer::GetUnitCount() const
{
	return static_cast<int>(Units.size());
}



/*----------------------------------------------------------------------------
--  Resource management
----------------------------------------------------------------------------*/

/**
**  Gets the player resource.
**
**  @param resource  Resource to get.
**  @param type      Storing type
**
**  @note Storing types: 0 - overall store, 1 - store buildings, 2 - both
*/
int CPlayer::GetResource(const int resource, const int type)
{
	switch (type) {
		case STORE_OVERALL:
			return this->Resources[resource];
		case STORE_BUILDING:
			return this->StoredResources[resource];
		case STORE_BOTH:
			return this->Resources[resource] + this->StoredResources[resource];
		default:
			DebugPrint("Wrong resource type\n");
			return -1;
	}
}

/**
**  Adds/subtracts some resources to/from the player store
**
**  @param resource  Resource to add/subtract.
**  @param value     How many of this resource (can be negative).
**  @param store     If true, sets the building store resources, else the overall resources.
*/
void CPlayer::ChangeResource(const int resource, const int value, const bool store)
{
	if (value < 0) {
		const int fromStore = std::min(this->StoredResources[resource], abs(value));
		this->StoredResources[resource] -= fromStore;
		this->Resources[resource] -= abs(value) - fromStore;
		this->Resources[resource] = std::max(this->Resources[resource], 0);
	} else {
		if (store && this->MaxResources[resource] != -1) {
			this->StoredResources[resource] += std::min(value, this->MaxResources[resource] - this->StoredResources[resource]);
		} else {
			this->Resources[resource] += value;
		}
	}
}

/**
**  Change the player resource.
**
**  @param resource  Resource to change.
**  @param value     How many of this resource.
**  @param type      Resource types: 0 - overall store, 1 - store buildings, 2 - both
*/
void CPlayer::SetResource(const int resource, const int value, const int type)
{
	if (type == STORE_BOTH) {
		if (this->MaxResources[resource] != -1) {
			const int toRes = std::max(0, value - this->StoredResources[resource]);
			this->Resources[resource] = std::max(0, toRes);
			this->StoredResources[resource] = std::min(value - toRes, this->MaxResources[resource]);
		} else {
			this->Resources[resource] = value;
		}
	} else if (type == STORE_BUILDING && this->MaxResources[resource] != -1) {
		this->StoredResources[resource] = std::min(value, this->MaxResources[resource]);
	} else if (type == STORE_OVERALL) {
		this->Resources[resource] = value;
	}
}

/**
**  Check, if there enough resources for action.
**
**  @param resource  Resource to change.
**  @param value     How many of this resource.
*/
bool CPlayer::CheckResource(const int resource, const int value)
{
	int result = this->Resources[resource];
	if (this->MaxResources[resource] != -1) {
		result += this->StoredResources[resource];
	}
	return result < value ? false : true;
}


int CPlayer::GetUnitTotalCount(const CUnitType &type) const
{
	int count = UnitTypesCount[type.Slot];
	for (std::vector<CUnit *>::const_iterator it = this->UnitBegin(); it != this->UnitEnd(); ++it) {
		CUnit &unit = **it;

		if (unit.CurrentAction() == UnitActionUpgradeTo) {
			COrder_UpgradeTo &order = dynamic_cast<COrder_UpgradeTo &>(*unit.CurrentOrder());
			if (order.GetUnitType().Slot == type.Slot) {
				++count;
			}
		}
	}
	return count;
}

/**
**  Check if the unit-type didn't break any unit limits.
**
**  @param type    Type of unit.
**
**  @return        True if enough, negative on problem.
**
**  @note The return values of the PlayerCheck functions are inconsistent.
*/
int CPlayer::CheckLimits(const CUnitType &type) const
{
	//  Check game limits.
	if (type.Building && NumBuildings >= BuildingLimit) {
		Notify("%s", _("Building Limit Reached"));
		return -1;
	}
	if (!type.Building && (this->GetUnitCount() - NumBuildings) >= UnitLimit) {
		Notify("%s", _("Unit Limit Reached"));
		return -2;
	}
	//Wyrmgus start
//	if (this->Demand + type.Stats[this->Index].Variables[DEMAND_INDEX].Value > this->Supply && type.Stats[this->Index].Variables[DEMAND_INDEX].Value) {
	if (this->Demand + (type.Stats[this->Index].Variables[DEMAND_INDEX].Value * (type.TrainQuantity ? type.TrainQuantity : 1)) > this->Supply && type.Stats[this->Index].Variables[DEMAND_INDEX].Value) {
	//Wyrmgus end
		Notify("%s", _("Insufficient Supply, increase Supply."));
		return -3;
	}
	if (this->GetUnitCount() >= TotalUnitLimit) {
		Notify("%s", _("Total Unit Limit Reached"));
		return -4;
	}
	if (GetUnitTotalCount(type) >= Allow.Units[type.Slot]) {
		Notify(_("Limit of %d reached for this unit type"), Allow.Units[type.Slot]);
		return -6;
	}
	return 1;
}

/**
**  Check if enough resources for are available.
**
**  @param costs   How many costs.
**
**  @return        False if all enough, otherwise a bit mask.
**
**  @note The return values of the PlayerCheck functions are inconsistent.
*/
int CPlayer::CheckCosts(const int *costs, bool notify) const
{
	//Wyrmgus start
	bool sound_played = false;
	//Wyrmgus end
	int err = 0;
	for (int i = 1; i < MaxCosts; ++i) {
		if (this->Resources[i] + this->StoredResources[i] >= costs[i]) {
			continue;
		}
		if (notify) {
			const char *name = DefaultResourceNames[i].c_str();
			const char *actionName = DefaultActions[i].c_str();

			//Wyrmgus start
//			Notify(_("Not enough %s...%s more %s."), _(name), _(actionName), _(name));
			Notify(_("Not enough %s... %s more %s."), _(name), _(actionName), _(name)); //added extra space to look better
			//Wyrmgus end

			//Wyrmgus start
//			if (this == ThisPlayer && GameSounds.NotEnoughRes[this->Race][i].Sound) {
			if (this == ThisPlayer && GameSounds.NotEnoughRes[this->Race][i].Sound && !sound_played) {
				sound_played = true;
			//Wyrmgus end
				PlayGameSound(GameSounds.NotEnoughRes[this->Race][i].Sound, MaxSampleVolume);
			}
		}
		err |= 1 << i;
	}
	return err;
}

/**
**  Check if enough resources for new unit is available.
**
**  @param type    Type of unit.
**
**  @return        False if all enough, otherwise a bit mask.
*/
int CPlayer::CheckUnitType(const CUnitType &type) const
{
	//Wyrmgus start
//	return this->CheckCosts(type.Stats[this->Index].Costs);
	int modified_costs[MaxCosts];
	for (int i = 1; i < MaxCosts; ++i) {
		modified_costs[i] = type.Stats[this->Index].Costs[i];
		if (type.TrainQuantity) 
		{
			modified_costs[i] *= type.TrainQuantity;
		}
	}
	return this->CheckCosts(modified_costs);
	//Wyrmgus end
}

/**
**  Add costs to the resources
**
**  @param costs   How many costs.
*/
void CPlayer::AddCosts(const int *costs)
{
	for (int i = 1; i < MaxCosts; ++i) {
		ChangeResource(i, costs[i], false);
	}
}

/**
**  Add the costs of an unit type to resources
**
**  @param type    Type of unit.
*/
void CPlayer::AddUnitType(const CUnitType &type)
{
	AddCosts(type.Stats[this->Index].Costs);
}

/**
**  Add a factor of costs to the resources
**
**  @param costs   How many costs.
**  @param factor  Factor of the costs to apply.
*/
void CPlayer::AddCostsFactor(const int *costs, int factor)
{
	for (int i = 1; i < MaxCosts; ++i) {
		ChangeResource(i, costs[i] * factor / 100, true);
	}
}

/**
**  Subtract costs from the resources
**
**  @param costs   How many costs.
*/
void CPlayer::SubCosts(const int *costs)
{
	for (int i = 1; i < MaxCosts; ++i) {
		ChangeResource(i, -costs[i], true);
	}
}

/**
**  Subtract the costs of new unit from resources
**
**  @param type    Type of unit.
*/
void CPlayer::SubUnitType(const CUnitType &type)
{
	//Wyrmgus start
//	this->SubCosts(type.Stats[this->Index].Costs);
	this->SubCostsFactor(type.Stats[this->Index].Costs, 100 * (type.TrainQuantity ? type.TrainQuantity : 1));
	//Wyrmgus end
}

/**
**  Subtract a factor of costs from the resources
**
**  @param costs   How many costs.
**  @param factor  Factor of the costs to apply.
*/
void CPlayer::SubCostsFactor(const int *costs, int factor)
{
	for (int i = 1; i < MaxCosts; ++i) {
		ChangeResource(i, -costs[i] * 100 / factor);
	}
}

/**
**  Have unit of type.
**
**  @param type    Type of unit.
**
**  @return        How many exists, false otherwise.
*/
int CPlayer::HaveUnitTypeByType(const CUnitType &type) const
{
	return UnitTypesCount[type.Slot];
}

/**
**  Have unit of type.
**
**  @param ident   Identifier of unit-type that should be lookuped.
**
**  @return        How many exists, false otherwise.
**
**  @note This function should not be used during run time.
*/
int CPlayer::HaveUnitTypeByIdent(const std::string &ident) const
{
	return UnitTypesCount[UnitTypeByIdent(ident)->Slot];
}

/**
**  Initialize the Ai for all players.
*/
void PlayersInitAi()
{
	for (int player = 0; player < NumPlayers; ++player) {
		if (Players[player].AiEnabled) {
			AiInit(Players[player]);
		}
	}
}

/**
**  Handle AI of all players each game cycle.
*/
void PlayersEachCycle()
{
	for (int player = 0; player < NumPlayers; ++player) {
		CPlayer &p = Players[player];
		
		//Wyrmgus start
		if (p.LostTownHallTimer && !p.Revealed && p.LostTownHallTimer < ((int) GameCycle)) {
			p.Revealed = true;
			for (int j = 0; j < NumPlayers; ++j) {
				if (player != j && Players[j].Type != PlayerNobody) {
					Players[j].Notify(_("%s's units have been revealed!"), p.Name.c_str());
				} else {
					Players[j].Notify("%s", _("Your units have been revealed!"));
				}
			}
		}
		//Wyrmgus end

		if (p.AiEnabled) {
			AiEachCycle(p);
		}
	}
}

/**
**  Handle AI of a player each second.
**
**  @param playerIdx  the player to update AI
*/
void PlayersEachSecond(int playerIdx)
{
	CPlayer &player = Players[playerIdx];

	if ((GameCycle / CYCLES_PER_SECOND) % 10 == 0) {
		for (int res = 0; res < MaxCosts; ++res) {
			player.Revenue[res] = player.Resources[res] + player.StoredResources[res] - player.LastResources[res];
			player.Revenue[res] *= 6;  // estimate per minute
			player.LastResources[res] = player.Resources[res] + player.StoredResources[res];
		}
	}
	if (player.AiEnabled) {
		AiEachSecond(player);
	}

	player.UpdateFreeWorkers();
}

/**
**  Change current color set to new player.
**
**  FIXME: use function pointer here.
**
**  @param player  Pointer to player.
**  @param sprite  The sprite in which the colors should be changed.
*/
//Wyrmgus start
//void GraphicPlayerPixels(CPlayer &player, const CGraphic &sprite)
void GraphicPlayerPixels(int player, const CGraphic &sprite)
//Wyrmgus end
{
	Assert(PlayerColorIndexCount);

	SDL_LockSurface(sprite.Surface);
	//Wyrmgus start
//	std::vector<SDL_Color> sdlColors(player.UnitColors.Colors.begin(), player.UnitColors.Colors.end());
	std::vector<SDL_Color> sdlColors(PlayerColorsRGB[player].begin(), PlayerColorsRGB[player].end());
	
	//convert colors according to time of day
	int time_of_day_red = 0;
	int time_of_day_green = 0;
	int time_of_day_blue = 0;
	
	if (sprite.TimeOfDay == DawnTimeOfDay) {
		time_of_day_red = -20;
		time_of_day_green = -20;
		time_of_day_blue = 0;
	} else if (sprite.TimeOfDay == MorningTimeOfDay || sprite.TimeOfDay == MiddayTimeOfDay || sprite.TimeOfDay == AfternoonTimeOfDay) {
		time_of_day_red = 0;
		time_of_day_green = 0;
		time_of_day_blue = 0;
	} else if (sprite.TimeOfDay == DuskTimeOfDay) {
		time_of_day_red = 0;
		time_of_day_green = -20;
		time_of_day_blue = -20;
	} else if (sprite.TimeOfDay == FirstWatchTimeOfDay || sprite.TimeOfDay == MidnightTimeOfDay || sprite.TimeOfDay == SecondWatchTimeOfDay) {
		time_of_day_red = -45;
		time_of_day_green = -35;
		time_of_day_blue = -10;
	}
	
	if (sprite.TimeOfDay && (time_of_day_red != 0 || time_of_day_green != 0 || time_of_day_blue != 0)) {
		for (int i = 0; i < PlayerColorIndexCount; ++i) {
			sdlColors[i].r = std::max<int>(0,std::min<int>(255,int(sdlColors[i].r) + time_of_day_red));
			sdlColors[i].g = std::max<int>(0,std::min<int>(255,int(sdlColors[i].g) + time_of_day_green));
			sdlColors[i].b = std::max<int>(0,std::min<int>(255,int(sdlColors[i].b) + time_of_day_blue));
		}
	}
	//Wyrmgus end
	SDL_SetColors(sprite.Surface, &sdlColors[0], PlayerColorIndexStart, PlayerColorIndexCount);
	if (sprite.SurfaceFlip) {
		SDL_SetColors(sprite.SurfaceFlip, &sdlColors[0], PlayerColorIndexStart, PlayerColorIndexCount);
	}
	SDL_UnlockSurface(sprite.Surface);
}

/**
**  Setup the player colors for the current palette.
**
**  @todo  FIXME: could be called before PixelsXX is setup.
*/
void SetPlayersPalette()
{
	for (int i = 0; i < PlayerMax; ++i) {
		//Wyrmgus start
//		Players[i].UnitColors.Colors = PlayerColorsRGB[i];
		if (Players[i].Faction == -1) {
			Players[i].UnitColors.Colors = PlayerColorsRGB[i];
		}
		//Wyrmgus end
	}
}

/**
**  Output debug information for players.
*/
void DebugPlayers()
{
#ifdef DEBUG
	DebugPrint("Nr   Color   I Name     Type         Race    Ai\n");
	DebugPrint("--  -------- - -------- ------------ ------- -----\n");
	for (int i = 0; i < PlayerMax; ++i) {
		if (Players[i].Type == PlayerNobody) {
			continue;
		}
		const char *playertype;

		switch (Players[i].Type) {
			case 0: playertype = "Don't know 0"; break;
			case 1: playertype = "Don't know 1"; break;
			case 2: playertype = "neutral     "; break;
			case 3: playertype = "nobody      "; break;
			case 4: playertype = "computer    "; break;
			case 5: playertype = "person      "; break;
			case 6: playertype = "rescue pas. "; break;
			case 7: playertype = "rescue akt. "; break;
			default : playertype = "?unknown?   "; break;
		}
		DebugPrint("%2d: %8.8s %c %-8.8s %s %7s %s\n" _C_ i _C_ PlayerColorNames[i].c_str() _C_
				   ThisPlayer == &Players[i] ? '*' :
				   Players[i].AiEnabled ? '+' : ' ' _C_
				   Players[i].Name.c_str() _C_ playertype _C_
				   PlayerRaces.Name[Players[i].Race].c_str() _C_
				   Players[i].AiName.c_str());
	}
#endif
}

/**
**  Notify player about a problem.
**
**  @param type    Problem type
**  @param pos     Map tile position
**  @param fmt     Message format
**  @param ...     Message varargs
**
**  @todo FIXME: We must also notfiy allied players.
*/
void CPlayer::Notify(int type, const Vec2i &pos, const char *fmt, ...) const
{
	Assert(Map.Info.IsPointOnMap(pos));
	char temp[128];
	Uint32 color;
	va_list va;

	// Notify me, and my TEAM members
	if (this != ThisPlayer && !IsTeamed(*ThisPlayer)) {
		return;
	}

	va_start(va, fmt);
	temp[sizeof(temp) - 1] = '\0';
	vsnprintf(temp, sizeof(temp) - 1, fmt, va);
	va_end(va);
	switch (type) {
		case NotifyRed:
			color = ColorRed;
			break;
		case NotifyYellow:
			color = ColorYellow;
			break;
		case NotifyGreen:
			color = ColorGreen;
			break;
		default: color = ColorWhite;
	}
	UI.Minimap.AddEvent(pos, color);
	if (this == ThisPlayer) {
		SetMessageEvent(pos, "%s", temp);
	} else {
		SetMessageEvent(pos, "(%s): %s", Name.c_str(), temp);
	}
}

/**
**  Notify player about a problem.
**
**  @param type    Problem type
**  @param pos     Map tile position
**  @param fmt     Message format
**  @param ...     Message varargs
**
**  @todo FIXME: We must also notfiy allied players.
*/
void CPlayer::Notify(const char *fmt, ...) const
{
	// Notify me, and my TEAM members
	if (this != ThisPlayer && !IsTeamed(*ThisPlayer)) {
		return;
	}
	char temp[128];
	va_list va;

	va_start(va, fmt);
	temp[sizeof(temp) - 1] = '\0';
	vsnprintf(temp, sizeof(temp) - 1, fmt, va);
	va_end(va);
	if (this == ThisPlayer) {
		SetMessage("%s", temp);
	} else {
		SetMessage("(%s): %s", Name.c_str(), temp);
	}
}

void CPlayer::SetDiplomacyNeutralWith(const CPlayer &player)
{
	this->Enemy &= ~(1 << player.Index);
	this->Allied &= ~(1 << player.Index);
}

void CPlayer::SetDiplomacyAlliedWith(const CPlayer &player)
{
	this->Enemy &= ~(1 << player.Index);
	this->Allied |= 1 << player.Index;
}

void CPlayer::SetDiplomacyEnemyWith(const CPlayer &player)
{
	this->Enemy |= 1 << player.Index;
	this->Allied &= ~(1 << player.Index);
}

void CPlayer::SetDiplomacyCrazyWith(const CPlayer &player)
{
	this->Enemy |= 1 << player.Index;
	this->Allied |= 1 << player.Index;
}

void CPlayer::ShareVisionWith(const CPlayer &player)
{
	this->SharedVision |= (1 << player.Index);
}

void CPlayer::UnshareVisionWith(const CPlayer &player)
{
	this->SharedVision &= ~(1 << player.Index);
}


/**
**  Check if the player is an enemy
*/
bool CPlayer::IsEnemy(const CPlayer &player) const
{
	return IsEnemy(player.Index);
}

/**
**  Check if the unit is an enemy
*/
bool CPlayer::IsEnemy(const CUnit &unit) const
{
	return IsEnemy(*unit.Player);
}

/**
**  Check if the player is an ally
*/
bool CPlayer::IsAllied(const CPlayer &player) const
{
	return (Allied & (1 << player.Index)) != 0;
}

/**
**  Check if the unit is an ally
*/
bool CPlayer::IsAllied(const CUnit &unit) const
{
	return IsAllied(*unit.Player);
}


bool CPlayer::IsVisionSharing() const
{
	return SharedVision != 0;
}

/**
**  Check if the player shares vision with the player
*/
bool CPlayer::IsSharedVision(const CPlayer &player) const
{
	return (SharedVision & (1 << player.Index)) != 0;
}

/**
**  Check if the player shares vision with the unit
*/
bool CPlayer::IsSharedVision(const CUnit &unit) const
{
	return IsSharedVision(*unit.Player);
}

/**
**  Check if the both players share vision
*/
bool CPlayer::IsBothSharedVision(const CPlayer &player) const
{
	return (SharedVision & (1 << player.Index)) != 0
		   && (player.SharedVision & (1 << Index)) != 0;
}

/**
**  Check if the player and the unit share vision
*/
bool CPlayer::IsBothSharedVision(const CUnit &unit) const
{
	return IsBothSharedVision(*unit.Player);
}

/**
**  Check if the player is teamed
*/
bool CPlayer::IsTeamed(const CPlayer &player) const
{
	return Team == player.Team;
}

/**
**  Check if the unit is teamed
*/
bool CPlayer::IsTeamed(const CUnit &unit) const
{
	return IsTeamed(*unit.Player);
}

//Wyrmgus start
void SetCivilizationStringToIndex(std::string civilization_name, int civilization_id)
{
	CivilizationStringToIndex[civilization_name] = civilization_id;
}

void SetFactionStringToIndex(int civilization, std::string faction_name, int faction_id)
{
	FactionStringToIndex[civilization][faction_name] = faction_id;
}

void NetworkSetFaction(int player, std::string faction_name)
{
	int faction = PlayerRaces.GetFactionIndexByName(Players[player].Race, faction_name);
	SendCommandSetFaction(player, faction);
}

std::string GetFactionEffectsString(std::string civilization_name, std::string faction_name)
{
	int civilization = PlayerRaces.GetRaceIndexByName(civilization_name.c_str());
	if (civilization != -1) {
		int faction = PlayerRaces.GetFactionIndexByName(civilization, faction_name);
		if (faction != -1) {
			std::string faction_effects_string;
			
			//check if the faction has a different unit type from its civilization
			bool first_element = true;
			for (int i = 0; i < UnitTypeClassMax; ++i) {
				int unit_type_id = PlayerRaces.GetFactionClassUnitType(civilization, faction, i);
				int base_unit_type_id = PlayerRaces.GetCivilizationClassUnitType(civilization, i);
				if (unit_type_id != -1 && unit_type_id != base_unit_type_id) {
					if (!first_element) {
						faction_effects_string += ", ";
					} else {
						first_element = false;
					}
					
					faction_effects_string += UnitTypes[unit_type_id]->Name;
					faction_effects_string += " (";
					
					if (UnitTypes[unit_type_id]->Name != UnitTypes[base_unit_type_id]->Name) {
						faction_effects_string += FindAndReplaceString(CapitalizeString(UnitTypes[unit_type_id]->Class), "-", " ");
						faction_effects_string += ", ";
					}
					
					bool first_var = true;
					for (size_t j = 0; j < UnitTypeVar.GetNumberVariable(); ++j) {
						if (j == PRIORITY_INDEX || j == POINTS_INDEX) {
							continue;
						}
						
						if (UnitTypes[unit_type_id]->DefaultStat.Variables[j].Value != UnitTypes[base_unit_type_id]->DefaultStat.Variables[j].Value) {
							if (!first_var) {
								faction_effects_string += ", ";
							} else {
								first_var = false;
							}
							
							int variable_difference = UnitTypes[unit_type_id]->DefaultStat.Variables[j].Value - UnitTypes[base_unit_type_id]->DefaultStat.Variables[j].Value;
							if (variable_difference > 0) {
								faction_effects_string += "+";
							}
							faction_effects_string += std::to_string((long long) variable_difference);
							if (j == BACKSTAB_INDEX || j == BONUSAGAINSTMOUNTED_INDEX || j == BONUSAGAINSTBUILDINGS_INDEX || j == BONUSAGAINSTAIR_INDEX || j == BONUSAGAINSTGIANTS_INDEX || j == BONUSAGAINSTDRAGONS_INDEX || j == FIRERESISTANCE_INDEX || j == COLDRESISTANCE_INDEX || j == ARCANERESISTANCE_INDEX || j == LIGHTNINGRESISTANCE_INDEX || j == AIRRESISTANCE_INDEX || j == EARTHRESISTANCE_INDEX || j == WATERRESISTANCE_INDEX || j == HACKRESISTANCE_INDEX || j == PIERCERESISTANCE_INDEX || j == BLUNTRESISTANCE_INDEX || j == TIMEEFFICIENCYBONUS_INDEX) {
								faction_effects_string += "%";
							}
							faction_effects_string += " ";
							
							std::string variable_name = UnitTypeVar.VariableNameLookup[j];
							variable_name = FindAndReplaceString(variable_name, "BasicDamage", "Damage");
							variable_name = FindAndReplaceString(variable_name, "SightRange", "Sight");
							variable_name = FindAndReplaceString(variable_name, "AttackRange", "Range");
							variable_name = SeparateCapitalizedStringElements(variable_name);
							faction_effects_string += variable_name;
						}
					}
					
					faction_effects_string += ")";
				}
			}
			
			//check if the faction's upgrade makes modifications to any units
			if (!PlayerRaces.Factions[civilization][faction]->FactionUpgrade.empty()) {
				int faction_upgrade_id = CUpgrade::Get(PlayerRaces.Factions[civilization][faction]->FactionUpgrade)->ID;
				
				for (int z = 0; z < NumUpgradeModifiers; ++z) {
					if (UpgradeModifiers[z]->UpgradeId == faction_upgrade_id && !UpgradeModifiers[z]->ConvertTo) {
						for (size_t i = 0; i < UnitTypes.size(); ++i) {
							Assert(UpgradeModifiers[z]->ApplyTo[i] == '?' || UpgradeModifiers[z]->ApplyTo[i] == 'X');

							if (UpgradeModifiers[z]->ApplyTo[i] == 'X') {
								if (!first_element) {
									faction_effects_string += ", ";
								} else {
									first_element = false;
								}
									
								faction_effects_string += UnitTypes[i]->Name;
								faction_effects_string += " (";

								bool first_var = true;
								for (size_t j = 0; j < UnitTypeVar.GetNumberVariable(); ++j) {
									if (j == PRIORITY_INDEX || j == POINTS_INDEX) {
										continue;
									}
						
									if (UpgradeModifiers[z]->Modifier.Variables[j].Value != 0) {
										if (!first_var) {
											faction_effects_string += ", ";
										} else {
											first_var = false;
										}
											
										if (UpgradeModifiers[z]->Modifier.Variables[j].Value > 0) {
											faction_effects_string += "+";
										}
										faction_effects_string += std::to_string((long long) UpgradeModifiers[z]->Modifier.Variables[j].Value);
										if (j == BACKSTAB_INDEX || j == BONUSAGAINSTMOUNTED_INDEX || j == BONUSAGAINSTBUILDINGS_INDEX || j == BONUSAGAINSTAIR_INDEX || j == BONUSAGAINSTGIANTS_INDEX || j == BONUSAGAINSTDRAGONS_INDEX || j == FIRERESISTANCE_INDEX || j == COLDRESISTANCE_INDEX || j == ARCANERESISTANCE_INDEX || j == LIGHTNINGRESISTANCE_INDEX || j == AIRRESISTANCE_INDEX || j == EARTHRESISTANCE_INDEX || j == WATERRESISTANCE_INDEX || j == HACKRESISTANCE_INDEX || j == PIERCERESISTANCE_INDEX || j == BLUNTRESISTANCE_INDEX || j == TIMEEFFICIENCYBONUS_INDEX) {
											faction_effects_string += "%";
										}
										faction_effects_string += " ";
											
										std::string variable_name = UnitTypeVar.VariableNameLookup[j];
										variable_name = FindAndReplaceString(variable_name, "BasicDamage", "Damage");
										variable_name = FindAndReplaceString(variable_name, "SightRange", "Sight");
										variable_name = FindAndReplaceString(variable_name, "AttackRange", "Range");
										variable_name = SeparateCapitalizedStringElements(variable_name);
										faction_effects_string += variable_name;
									}
								}
						
								faction_effects_string += ")";
							}
						}
					}
				}
			}
			
			return faction_effects_string;
		}
	}
	
	return "";
}

std::string GetGovernmentTypeNameById(int government_type)
{
	if (government_type == GovernmentTypeMonarchy) {
		return "monarchy";
	} else if (government_type == GovernmentTypeRepublic) {
		return "republic";
	} else if (government_type == GovernmentTypeTheocracy) {
		return "theocracy";
	}

	return "";
}

int GetGovernmentTypeIdByName(std::string government_type)
{
	if (government_type == "monarchy") {
		return GovernmentTypeMonarchy;
	} else if (government_type == "republic") {
		return GovernmentTypeRepublic;
	} else if (government_type == "theocracy") {
		return GovernmentTypeTheocracy;
	}

	return -1;
}

std::string GetWordTypeNameById(int word_type)
{
	if (word_type == WordTypeNoun) {
		return "noun";
	} else if (word_type == WordTypeVerb) {
		return "verb";
	} else if (word_type == WordTypeAdjective) {
		return "adjective";
	} else if (word_type == WordTypePronoun) {
		return "pronoun";
	} else if (word_type == WordTypeAdverb) {
		return "adverb";
	} else if (word_type == WordTypeConjunction) {
		return "conjunction";
	} else if (word_type == WordTypeAdposition) {
		return "adposition";
	} else if (word_type == WordTypeArticle) {
		return "article";
	} else if (word_type == WordTypeNumeral) {
		return "numeral";
	}

	return "";
}

int GetWordTypeIdByName(std::string word_type)
{
	if (word_type == "noun") {
		return WordTypeNoun;
	} else if (word_type == "verb") {
		return WordTypeVerb;
	} else if (word_type == "adjective") {
		return WordTypeAdjective;
	} else if (word_type == "pronoun") {
		return WordTypePronoun;
	} else if (word_type == "adverb") {
		return WordTypeAdverb;
	} else if (word_type == "conjunction") {
		return WordTypeConjunction;
	} else if (word_type == "adposition") {
		return WordTypeAdposition;
	} else if (word_type == "article") {
		return WordTypeArticle;
	} else if (word_type == "numeral") {
		return WordTypeNumeral;
	}

	return -1;
}

std::string GetArticleTypeNameById(int article_type)
{
	if (article_type == ArticleTypeNoArticle) {
		return "no-article";
	} else if (article_type == ArticleTypeDefinite) {
		return "definite";
	} else if (article_type == ArticleTypeIndefinite) {
		return "indefinite";
	}

	return "";
}

int GetArticleTypeIdByName(std::string article_type)
{
	if (article_type == "no-article") {
		return ArticleTypeNoArticle;
	} else if (article_type == "definite") {
		return ArticleTypeDefinite;
	} else if (article_type == "indefinite") {
		return ArticleTypeIndefinite;
	}

	return -1;
}

std::string GetGrammaticalCaseNameById(int grammatical_case)
{
	if (grammatical_case == GrammaticalCaseNominative) {
		return "nominative";
	} else if (grammatical_case == GrammaticalCaseAccusative) {
		return "accusative";
	} else if (grammatical_case == GrammaticalCaseDative) {
		return "dative";
	} else if (grammatical_case == GrammaticalCaseGenitive) {
		return "genitive";
	}

	return "";
}

int GetGrammaticalCaseIdByName(std::string grammatical_case)
{
	if (grammatical_case == "nominative") {
		return GrammaticalCaseNominative;
	} else if (grammatical_case == "accusative") {
		return GrammaticalCaseAccusative;
	} else if (grammatical_case == "dative") {
		return GrammaticalCaseDative;
	} else if (grammatical_case == "genitive") {
		return GrammaticalCaseGenitive;
	}

	return -1;
}

std::string GetGrammaticalNumberNameById(int grammatical_number)
{
	if (grammatical_number == GrammaticalNumberNoNumber) {
		return "no-number";
	} else if (grammatical_number == GrammaticalNumberSingular) {
		return "singular";
	} else if (grammatical_number == GrammaticalNumberPlural) {
		return "plural";
	}

	return "";
}

int GetGrammaticalNumberIdByName(std::string grammatical_number)
{
	if (grammatical_number == "no-number") {
		return GrammaticalNumberNoNumber;
	} else if (grammatical_number == "singular") {
		return GrammaticalNumberSingular;
	} else if (grammatical_number == "plural") {
		return GrammaticalNumberPlural;
	}

	return -1;
}

std::string GetGrammaticalPersonNameById(int grammatical_person)
{
	if (grammatical_person == GrammaticalPersonFirstPerson) {
		return "first-person";
	} else if (grammatical_person == GrammaticalPersonSecondPerson) {
		return "second-person";
	} else if (grammatical_person == GrammaticalPersonThirdPerson) {
		return "third-person";
	}

	return "";
}

int GetGrammaticalPersonIdByName(std::string grammatical_person)
{
	if (grammatical_person == "first-person") {
		return GrammaticalPersonFirstPerson;
	} else if (grammatical_person == "second-person") {
		return GrammaticalPersonSecondPerson;
	} else if (grammatical_person == "third-person") {
		return GrammaticalPersonThirdPerson;
	}

	return -1;
}

std::string GetGrammaticalGenderNameById(int grammatical_gender)
{
	if (grammatical_gender == GrammaticalGenderNoGender) {
		return "no-gender";
	} else if (grammatical_gender == GrammaticalGenderMasculine) {
		return "masculine";
	} else if (grammatical_gender == GrammaticalGenderFeminine) {
		return "feminine";
	} else if (grammatical_gender == GrammaticalGenderNeuter) {
		return "neuter";
	}

	return "";
}

int GetGrammaticalGenderIdByName(std::string grammatical_gender)
{
	if (grammatical_gender == "no-gender") {
		return GrammaticalGenderNoGender;
	} else if (grammatical_gender == "masculine") {
		return GrammaticalGenderMasculine;
	} else if (grammatical_gender == "feminine") {
		return GrammaticalGenderFeminine;
	} else if (grammatical_gender == "neuter") {
		return GrammaticalGenderNeuter;
	}

	return -1;
}

std::string GetGrammaticalTenseNameById(int grammatical_tense)
{
	if (grammatical_tense == GrammaticalTensePresent) {
		return "present";
	} else if (grammatical_tense == GrammaticalTensePast) {
		return "past";
	} else if (grammatical_tense == GrammaticalTenseFuture) {
		return "future";
	}

	return "";
}

int GetGrammaticalTenseIdByName(std::string grammatical_tense)
{
	if (grammatical_tense == "present") {
		return GrammaticalTensePresent;
	} else if (grammatical_tense == "past") {
		return GrammaticalTensePast;
	} else if (grammatical_tense == "future") {
		return GrammaticalTenseFuture;
	}

	return -1;
}

std::string GetGrammaticalMoodNameById(int grammatical_mood)
{
	if (grammatical_mood == GrammaticalMoodIndicative) {
		return "indicative";
	} else if (grammatical_mood == GrammaticalMoodSubjunctive) {
		return "subjunctive";
	}

	return "";
}

int GetGrammaticalMoodIdByName(std::string grammatical_mood)
{
	if (grammatical_mood == "indicative") {
		return GrammaticalMoodIndicative;
	} else if (grammatical_mood == "subjunctive") {
		return GrammaticalMoodSubjunctive;
	}

	return -1;
}

std::string GetComparisonDegreeNameById(int comparison_degree)
{
	if (comparison_degree == ComparisonDegreePositive) {
		return "positive";
	} else if (comparison_degree == ComparisonDegreeComparative) {
		return "comparative";
	} else if (comparison_degree == ComparisonDegreeSuperlative) {
		return "superlative";
	}

	return "";
}

int GetComparisonDegreeIdByName(std::string comparison_degree)
{
	if (comparison_degree == "positive") {
		return ComparisonDegreePositive;
	} else if (comparison_degree == "comparative") {
		return ComparisonDegreeComparative;
	} else if (comparison_degree == "superlative") {
		return ComparisonDegreeSuperlative;
	}

	return -1;
}

std::string GetAffixTypeNameById(int affix_type)
{
	if (affix_type == AffixTypePrefix) {
		return "prefix";
	} else if (affix_type == AffixTypeSuffix) {
		return "suffix";
	} else if (affix_type == AffixTypeInfix) {
		return "infix";
	}

	return "";
}

int GetAffixTypeIdByName(std::string affix_type)
{
	if (affix_type == "prefix") {
		return AffixTypePrefix;
	} else if (affix_type == "suffix") {
		return AffixTypeSuffix;
	} else if (affix_type == "infix") {
		return AffixTypeInfix;
	}

	return -1;
}

std::string GetWordJunctionTypeNameById(int word_junction_type)
{
	if (word_junction_type == WordJunctionTypeCompound) {
		return "compound";
	} else if (word_junction_type == WordJunctionTypeSeparate) {
		return "separate";
	}

	return "";
}

int GetWordJunctionTypeIdByName(std::string word_junction_type)
{
	if (word_junction_type == "compound") {
		return WordJunctionTypeCompound;
	} else if (word_junction_type == "separate") {
		return WordJunctionTypeSeparate;
	}

	return -1;
}

std::string CLanguage::GetArticle(int gender, int grammatical_case, int article_type, int grammatical_number)
{
	for (size_t i = 0; i < this->LanguageWords.size(); ++i) {
		if (this->LanguageWords[i]->Type != WordTypeArticle || this->LanguageWords[i]->ArticleType != article_type) {
			continue;
		}
		
		if (grammatical_number != -1 && this->LanguageWords[i]->GrammaticalNumber != -1 && this->LanguageWords[i]->GrammaticalNumber != grammatical_number) {
			continue;
		}
		
		if (gender == -1 || this->LanguageWords[i]->Gender == -1 || gender == this->LanguageWords[i]->Gender) {
			if (grammatical_case == GrammaticalCaseNominative && !this->LanguageWords[i]->Nominative.empty()) {
				return this->LanguageWords[i]->Nominative;
			} else if (grammatical_case == GrammaticalCaseAccusative && !this->LanguageWords[i]->Accusative.empty()) {
				return this->LanguageWords[i]->Accusative;
			} else if (grammatical_case == GrammaticalCaseDative && !this->LanguageWords[i]->Dative.empty()) {
				return this->LanguageWords[i]->Dative;
			} else if (grammatical_case == GrammaticalCaseGenitive && !this->LanguageWords[i]->Genitive.empty()) {
				return this->LanguageWords[i]->Genitive;
			}
		}
	}
	return "";
}

std::string CLanguage::GetAdjectiveEnding(int article_type, int grammatical_case, int grammatical_number, int grammatical_gender)
{
	if (!this->ArticleCaseNumberGenderAdjectiveEndings[article_type][grammatical_case][grammatical_number][grammatical_gender].empty()) {
		return this->ArticleCaseNumberGenderAdjectiveEndings[article_type][grammatical_case][grammatical_number][grammatical_gender];
	} else if (!this->ArticleCaseNumberGenderAdjectiveEndings[article_type][grammatical_case][grammatical_number][GrammaticalGenderNoGender].empty()) {
		return this->ArticleCaseNumberGenderAdjectiveEndings[article_type][grammatical_case][grammatical_number][GrammaticalGenderNoGender];
	} else if (!this->ArticleCaseNumberGenderAdjectiveEndings[article_type][grammatical_case][GrammaticalNumberNoNumber][GrammaticalGenderNoGender].empty()) {
		return this->ArticleCaseNumberGenderAdjectiveEndings[article_type][grammatical_case][GrammaticalNumberNoNumber][GrammaticalGenderNoGender];
	}
	
	return "";
}

int CLanguage::GetPotentialNameQuantityForType(std::string type)
{
	int name_count = 0;
	int affix_count[MaxWordJunctionTypes][MaxAffixTypes];
	for (int i = 0; i < MaxWordJunctionTypes; ++i) {
		for (int j = 0; j < MaxAffixTypes; ++j) {
			affix_count[i][j] = 0;
		}
	}

	for (size_t i = 0; i < this->LanguageWords.size(); ++i) {
		if (this->LanguageWords[i]->HasNameType(type)) {
			name_count += 1;
		}
		
		for (int j = 0; j < MaxWordJunctionTypes; ++j) {
			for (int k = 0; k < MaxAffixTypes; ++k) {
				for (int n = 0; n < MaxGrammaticalNumbers; ++n) {
					if (this->LanguageWords[i]->HasAffixNameType(type, j, k, n)) {
						affix_count[j][k] += 1;
					}
				}
			}
		}
	}
	
	for (int i = 0; i < MaxWordJunctionTypes; ++i) {
		name_count += affix_count[i][AffixTypePrefix] * affix_count[i][AffixTypeSuffix];
		name_count += affix_count[i][AffixTypePrefix] * affix_count[i][AffixTypeInfix] * affix_count[i][AffixTypeSuffix];
	}
	
	return name_count;
}

bool LanguageWord::HasNameType(std::string type)
{
	return std::find(this->NameTypes.begin(), this->NameTypes.end(), type) != this->NameTypes.end();
}

bool LanguageWord::HasAffixNameType(std::string type, int word_junction_type, int affix_type, int grammatical_number)
{
	return std::find(this->AffixNameTypes[word_junction_type][affix_type][grammatical_number].begin(), this->AffixNameTypes[word_junction_type][affix_type][grammatical_number].end(), type) != this->AffixNameTypes[word_junction_type][affix_type][grammatical_number].end();
}

bool LanguageWord::HasMeaning(std::string meaning)
{
	return std::find(this->Meanings.begin(), this->Meanings.end(), meaning) != this->Meanings.end();
}

std::string LanguageWord::GetNumberCaseInflection(int grammatical_number, int grammatical_case)
{
	if (!this->NumberCaseInflections[grammatical_number][grammatical_case].empty()) {
		return this->NumberCaseInflections[grammatical_number][grammatical_case];
	}
	
	return this->Word;
}

std::string LanguageWord::GetNumberPersonTenseMoodInflection(int grammatical_number, int grammatical_person, int grammatical_tense, int grammatical_mood)
{
	if (!this->NumberPersonTenseMoodInflections[grammatical_number][grammatical_person][grammatical_tense][grammatical_mood].empty()) {
		return this->NumberPersonTenseMoodInflections[grammatical_number][grammatical_person][grammatical_tense][grammatical_mood];
	}
	
	return this->Word;
}

std::string LanguageWord::GetComparisonDegreeInflection(int comparison_degree)
{
	if (!this->ComparisonDegreeInflections[comparison_degree].empty()) {
		return this->ComparisonDegreeInflections[comparison_degree];
	}
	
	return this->Word;
}

std::string LanguageWord::GetParticiple(int grammatical_tense)
{
	if (!this->Participles[grammatical_tense].empty()) {
		return this->Participles[grammatical_tense];
	}
	
	return this->Word;
}

std::string LanguageWord::GetAffixForm(LanguageWord *prefix, LanguageWord *suffix, std::string type, int word_junction_type, int affix_type)
{
	int grammatical_number = GrammaticalNumberSingular;
	int grammatical_case = GrammaticalCaseNominative;
	int grammatical_tense = GrammaticalTensePresent;
	if (this->Type == WordTypeNoun) {
		if (suffix != NULL && suffix->Type == WordTypeNoun && !this->Uncountable && type == "person") { //if both this word and the element coming after it are nouns, and this isn't a personal name, and the word isn't uncountable, give a chance that the genitive be used
			if (SyncRand(2) == 0) { //50% chance the genitive will be used, 50% chance the nominative will be used instead
				grammatical_case = GrammaticalCaseGenitive;
			} else {
				grammatical_case = GrammaticalCaseNominative;
			}
		}
		
		if (prefix != NULL && prefix->Type == WordTypeNumeral && prefix->Number > 1) { //if prefix is a numeral that is greater than one, the grammatical number of this word must be plural
			grammatical_number = GrammaticalNumberPlural;
		} else if (this->HasAffixNameType(type, word_junction_type, affix_type, GrammaticalNumberSingular) && this->HasAffixNameType(type, word_junction_type, affix_type, GrammaticalNumberPlural)) {
			if (SyncRand(2) == 0) { //50% chance the plural will be used, 50% chance the singular will be used instead
				grammatical_number = GrammaticalNumberPlural;
			} else {
				grammatical_number = GrammaticalNumberSingular;
			}
		} else if (this->HasAffixNameType(type, word_junction_type, affix_type, GrammaticalNumberSingular)) {
			grammatical_number = GrammaticalNumberSingular;
		} else if (this->HasAffixNameType(type, word_junction_type, affix_type, GrammaticalNumberPlural)) {
			grammatical_number = GrammaticalNumberPlural;
		}
		
		return this->GetNumberCaseInflection(grammatical_number, grammatical_case);
	} else if (this->Type == WordTypeVerb) {
		// only using verb participles for now; maybe should add more possibilities?
		if (SyncRand(2) == 0) { //50% chance the present participle will be used, 50% chance the past participle will be used instead
			grammatical_tense = GrammaticalTensePresent;
		} else {
			grammatical_tense = GrammaticalTensePast;
		}
		return this->GetParticiple(grammatical_tense);
	}
	
	return this->Word;
}

void LanguageWord::AddNameTypeGenerationFromWord(LanguageWord *word, std::string type)
{
	if (word->HasNameType(type) && !this->HasNameType(type)) {
		this->NameTypes.push_back(type);
	}
	
	for (int i = 0; i < MaxWordJunctionTypes; ++i) {
		for (int j = 0; j < MaxAffixTypes; ++j) {
			for (int k = 0; k < MaxGrammaticalNumbers; ++k) {
				if (word->HasAffixNameType(type, i, j, k) && !this->HasAffixNameType(type, i, j, k)) {
					if (i != WordJunctionTypeSeparate || word->Type == this->Type) { //only add separate name type generation from the word if both are of the same type
						this->AffixNameTypes[i][j][k].push_back(type);
					}
				}
			}
		}
	}
}

void GenerateMissingLanguageData()
{
	std::vector<std::string> types;
	int minimum_desired_names = 25;
	
	int default_language = PlayerRaces.GetLanguageIndexByIdent("english");
	size_t markov_chain_size = 2;
	
	// generate "missing" words (missing equivalent words for English words) for languages which have that enabled, with a Markov chain process
	for (size_t i = 0; i < PlayerRaces.Languages.size(); ++i) {
		if (default_language == -1) {
			break;
		}
		
		if (PlayerRaces.Languages[i]->GenerateMissingWords) {
			// produce a list with the Markov chain elements
			size_t maximum_word_length = 0;
			std::map<std::string, std::vector<std::string>> markov_elements;
			std::map<std::string, std::vector<std::string>> markov_elements_per_type[MaxWordTypes];
			for (size_t j = 0; j < PlayerRaces.Languages[i]->LanguageWords.size(); ++j) {
				std::string word = DecapitalizeString(PlayerRaces.Languages[i]->LanguageWords[j]->Word);
				if (word.size() > maximum_word_length) {
					maximum_word_length = word.size();
				}
				
				for(size_t k = 0; k < word.size(); ++k) {
					std::string element = word.substr(k, markov_chain_size);
					
					const std::string following_letter = k < (word.size() - 1) ? word.substr(k + markov_chain_size, 1) : ""; // an empty string indicates ending the name
					markov_elements[element].push_back(following_letter);
					markov_elements_per_type[PlayerRaces.Languages[i]->LanguageWords[j]->Type][element].push_back(following_letter);

					if (k == 0) {
						markov_elements["^"].push_back(element); // "^" indicates beginning the name
						markov_elements_per_type[PlayerRaces.Languages[i]->LanguageWords[j]->Type]["^"].push_back(element);
					
						const std::string starting_following_letter = k < (word.size() - 1) ? word.substr(k + markov_chain_size - 1, 1) : "";
						std::string starting_element = "^" + word.substr(k, markov_chain_size - 1);
						markov_elements[starting_element].push_back(starting_following_letter);
						markov_elements_per_type[PlayerRaces.Languages[i]->LanguageWords[j]->Type][starting_element].push_back(starting_following_letter);
					}
				}
			}
			
			for (size_t j = 0; j < PlayerRaces.Languages[default_language]->LanguageWords.size(); ++j) {
				if (!PlayerRaces.Languages[default_language]->LanguageWords[j]->Archaic && PlayerRaces.Languages[default_language]->LanguageWords[j]->Meanings.size() > 0) {
					bool language_has_equivalent = false;
					for (size_t k = 0; k < PlayerRaces.Languages[i]->LanguageWords.size(); ++k) {
						for (size_t n = 0; n < PlayerRaces.Languages[default_language]->LanguageWords[j]->Meanings.size(); ++n) {
							if (PlayerRaces.Languages[i]->LanguageWords[k]->HasMeaning(PlayerRaces.Languages[default_language]->LanguageWords[j]->Meanings[n])) {
								language_has_equivalent = true;
								break;
							}
						}
						if (language_has_equivalent) {
							break;
						}
					}
					if (language_has_equivalent) {
						continue;
					}
					
					//generate the word from the markov elements
					std::string previous_element = "^";
					std::string new_word = "^";
					int word_type = PlayerRaces.Languages[default_language]->LanguageWords[j]->Type;
					
					while ((new_word.size() - 1) < maximum_word_length) {
						std::string next_element;
						
						if (markov_elements_per_type[word_type].size() > 0 && markov_elements_per_type[word_type][previous_element].size() > 0) {
							next_element = markov_elements_per_type[word_type][previous_element][SyncRand(markov_elements_per_type[word_type][previous_element].size())];
						} else if (markov_elements[previous_element].size() > 0) {
							next_element = markov_elements[previous_element][SyncRand(markov_elements[previous_element].size())];
						} else {
							fprintf(stderr, "Markov element \"%s\" has no elements it leads to.\n", previous_element.c_str());
						}
						
						if (next_element == "") {
							break;
						}
						
						new_word += next_element;
						previous_element = new_word.substr(new_word.size() - markov_chain_size, markov_chain_size);

						if (word_type == WordTypeArticle && (new_word.size() - 1) >= 3) { //articles can only have up to three letters
							break;
						}
					}
					
					if ((new_word.size() - 1) >= maximum_word_length || (word_type == WordTypeArticle && (new_word.size() - 1) >= 3)) { // if the word stopped being generated because it hit the character limit, take away characters until the last component of the name has an end of string as its next element
						std::string fixed_new_word = new_word;
						while (!fixed_new_word.empty()) {
							previous_element = fixed_new_word.substr(fixed_new_word.size() - std::min(markov_chain_size, fixed_new_word.size()), std::min(markov_chain_size, fixed_new_word.size()));
							bool has_string_ending = false;
							if (markov_elements_per_type[word_type].size() > 0 && markov_elements_per_type[word_type][previous_element].size()) {
								has_string_ending = std::find(markov_elements_per_type[word_type][previous_element].begin(), markov_elements_per_type[word_type][previous_element].end(), "") != markov_elements_per_type[word_type][previous_element].end();
							} else if (markov_elements[previous_element].size() > 0) {
								has_string_ending = std::find(markov_elements[previous_element].begin(), markov_elements[previous_element].end(), "") != markov_elements[previous_element].end();
							} else {
								fprintf(stderr, "Markov element \"%s\" has no elements it leads to.\n", previous_element.c_str());
							}
							
							if (has_string_ending) {
								break;
							} else {
								fixed_new_word = fixed_new_word.substr(0, fixed_new_word.size() - 1);
							}
						}
						if (!fixed_new_word.empty()) {
							new_word = fixed_new_word;
						}
					}
					
					new_word = FindAndReplaceStringBeginning(new_word, "^", "");
					new_word = CapitalizeString(new_word);
					
					LanguageWord *word = new LanguageWord;
					word->Word = new_word;
					word->Language = i;
					PlayerRaces.Languages[i]->LanguageWords.push_back(word);
					word->Type = word_type;
					word->Gender = PlayerRaces.Languages[default_language]->LanguageWords[j]->Gender;
					word->GrammaticalNumber = PlayerRaces.Languages[default_language]->LanguageWords[j]->GrammaticalNumber;
					word->Uncountable = PlayerRaces.Languages[default_language]->LanguageWords[j]->Uncountable;
					word->ArticleType = PlayerRaces.Languages[default_language]->LanguageWords[j]->ArticleType;
					word->Number = PlayerRaces.Languages[default_language]->LanguageWords[j]->Number;
					word->Nominative = word->Word;
					for (size_t k = 0; k < PlayerRaces.Languages[default_language]->LanguageWords[j]->Meanings.size(); ++k) {
						word->Meanings.push_back(PlayerRaces.Languages[default_language]->LanguageWords[j]->Meanings[k]);
					}
					
					fprintf(stdout, "Generated word: \"%s\" (\"%s\"), %s, %s language.\n", new_word.c_str(), PlayerRaces.Languages[default_language]->LanguageWords[j]->Word.c_str(), GetWordTypeNameById(word_type).c_str(), PlayerRaces.Languages[i]->Name.c_str());					
				}
			}
		}
	}

	// now, moving on to dealing with word type names
	// first build a vector with all the types
	for (size_t i = 0; i < PlayerRaces.Languages.size(); ++i) {
		for (size_t j = 0; j < PlayerRaces.Languages[i]->LanguageWords.size(); ++j) {
			for (size_t k = 0; k < PlayerRaces.Languages[i]->LanguageWords[j]->NameTypes.size(); ++k) {
				if (std::find(types.begin(), types.end(), PlayerRaces.Languages[i]->LanguageWords[j]->NameTypes[k]) == types.end()) {
					types.push_back(PlayerRaces.Languages[i]->LanguageWords[j]->NameTypes[k]);
				}
			}
			for (int k = 0; k < MaxWordJunctionTypes; ++k) {
				for (int n = 0; n < MaxAffixTypes; ++n) {
					for (int o = 0; o < MaxGrammaticalNumbers; ++o) {
						for (size_t p = 0; p < PlayerRaces.Languages[i]->LanguageWords[j]->AffixNameTypes[k][n][o].size(); ++p) {
							if (std::find(types.begin(), types.end(), PlayerRaces.Languages[i]->LanguageWords[j]->AffixNameTypes[k][n][o][p]) == types.end()) {
								types.push_back(PlayerRaces.Languages[i]->LanguageWords[j]->AffixNameTypes[k][n][o][p]);
							}
						}
					}
				}
			}
		}
	}
	
	// now, try to get a minimum quantity of names per language for each type; when failing in one of them, try to assign type name settings based on those of words derived from and to the words in the failing language
	for (size_t i = 0; i < PlayerRaces.Languages.size(); ++i) {
		for (size_t j = 0; j < types.size(); ++j) {
			bool deeper_related_word_level_exists = true;
			int relationship_depth_level = 1;
			while (PlayerRaces.Languages[i]->GetPotentialNameQuantityForType(types[j]) < minimum_desired_names && deeper_related_word_level_exists) {
				deeper_related_word_level_exists = false;
				
				for (size_t k = 0; k < PlayerRaces.Languages[i]->LanguageWords.size(); ++k) {
					std::vector<LanguageWord *> related_words;
					
					// fill the vector with all the related words for the desired relationship depth level
					if (PlayerRaces.Languages[i]->LanguageWords[k]->DerivesFrom != NULL) {
						related_words.push_back(PlayerRaces.Languages[i]->LanguageWords[k]->DerivesFrom);
					}
					for (size_t n = 0; n < PlayerRaces.Languages[i]->LanguageWords[k]->DerivesTo.size(); ++n) {
						related_words.push_back(PlayerRaces.Languages[i]->LanguageWords[k]->DerivesTo[n]);
					}
					
					for (int n = 0; n < relationship_depth_level; ++n) {
						for (size_t o = 0; o < related_words.size(); ++o) {
							if (related_words[o]->DerivesFrom != NULL && related_words[o]->DerivesFrom != PlayerRaces.Languages[i]->LanguageWords[k] && std::find(related_words.begin(), related_words.end(), related_words[o]->DerivesFrom) == related_words.end()) {
								if (n < (relationship_depth_level - 1)) {
									related_words.push_back(related_words[o]->DerivesFrom);
								} else {
									deeper_related_word_level_exists = true;
								}
							}
							for (size_t p = 0; p < related_words[o]->DerivesTo.size(); ++p) {
								if (related_words[o]->DerivesTo[p] != PlayerRaces.Languages[i]->LanguageWords[k] && std::find(related_words.begin(), related_words.end(), related_words[o]->DerivesTo[p]) == related_words.end()) {
									if (n < (relationship_depth_level - 1)) {
										related_words.push_back(related_words[o]->DerivesTo[p]);
									} else {
										deeper_related_word_level_exists = true;
									}
								}
							}
						}
					}
					
					//now attach the new type name to the word from its related words, if it is found in them
					for (size_t n = 0; n < related_words.size(); ++n) {
						PlayerRaces.Languages[i]->LanguageWords[k]->AddNameTypeGenerationFromWord(related_words[n], types[j]);
					}
				}
				
				relationship_depth_level += 1;
			}
			
			if (PlayerRaces.Languages[i]->GetPotentialNameQuantityForType(types[j]) < minimum_desired_names && default_language != -1) { //if the quantity of names is still too low, try to add name generation of this type for this language based on the default language, for words which share a meaning
				for (size_t k = 0; k < PlayerRaces.Languages[i]->LanguageWords.size(); ++k) {
					for (size_t n = 0; n < PlayerRaces.Languages[default_language]->LanguageWords.size(); ++n) {
						if (PlayerRaces.Languages[default_language]->LanguageWords[n]->Type == PlayerRaces.Languages[i]->LanguageWords[k]->Type) {
							for (size_t o = 0; o < PlayerRaces.Languages[i]->LanguageWords[k]->Meanings.size(); ++o) {
								if (PlayerRaces.Languages[default_language]->LanguageWords[n]->HasMeaning(PlayerRaces.Languages[i]->LanguageWords[k]->Meanings[o])) {
									PlayerRaces.Languages[i]->LanguageWords[k]->AddNameTypeGenerationFromWord(PlayerRaces.Languages[default_language]->LanguageWords[n], types[j]);
									break;
								}
							}
						}
					}
				}
			}
		}
	}
}
//Wyrmgus end

//@}
