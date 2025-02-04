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

#include "database/data_type.h"
#include "database/detailed_data_entry.h"
#include "data_type.h"
#include "item/item_slot.h"
#include "time/date.h"

class CFile;
class CPlayer;
class CProvince;
class CUnit;
class CUpgrade;
class LuaCallback;
class Spell_Polymorph;
struct lua_State;

extern int CclDefineCharacter(lua_State *l);
extern int CclDefineCustomHero(lua_State *l);

namespace archimedes {
	class calendar;
	class language;
	enum class gender;
}

namespace wyrmgus {
	class character_history;
	class civilization;
	class deity;
	class dynasty;
	class epithet;
	class faction;
	class historical_location;
	class icon;
	class persistent_item;
	class quest;
	class religion;
	class site;
	class unit_sound_set;
	class unit_type;
	class variation_tag;
	enum class character_title;

	template <typename scope_type>
	class and_condition;
}

enum Attributes {
	StrengthAttribute,
	DexterityAttribute,
	IntelligenceAttribute,
	CharismaAttribute,

	MaxAttributes
};

namespace wyrmgus {

class character : public detailed_data_entry, public data_type<character>, public CDataType
{
	Q_OBJECT

	Q_PROPERTY(wyrmgus::dynasty* dynasty MEMBER dynasty)
	Q_PROPERTY(wyrmgus::epithet* epithet MEMBER epithet)
	Q_PROPERTY(QString surname READ get_surname_qstring)
	Q_PROPERTY(QString full_name READ get_full_name_qstring NOTIFY changed)
	Q_PROPERTY(wyrmgus::icon* icon READ get_icon WRITE set_base_icon NOTIFY changed)
	Q_PROPERTY(wyrmgus::icon* heroic_icon MEMBER heroic_icon)
	Q_PROPERTY(wyrmgus::unit_type* unit_type MEMBER unit_type WRITE set_unit_type)
	Q_PROPERTY(wyrmgus::civilization* civilization MEMBER civilization NOTIFY changed)
	Q_PROPERTY(wyrmgus::faction* default_faction MEMBER default_faction NOTIFY changed)
	Q_PROPERTY(archimedes::gender gender READ get_gender WRITE set_gender)
	Q_PROPERTY(wyrmgus::character* father READ get_father WRITE set_father)
	Q_PROPERTY(wyrmgus::character* mother READ get_mother WRITE set_mother)
	Q_PROPERTY(wyrmgus::site* home_settlement MEMBER home_settlement)
	Q_PROPERTY(int start_year MEMBER start_year READ get_start_year)
	Q_PROPERTY(int end_year MEMBER end_year READ get_end_year)
	Q_PROPERTY(bool ai_active MEMBER ai_active READ is_ai_active)
	Q_PROPERTY(int base_level MEMBER base_level READ get_base_level)
	Q_PROPERTY(wyrmgus::character* tree_parent READ get_tree_parent_character NOTIFY changed)

public:
	static constexpr const char *class_identifier = "character";
	static constexpr const char property_class_identifier[] = "wyrmgus::character*";
	static constexpr const char *database_folder = "characters";
	static constexpr bool history_enabled = true;

	static void initialize_all()
	{
		for (character *custom_hero : character::get_custom_heroes()) {
			if (!custom_hero->is_initialized()) {
				custom_hero->initialize();
			}
		}

		data_type::initialize_all();
	}

	static void check_all()
	{
		for (const character *custom_hero : character::get_custom_heroes()) {
			custom_hero->check();
		}

		data_type::check_all();
	}

	static void clear();

	static bool compare_encyclopedia_entries(const character *lhs, const character *rhs);

	static std::vector<character *> get_all_with_custom();

	static const std::vector<character *> &get_custom_heroes()
	{
		return character::custom_heroes;
	}

	static character *get_custom_hero(const std::string &identifier)
	{
		const auto find_iterator = character::custom_heroes_by_identifier.find(identifier);
		if (find_iterator != character::custom_heroes_by_identifier.end()) {
			return find_iterator->second.get();
		}

		return nullptr;
	}

	static void create_custom_hero(const std::string &name, const std::string &surname, wyrmgus::civilization *civilization, wyrmgus::unit_type *unit_type, const std::vector<const CUpgrade *> &traits, const std::string &variation_tag_identifier);
	static void remove_custom_hero(character *custom_hero);

	static bool is_name_valid_for_custom_hero(const std::string &name);

private:
	static inline std::vector<character *> custom_heroes;
	static inline std::map<std::string, qunique_ptr<character>> custom_heroes_by_identifier;

public:
	explicit character(const std::string &identifier);
	~character();
	
	virtual void process_gsml_property(const gsml_property &property) override;
	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void ProcessConfigData(const CConfigData *config_data) override;
	virtual void initialize() override;
	virtual void check() const override;

	virtual data_entry_history *get_history_base() override;

	character_history *get_history()
	{
		return this->history.get();
	}

	const character_history *get_history() const
	{
		return this->history.get();
	}

	virtual void reset_history() override;

	virtual bool has_encyclopedia_entry() const override
	{
		if (!this->IsUsable()) {
			return false;
		}

		if (this->is_deity()) {
			return false; //already covered by the deity's encyclopedia entry
		}

		return detailed_data_entry::has_encyclopedia_entry();
	}

	virtual std::string get_encyclopedia_text() const override;
	std::string get_encyclopedia_genealogical_text() const;

	virtual std::string get_link_name() const override
	{
		return this->get_full_name();
	}

	Q_INVOKABLE void save() const;

	const wyrmgus::dynasty *get_dynasty() const
	{
		return this->dynasty;
	}

	const wyrmgus::epithet *get_epithet() const
	{
		return this->epithet;
	}

	const std::string &get_surname() const
	{
		return this->surname;
	}

	Q_INVOKABLE void set_surname(const std::string &surname)
	{
		this->surname = surname;
	}

	QString get_surname_qstring() const
	{
		return QString::fromStdString(this->get_surname());
	}

	const wyrmgus::unit_type *get_unit_type() const
	{
		return this->unit_type;
	}

	void set_unit_type(const wyrmgus::unit_type *unit_type)
	{
		if (unit_type == this->get_unit_type()) {
			return;
		}

		this->unit_type = const_cast<wyrmgus::unit_type *>(unit_type);
	}

	const std::vector<const CUpgrade *> &get_traits() const
	{
		return this->traits;
	}

	const wyrmgus::civilization *get_civilization() const
	{
		return this->civilization;
	}

	const faction *get_default_faction() const
	{
		return this->default_faction;
	}

	const wyrmgus::deity *get_deity() const
	{
		return this->deity;
	}

	void set_deity(const wyrmgus::deity *deity)
	{
		this->deity = deity;
	}

	bool is_deity() const
	{
		return this->get_deity() != nullptr;
	}

	archimedes::gender get_gender() const
	{
		return this->gender;
	}

	void set_gender(const archimedes::gender gender)
	{
		if (gender == this->get_gender()) {
			return;
		}

		this->gender = gender;
	}

	void GenerateMissingDates();
	int GetMartialAttribute() const;
	int GetAttributeModifier(int attribute) const;
	const religion *get_religion() const;
	const language *get_language() const;
	calendar *get_calendar() const;

	const std::vector<std::unique_ptr<persistent_item>> &get_items() const
	{
		return this->items;
	}

	void add_item(std::unique_ptr<persistent_item> &&item);
	void remove_item(persistent_item *item);
	persistent_item *get_item(const CUnit *item_unit);
	const persistent_item *get_item(const CUnit *item_unit) const;

	bool has_item(const CUnit *item_unit) const
	{
		return this->get_item(item_unit) != nullptr;
	}

	bool is_item_equipped(const persistent_item *item) const;
	void equip_item(const persistent_item *item);
	void deequip_item(const persistent_item *item);

	bool IsUsable() const;
	bool is_playable() const;
	bool CanAppear(bool ignore_neutral = false) const;
	bool CanWorship() const;
	bool HasMajorDeity() const;
	std::string get_full_name() const;

	QString get_full_name_qstring() const
	{
		return QString::fromStdString(this->get_full_name());
	}

	const std::set<const variation_tag *> &get_variation_tags() const
	{
		return this->variation_tags;
	}

	wyrmgus::icon *get_base_icon() const
	{
		return this->icon;
	}

	void set_base_icon(wyrmgus::icon *icon)
	{
		if (icon == this->get_icon()) {
			return;
		}

		this->icon = icon;
	}

	const wyrmgus::icon *get_heroic_icon() const
	{
		return this->heroic_icon;
	}

	wyrmgus::icon *get_icon() const;

	void UpdateAttributes();

	bool is_ai_active() const
	{
		return this->ai_active;
	}

	character *get_father() const
	{
		return this->father;
	}

	void set_father(character *character)
	{
		if (character == this->get_father()) {
			return;
		}

		this->father = character;
	}

	character *get_mother() const
	{
		return this->mother;
	}

	void set_mother(character *character)
	{
		if (character == this->get_mother()) {
			return;
		}

		this->mother = character;
	}

	const std::vector<character *> &get_children() const
	{
		return this->children;
	}

	void add_child(character *child);

	int get_start_year() const
	{
		return this->start_year;
	}

	int get_end_year() const
	{
		return this->end_year;
	}

	int get_base_level() const
	{
		return this->base_level;
	}

	int get_level() const
	{
		return this->level;
	}

	void set_level(const int level)
	{
		this->level = level;
	}

	bool is_custom() const
	{
		return this->custom;
	}

	const std::vector<const CUpgrade *> &get_base_abilities() const
	{
		return this->base_abilities;
	}

	const std::vector<const CUpgrade *> &get_abilities() const
	{
		return this->abilities;
	}

	void add_ability(const CUpgrade *ability)
	{
		this->abilities.push_back(ability);
	}

	void remove_ability(const CUpgrade *ability);

	int get_ability_count(const CUpgrade *counted_ability) const
	{
		int count = 0;

		for (const CUpgrade *ability : this->get_abilities()) {
			if (ability == counted_ability) {
				++count;
			}
		}

		return count;
	}

	const std::vector<const CUpgrade *> &get_bonus_abilities() const
	{
		return this->bonus_abilities;
	}

	void add_bonus_ability(const CUpgrade *ability);

	const unit_sound_set *get_sound_set() const
	{
		return this->sound_set.get();
	}

	const and_condition<CPlayer> *get_conditions() const
	{
		return this->conditions.get();
	}

	virtual std::unique_ptr<text_processor_base> create_text_processor() const override;

	CUnit *get_unit() const;

	std::filesystem::path get_save_filepath() const;

	character *get_tree_parent_character() const
	{
		if (this->get_dynasty() == nullptr) {
			return nullptr;
		}

		if (this->get_father() != nullptr && this->get_father()->get_dynasty() == this->get_dynasty() && !this->get_father()->is_hidden_in_tree()) {
			return this->get_father();
		}

		if (this->get_mother() != nullptr && this->get_mother()->get_dynasty() == this->get_dynasty() && !this->get_mother()->is_hidden_in_tree()) {
			return this->get_mother();
		}

		return nullptr;
	}

	virtual named_data_entry *get_tree_parent() const override
	{
		return this->get_tree_parent_character();
	}

	virtual bool is_hidden_in_tree() const override
	{
		return !this->has_encyclopedia_entry();
	}

	virtual std::vector<const named_data_entry *> get_top_tree_elements() const override;

signals:
	void changed();

public:
	CDate BirthDate;			/// Date in which the character was born
	CDate StartDate;			/// Date in which the character historically starts being active
	CDate DeathDate;			/// Date in which the character historically died
private:
	wyrmgus::dynasty *dynasty = nullptr;
	wyrmgus::civilization *civilization = nullptr;	/// Culture to which the character belongs
	faction *default_faction = nullptr;	//the default faction to which the character belongs
	const wyrmgus::deity *deity = nullptr; //the deity which the character is (if it is a deity)
	archimedes::gender gender;				/// Character's gender
	int base_level = 1; //the level that the character starts with
	int level = 0; //the character's current level
public:
	int ExperiencePercent = 0;	/// Character's experience, as a percentage of the experience required to level up
private:
	bool custom = false; //whether this character is a custom hero
	wyrmgus::epithet *epithet = nullptr;
	std::string surname; //the character's surname
	std::set<const variation_tag *> variation_tags;
	wyrmgus::icon *icon = nullptr;
	wyrmgus::icon *heroic_icon = nullptr; //the character's heroic icon (level 3 and upper)
	wyrmgus::unit_type *unit_type = nullptr;
	std::vector<const CUpgrade *> traits;
	character *father = nullptr;
	character *mother = nullptr;
	std::vector<character *> children;
	site *home_settlement = nullptr; //the home settlement of this character, where they can preferentially be recruited
	int start_year = 0;
	int end_year = 0;
public:
	std::unique_ptr<LuaCallback> Conditions;
private:
	std::unique_ptr<const and_condition<CPlayer>> conditions;
	std::map<item_slot, std::vector<const persistent_item *>> equipped_items;
	bool ai_active = true; //whether the character's AI is active
public:
	std::vector<const wyrmgus::deity *> Deities; //deities chosen by this character to worship
private:
	std::vector<const CUpgrade *> base_abilities; //the character's base abilities; these will not be lost after retraining
	std::vector<const CUpgrade *> abilities;
	std::vector<const CUpgrade *> bonus_abilities; //abilities received as a bonus e.g. from achievements; these are not lost after retraining
public:
	std::vector<const CUpgrade *> ReadWorks;
	std::vector<const CUpgrade *> ConsumedElixirs;
	std::vector<CUpgrade *> AuthoredWorks;	/// Literary works of which this character is the author
	std::vector<CUpgrade *> LiteraryAppearances;	/// Literary works in which this character appears
private:
	std::vector<std::unique_ptr<persistent_item>> default_items;
	std::vector<std::unique_ptr<persistent_item>> items;
public:
	int Attributes[MaxAttributes];
	std::vector<wyrmgus::unit_type *> ForbiddenUpgrades;	/// which unit types this character is forbidden to upgrade to
private:
	std::unique_ptr<unit_sound_set> sound_set;
	std::unique_ptr<character_history> history;
public:
	std::vector<std::pair<CDate, wyrmgus::faction *>> HistoricalFactions;
	std::vector<std::unique_ptr<historical_location>> HistoricalLocations;
	std::vector<std::tuple<CDate, CDate, wyrmgus::faction *, character_title>> HistoricalTitles; //historical titles of the character, the first element is the beginning date of the term, the second one the end date, the third the faction it pertains to (if any, if not then it is null), and the fourth is the character title itself (from the character title enums)
	std::vector<std::tuple<int, int, CProvince *, int>> HistoricalProvinceTitles;

	friend ::Spell_Polymorph;
	friend int ::CclDefineCharacter(lua_State *l);
	friend int ::CclDefineCustomHero(lua_State *l);
};

}

extern bool LoadingPersistentHeroes;

extern int GetAttributeVariableIndex(int attribute);
extern void SaveHeroes();
extern void CharacterCclRegister();
