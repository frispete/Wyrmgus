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
#include "data_type.h"
#include "map/site_container.h"
#include "map/terrain_geodata_map.h"
#include "time/date.h"
#include "util/geocoordinate.h"
#include "util/georectangle.h"
#include "util/point_container.h"
#include "vec2i.h"

class CPlayer;
class CUnit;
struct lua_State;

extern int CclDefineMapTemplate(lua_State *l);

namespace archimedes {
	class map_projection;
}

namespace wyrmgus {

class campaign;
class character;
class character_substitution;
class character_unit;
class dungeon_generation_settings;
class faction;
class generated_terrain;
class historical_location;
class historical_unit;
class map_template_history;
class map_template_unit;
class region;
class site;
class terrain_feature;
class terrain_type;
class tile;
class tileset;
class unique_item;
class unit_class;
class unit_type;
class world;

class map_template final : public named_data_entry, public data_type<map_template>, public CDataType
{
	Q_OBJECT

	Q_PROPERTY(QSize size MEMBER size READ get_size)
	Q_PROPERTY(int scale_multiplier MEMBER scale_multiplier READ get_scale_multiplier)
	Q_PROPERTY(int scale_divisor MEMBER scale_divisor READ get_scale_divisor)
	Q_PROPERTY(bool circle MEMBER circle READ is_circle)
	Q_PROPERTY(bool optional MEMBER optional READ is_optional)
	Q_PROPERTY(bool constructed_only MEMBER constructed_only READ is_constructed_only)
	Q_PROPERTY(bool tile_adjustment_enabled MEMBER tile_adjustment_enabled READ is_tile_adjustment_enabled)
	Q_PROPERTY(QPoint start_pos MEMBER start_pos READ get_start_pos)
	Q_PROPERTY(QPoint end_pos MEMBER end_pos READ get_end_pos)
	Q_PROPERTY(QPoint subtemplate_top_left_pos MEMBER subtemplate_top_left_pos READ get_subtemplate_top_left_pos)
	Q_PROPERTY(QPoint subtemplate_center_pos MEMBER subtemplate_center_pos READ get_subtemplate_center_pos)
	Q_PROPERTY(QPoint min_subtemplate_pos MEMBER min_subtemplate_pos READ get_min_subtemplate_pos)
	Q_PROPERTY(QPoint max_subtemplate_pos MEMBER max_subtemplate_pos READ get_max_subtemplate_pos)
	Q_PROPERTY(archimedes::geocoordinate min_subtemplate_geocoordinate MEMBER min_subtemplate_geocoordinate)
	Q_PROPERTY(archimedes::geocoordinate max_subtemplate_geocoordinate MEMBER max_subtemplate_geocoordinate)
	Q_PROPERTY(wyrmgus::world* world MEMBER world)
	Q_PROPERTY(wyrmgus::tileset* tileset MEMBER tileset)
	Q_PROPERTY(wyrmgus::map_template* main_template READ get_main_template WRITE set_main_template)
	Q_PROPERTY(std::filesystem::path terrain_file MEMBER terrain_file WRITE set_terrain_file)
	Q_PROPERTY(std::filesystem::path overlay_terrain_file MEMBER overlay_terrain_file WRITE set_overlay_terrain_file)
	Q_PROPERTY(std::filesystem::path trade_route_file MEMBER trade_route_file WRITE set_trade_route_file)
	Q_PROPERTY(std::filesystem::path territory_file MEMBER territory_file WRITE set_territory_file)
	Q_PROPERTY(wyrmgus::terrain_type* base_terrain_type MEMBER base_terrain_type READ get_base_terrain_type)
	Q_PROPERTY(wyrmgus::terrain_type* base_overlay_terrain_type MEMBER base_overlay_terrain_type READ get_base_overlay_terrain_type)
	Q_PROPERTY(wyrmgus::terrain_type* border_terrain_type MEMBER border_terrain_type READ get_border_terrain_type)
	Q_PROPERTY(wyrmgus::terrain_type* border_overlay_terrain_type MEMBER border_overlay_terrain_type READ get_border_overlay_terrain_type)
	Q_PROPERTY(wyrmgus::terrain_type* unusable_area_terrain_type MEMBER unusable_area_terrain_type READ get_unusable_area_terrain_type)
	Q_PROPERTY(wyrmgus::terrain_type* unusable_area_overlay_terrain_type MEMBER unusable_area_overlay_terrain_type READ get_unusable_area_overlay_terrain_type)
	Q_PROPERTY(wyrmgus::terrain_type* surrounding_terrain_type MEMBER surrounding_terrain_type READ get_surrounding_terrain_type)
	Q_PROPERTY(wyrmgus::terrain_type* surrounding_overlay_terrain_type MEMBER surrounding_overlay_terrain_type READ get_surrounding_overlay_terrain_type)
	Q_PROPERTY(wyrmgus::terrain_type* default_water_terrain_type MEMBER default_water_terrain_type)
	Q_PROPERTY(bool clear_terrain MEMBER clear_terrain)
	Q_PROPERTY(bool clear_path_to_other_subtemplates MEMBER clear_path_to_other_subtemplates READ clears_path_to_other_subtemplates)
	Q_PROPERTY(bool output_terrain_image MEMBER output_terrain_image READ outputs_terrain_image)
	Q_PROPERTY(bool output_territory_image MEMBER output_territory_image READ outputs_territory_image)
	Q_PROPERTY(bool random_created_settlements MEMBER random_created_settlements)
	Q_PROPERTY(bool create_starting_mine MEMBER create_starting_mine)
	Q_PROPERTY(archimedes::map_projection* map_projection MEMBER map_projection)
	Q_PROPERTY(archimedes::decimillesimal_int min_longitude READ get_min_longitude WRITE set_min_longitude)
	Q_PROPERTY(archimedes::decimillesimal_int max_longitude READ get_max_longitude WRITE set_max_longitude)
	Q_PROPERTY(archimedes::decimillesimal_int min_latitude READ get_min_latitude WRITE set_min_latitude)
	Q_PROPERTY(archimedes::decimillesimal_int max_latitude READ get_max_latitude WRITE set_max_latitude)
	Q_PROPERTY(archimedes::decimillesimal_int astrodistance_multiplier MEMBER astrodistance_multiplier READ get_astrodistance_multiplier)
	Q_PROPERTY(int astrodistance_additive_modifier MEMBER astrodistance_additive_modifier READ get_astrodistance_additive_modifier)
	Q_PROPERTY(wyrmgus::map_template* default_astrocoordinate_reference_subtemplate MEMBER default_astrocoordinate_reference_subtemplate)
	Q_PROPERTY(archimedes::decimillesimal_int orbit_distance_multiplier MEMBER orbit_distance_multiplier READ get_orbit_distance_multiplier)

public:
	using terrain_character_map_type = std::vector<std::vector<char>>;
	using zone_variant = std::variant<std::monostate, const site *, const terrain_feature *>;

	static constexpr const char *class_identifier = "map_template";
	static constexpr const char property_class_identifier[] = "wyrmgus::map_template*";
	static constexpr const char *database_folder = "map_templates";
	static constexpr bool history_enabled = true;
	static constexpr QPoint min_adjacent_template_distance = QPoint(4, 4);
	static constexpr QPoint max_adjacent_template_distance = QPoint(16, 16);
	static constexpr QPoint min_constructed_adjacent_template_distance = QPoint(1, 1);

	static const std::set<std::string> database_dependencies;

	static void set_terrain_image_pixel(QImage &terrain_image, const QPoint &pos, const terrain_type *terrain);

	explicit map_template(const std::string &identifier);
	~map_template();

	virtual void process_gsml_property(const gsml_property &property) override;
	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void ProcessConfigData(const CConfigData *config_data) override;
	virtual void initialize() override;
	virtual void check() const override;
	virtual data_entry_history *get_history_base() override;
	virtual void reset_history() override;
	void reset_game_data();

	void apply_terrain(const QPoint &template_start_pos, const QPoint &map_start_pos, const int z);
	void apply_terrain(const bool overlay, const QPoint &template_start_pos, const QPoint &map_start_pos, const int z);
	void apply_terrain_image(const bool overlay, const QPoint &template_start_pos, const QPoint &map_start_pos, const int z);
	void apply_territory_image(const QPoint &template_start_pos, const QPoint &map_start_pos, const int z) const;
	void apply(const QPoint &template_start_pos, const QPoint &map_start_pos, const int z);
	void apply_subtemplates(const QPoint &template_start_pos, const QPoint &map_start_pos, const QPoint &map_end, int z, bool random, bool constructed) const;
	void apply_subtemplate(map_template *subtemplate, const QPoint &template_start_pos, const QPoint &map_start_pos, const QPoint &map_end, const int z, const bool random = false) const;
	void apply_sites(const QPoint &template_start_pos, const QPoint &map_start_pos, const QPoint &map_end, const int z, const bool random = false) const;
	void apply_site(const site *site, const QPoint &site_pos, const int z) const;
	void apply_satellite_site(const site *site, int64_t &orbit_distance) const;
	void generate_zones(const int z) const;
	std::vector<QPoint> generate_zone_seeds(const int z, const size_t seed_count) const;
	zone_variant take_zone_to_generate(const int z, const std::vector<zone_variant> &generated_zones, std::vector<zone_variant> &zones_to_generate, const std::vector<zone_variant> &placed_zones, const std::vector<const region *> &zone_regions, const geocoordinate &min_geocoordinate, const geocoordinate &max_geocoordinate, const std::vector<zone_variant> &tile_zones, const std::vector<QPoint> &zone_seeds, QPoint &near_pos) const;
	void expand_zones(std::vector<zone_variant> &tile_zones, std::vector<QPoint> &&seeds, const int z) const;
	void generate_site(const site *site, const QPoint &map_start_pos, const QPoint &map_end, const int z) const;
	void apply_population_unit(const unit_class *unit_class, const int population, const QPoint &unit_pos, const int z, CPlayer *player, const site *settlement) const;
	void apply_remaining_site_populations() const;
	void ApplyConnectors(const QPoint &template_start_pos, const QPoint &map_start_pos, const QPoint &map_end, const int z, const bool random = false) const;
	void ApplyUnits(const QPoint &template_start_pos, const QPoint &map_start_pos, const QPoint &map_end, const int z, const bool random = false) const;
	void apply_character_map_units(const QPoint &template_start_pos, const QPoint &map_start_pos, const int z) const;
	void apply_historical_unit(const historical_unit *historical_unit, const QPoint &template_start_pos, const QPoint &map_start_pos, const QPoint &map_end, const int z, const bool random) const;
	void apply_character(character *character, const QPoint &template_start_pos, const QPoint &map_start_pos, const QPoint &map_end, const int z, const bool random) const;
	void apply_unit_site_properties(CUnit *unit, const site *site) const;

	void clear_application_data();

	bool IsSubtemplateArea() const;
	const map_template *GetTopMapTemplate() const;

	const QSize &get_size() const
	{
		return this->size;
	}

	int get_width() const
	{
		return this->get_size().width();
	}

	int get_height() const
	{
		return this->get_size().height();
	}

	int get_scale_multiplier() const
	{
		return this->scale_multiplier;
	}

	int get_scale_multiplier_y_offset(const int x) const
	{
		if (this->get_scale_multiplier() == 1) {
			return 0;
		}

		if (x % 2 != 0) {
			return -1;
		}

		return 0;
	}

	int get_scale_divisor() const
	{
		return this->scale_divisor;
	}

	const QPoint &get_start_pos() const
	{
		return this->start_pos;
	}

	const QPoint &get_end_pos() const
	{
		return this->end_pos;
	}

	bool contains_pos(const QPoint &pos) const
	{
		if (this->get_end_pos().x() != -1 && pos.x() > this->get_end_pos().x()) {
			return false;
		}

		if (this->get_end_pos().y() != -1 && pos.y() > this->get_end_pos().y()) {
			return false;
		}

		return pos.x() >= this->get_start_pos().x() && pos.y() >= this->get_start_pos().y();
	}

	QSize get_applied_size() const;

	int get_applied_width() const
	{
		return this->get_applied_size().width();
	}

	int get_applied_height() const
	{
		return this->get_applied_size().height();
	}

	QSize get_applied_size_with_dependent_template_offsets() const
	{
		QSize applied_size = this->get_applied_size();
		if (!this->is_optional()) {
			applied_size += QSize(this->GetDependentTemplatesWestOffset() + this->GetDependentTemplatesEastOffset(), this->GetDependentTemplatesNorthOffset() + this->GetDependentTemplatesSouthOffset());
		}
		return applied_size;
	}

	int get_area() const
	{
		return this->get_width() * this->get_height();
	}

	int get_applied_area_with_dependent_template_offsets() const
	{
		const QSize applied_size = this->get_applied_size_with_dependent_template_offsets();
		return applied_size.width() * applied_size.height();
	}

	int GetDependentTemplatesNorthOffset() const
	{
		int offset = 0;

		for (const map_template *map_template : this->get_main_template()->get_subtemplates()) {
			if (std::find(map_template->NorthOfTemplates.begin(), map_template->NorthOfTemplates.end(), this) == map_template->NorthOfTemplates.end()) {
				continue;
			}

			offset = std::max(offset, map_template::min_adjacent_template_distance.y() + map_template->get_height() + map_template->GetDependentTemplatesNorthOffset());
		}

		return offset;
	}

	int GetDependentTemplatesSouthOffset() const
	{
		int offset = 0;

		for (const map_template *map_template : this->get_main_template()->get_subtemplates()) {
			if (std::find(map_template->SouthOfTemplates.begin(), map_template->SouthOfTemplates.end(), this) == map_template->SouthOfTemplates.end()) {
				continue;
			}

			offset = std::max(offset, map_template::min_adjacent_template_distance.y() + map_template->get_height() + map_template->GetDependentTemplatesSouthOffset());
		}

		return offset;
	}

	int GetDependentTemplatesWestOffset() const
	{
		int offset = 0;

		for (const map_template *map_template : this->get_main_template()->get_subtemplates()) {
			if (std::find(map_template->WestOfTemplates.begin(), map_template->WestOfTemplates.end(), this) == map_template->WestOfTemplates.end()) {
				continue;
			}

			offset = std::max(offset, map_template::min_adjacent_template_distance.x() + map_template->get_width() + map_template->GetDependentTemplatesWestOffset());
		}

		return offset;
	}

	int GetDependentTemplatesEastOffset() const
	{
		int offset = 0;

		for (const map_template *map_template : this->get_main_template()->get_subtemplates()) {
			if (std::find(map_template->EastOfTemplates.begin(), map_template->EastOfTemplates.end(), this) == map_template->EastOfTemplates.end()) {
				continue;
			}

			offset = std::max(offset, map_template::min_adjacent_template_distance.x() + map_template->get_width() + map_template->GetDependentTemplatesEastOffset());
		}

		return offset;
	}

	bool is_circle() const
	{
		return this->circle;
	}

	bool is_optional() const
	{
		return this->optional;
	}

	bool is_optional_for_campaign(const campaign *campaign) const;

	bool is_constructed_only() const
	{
		return this->constructed_only;
	}

	bool is_tile_adjustment_enabled() const
	{
		return this->tile_adjustment_enabled;
	}

	bool contains_map_pos(const QPoint &pos) const
	{
		if (!this->is_on_map()) {
			return false;
		}

		const QPoint &start_pos = this->get_current_map_start_pos();
		const QPoint &end_pos = this->get_current_map_end_pos();
		return pos.x() >= start_pos.x() && pos.y() >= start_pos.y() && pos.x() <= end_pos.x() && pos.y() <= end_pos.y();
	}

	const QPoint &get_current_map_start_pos() const
	{
		return this->current_map_start_pos;
	}

	const QPoint &get_current_map_end_pos() const
	{
		return this->current_map_end_pos;
	}

	bool is_on_map() const
	{
		return this->get_current_map_start_pos() != QPoint(-1, -1);
	}

	//whether a position relative to the entire map is a usable part of the map template
	bool is_map_pos_usable(const QPoint &pos) const
	{
		if (!this->contains_map_pos(pos)) {
			return false;
		}

		if (!this->is_pos_usable(pos - this->get_current_map_start_pos())) {
			return false;
		}

		if (this->get_main_template() != nullptr) {
			return this->get_main_template()->is_map_pos_usable(pos);
		}

		return true;
	}

	//whether a position relative to the map template itself is a usable part of it
	bool is_pos_usable(const QPoint &pos) const;

	const wyrmgus::world *get_world() const
	{
		return this->world;
	}

	const wyrmgus::tileset *get_tileset() const
	{
		return this->tileset;
	}

	map_template *get_main_template() const
	{
		return this->main_template;
	}

	void set_main_template(map_template *map_template) 
	{
		if (map_template == this->get_main_template()) {
			return;
		}

		this->main_template = map_template;
		this->main_template->subtemplates.push_back(this);
	}

	const std::vector<map_template *> &get_subtemplates() const
	{
		return this->subtemplates;
	}

	bool is_subtemplate_of(const map_template *other) const
	{
		return this->get_main_template() == other;
	}

	bool is_any_subtemplate_of(const map_template *other) const
	{
		if (this->is_subtemplate_of(other)) {
			return true;
		}

		if (this->get_main_template() != nullptr) {
			return this->get_main_template()->is_any_subtemplate_of(other);
		}

		return false;
	}

	const QPoint &get_min_adjacent_template_distance() const
	{
		if (this->is_constructed_only()) {
			return map_template::min_constructed_adjacent_template_distance;
		} else {
			return map_template::min_adjacent_template_distance;
		}
	}

	void load_terrain_character_map(const bool overlay);

	void clear_terrain_character_maps()
	{
		this->terrain_character_map.clear();
		this->overlay_terrain_character_map.clear();
	}

	void do_character_substitutions(const bool overlay);

	const std::filesystem::path &get_terrain_file() const
	{
		return this->terrain_file;
	}

	void set_terrain_file(const std::filesystem::path &filepath);

	const std::filesystem::path &get_overlay_terrain_file() const
	{
		return this->overlay_terrain_file;
	}

	void set_overlay_terrain_file(const std::filesystem::path &filepath);

	const std::filesystem::path &get_trade_route_file() const
	{
		return this->trade_route_file;
	}

	void set_trade_route_file(const std::filesystem::path &filepath);

	void load_terrain(const bool overlay);
	void load_terrain_file(const bool overlay);
	void load_wesnoth_terrain_file();
	void load_0_ad_terrain_file();
	void load_freeciv_terrain_file();
	void load_stratagus_terrain_file();
	QImage load_terrain_image_file(const std::filesystem::path &filepath);
	void load_terrain_image(const bool overlay);
	void load_trade_route_image();

	void clear_terrain_images()
	{
		this->terrain_image = QImage();
		this->overlay_terrain_image = QImage();
	}

	const std::filesystem::path &get_territory_file() const
	{
		return this->territory_file;
	}

	void set_territory_file(const std::filesystem::path &filepath);

	const QPoint &get_subtemplate_top_left_pos() const
	{
		return this->subtemplate_top_left_pos;
	}

	const QPoint &get_subtemplate_center_pos() const
	{
		return this->subtemplate_center_pos;
	}

	const QPoint &get_min_subtemplate_pos() const
	{
		return this->min_subtemplate_pos;
	}

	const QPoint &get_max_subtemplate_pos() const
	{
		return this->max_subtemplate_pos;
	}

	const QPoint get_top_template_relative_pos() const
	{
		//get the top-left position of this map template relative to its top template, if any
		if (this->get_main_template() == nullptr) {
			return QPoint(0, 0);
		}

		return this->get_main_template()->get_top_template_relative_pos() + this->get_subtemplate_top_left_pos();
	}

	bool is_dependent_on(const map_template *other_template) const;
	void add_dependency_template(const map_template *other_template);

	size_t get_total_adjacent_template_count() const
	{
		//return the total number of adjacent templates on which this template ultimately depends
		size_t count = 0;
		for (const map_template *map_template : this->AdjacentTemplates) {
			count++;
			count += map_template->get_total_adjacent_template_count();
		}
		return count;
	}

	QPoint generate_subtemplate_position(map_template *subtemplate, const QPoint &template_start_pos, const QPoint &map_start_pos, const QPoint &map_end, int z, const QPoint &max_adjacent_template_distance, bool &adjacency_restriction_occurred) const;
	bool is_constructed_subtemplate_compatible_with_terrain(map_template *subtemplate, const QPoint &map_start_pos, int z) const;
	bool is_constructed_subtemplate_compatible_with_terrain_file(map_template *subtemplate, const QPoint &map_start_pos, int z) const;
	bool is_constructed_subtemplate_compatible_with_terrain_image(map_template *subtemplate, const QPoint &map_start_pos, int z) const;

	QPoint generate_celestial_site_position(const site *site, const int z) const;
	QPoint generate_site_orbit_position(const site *site, const int z, const int64_t orbit_distance) const;

	Vec2i get_best_location_map_position(const std::vector<std::unique_ptr<historical_location>> &historical_location_list, bool &in_another_map_template, const Vec2i &template_start_pos, const Vec2i &map_start_pos, const bool random) const;

	QPoint get_location_map_position(const historical_location *historical_location, const QPoint &template_start_pos, const QPoint &map_start_pos, const bool random) const;

	terrain_type *get_base_terrain_type() const
	{
		return this->base_terrain_type;
	}
	
	terrain_type *get_base_overlay_terrain_type() const
	{
		return this->base_overlay_terrain_type;
	}

	terrain_type *get_border_terrain_type() const
	{
		return this->border_terrain_type;
	}
	
	terrain_type *get_border_overlay_terrain_type() const
	{
		return this->border_overlay_terrain_type;
	}
	
	terrain_type *get_unusable_area_terrain_type() const
	{
		return this->unusable_area_terrain_type;
	}
	
	terrain_type *get_unusable_area_overlay_terrain_type() const
	{
		return this->unusable_area_overlay_terrain_type;
	}

	terrain_type *get_surrounding_terrain_type() const
	{
		return this->surrounding_terrain_type;
	}

	terrain_type *get_surrounding_overlay_terrain_type() const
	{
		return this->surrounding_overlay_terrain_type;
	}

	bool clears_path_to_other_subtemplates() const
	{
		return this->clear_path_to_other_subtemplates;
	}

	bool outputs_terrain_image() const
	{
		return this->output_terrain_image;
	}

	bool outputs_territory_image() const
	{
		return this->output_territory_image;
	}

	const point_map<const terrain_type *> &get_tile_terrains() const
	{
		return this->tile_terrains;
	}

	const point_map<const terrain_type *> &get_overlay_tile_terrains() const
	{
		return this->overlay_tile_terrains;
	}

	void set_tile_terrain(const QPoint &tile_pos, const terrain_type *terrain);
	void apply_tile_terrains(const bool overlay, const QPoint &template_start_pos, const QPoint &map_start_pos, const int z);

	const std::vector<std::unique_ptr<map_template_unit>> &get_units() const
	{
		return this->units;
	}

	const archimedes::map_projection *get_map_projection() const;

	const decimillesimal_int &get_min_longitude() const
	{
		return this->get_georectangle().get_min_longitude();
	}

	void set_min_longitude(const decimillesimal_int &lon)
	{
		this->georectangle.set_min_longitude(lon);
	}

	const decimillesimal_int &get_max_longitude() const
	{
		return this->get_georectangle().get_max_longitude();
	}

	void set_max_longitude(const decimillesimal_int &lon)
	{
		this->georectangle.set_max_longitude(lon);
	}

	const decimillesimal_int &get_min_latitude() const
	{
		return this->get_georectangle().get_min_latitude();
	}

	void set_min_latitude(const decimillesimal_int &lat)
	{
		this->georectangle.set_min_latitude(lat);
	}

	const decimillesimal_int &get_max_latitude() const
	{
		return this->get_georectangle().get_max_latitude();
	}

	void set_max_latitude(const decimillesimal_int &lat)
	{
		this->georectangle.set_max_latitude(lat);
	}

	const decimillesimal_int &get_astrodistance_multiplier() const
	{
		return this->astrodistance_multiplier;
	}

	int get_astrodistance_additive_modifier() const
	{
		return this->astrodistance_additive_modifier;
	}

	const map_template *get_default_astrocoordinate_reference_subtemplate() const
	{
		return this->default_astrocoordinate_reference_subtemplate;
	}

	const decimillesimal_int &get_orbit_distance_multiplier() const
	{
		return this->orbit_distance_multiplier;
	}

	const archimedes::georectangle &get_georectangle() const
	{
		return this->georectangle;
	}

	QPoint get_geocoordinate_pos(const geocoordinate &geocoordinate) const;
	geocoordinate get_pos_geocoordinate(const QPoint &pos) const;

	void save_terrain_images();
	void save_terrain_image(const std::string &filename, const QImage &loaded_terrain_image, const terrain_geodata_ptr_map &terrain_data, const point_map<const terrain_type *> &terrain_map) const;
	void create_terrain_image_from_geodata(QImage &image, const terrain_geodata_ptr_map &terrain_data, const std::string &image_checkpoint_save_filename) const;
	void create_terrain_image_from_map(QImage &image, const point_map<const terrain_type *> &terrain_map) const;
	void save_territory_image(const std::string &filename, site_map<std::vector<std::unique_ptr<QGeoShape>>> &&territory_data) const;

	QPoint pos_to_map_pos(const QPoint &pos) const
	{
		if (!this->is_on_map()) {
			throw std::runtime_error("Canot convert pos to map pos for map template \"" + this->get_identifier() + "\", as it is not on the map.");
		}

		return this->get_current_map_start_pos() + pos - this->get_start_pos();
	}

	bool is_active_by_default() const
	{
		return !this->is_constructed_only();
	}

	const character_unit *get_character_unit(const char character) const
	{
		const auto find_iterator = this->character_units.find(character);
		if (find_iterator != this->character_units.end()) {
			return find_iterator->second.get();
		}

		return nullptr;
	}

	void add_site(const site *site);

	const terrain_type *get_default_base_terrain(const terrain_type *overlay_terrain) const;

	bool is_dungeon() const
	{
		return this->dungeon_generation != nullptr;
	}
	
private:
	terrain_character_map_type terrain_character_map;
	terrain_character_map_type overlay_terrain_character_map;
	std::filesystem::path terrain_file;
	QImage terrain_image;
	std::filesystem::path overlay_terrain_file;
	QImage overlay_terrain_image;
	std::filesystem::path trade_route_file;
	std::filesystem::path territory_file;
	QSize size = QSize(0, 0);
	int scale_multiplier = 1; //used for Wesnoth maps
	int scale_divisor = 1; //used for 0 AD maps
public:
	int Priority = 100; //the priority of this map template, for the order of application of subtemplates
private:
	bool circle = false; //whether the template should be applied as a circle, i.e. it should apply no subtemplates and etc. or generate terrain outside the boundaries of the circle
	bool optional = false;
	bool constructed_only = false;
	bool tile_adjustment_enabled = true;
	bool output_terrain_image = false;
	bool output_territory_image = false;
private:
	QPoint subtemplate_top_left_pos = QPoint(-1, -1); //this template's position as a subtemplate in its main template; the position is relative to the subtemplate's top left
	QPoint subtemplate_center_pos = QPoint(-1, -1); //this template's position as a subtemplate in its main template; the position is relative to the subtemplate's center
	QPoint min_subtemplate_pos = QPoint(-1, -1); //the minimum position this subtemplate can be applied to in its main template
	QPoint max_subtemplate_pos = QPoint(-1, -1); //the maximum position this subtemplate can be applied to in its main template
	archimedes::geocoordinate min_subtemplate_geocoordinate;
	archimedes::geocoordinate max_subtemplate_geocoordinate;
	QPoint start_pos = QPoint(0, 0); //the start position within the map template to be applied when it is used
	QPoint end_pos = QPoint(-1, -1); //the end position within the map template to be applied when it is used
private:
	QPoint current_map_start_pos = QPoint(-1, -1);
	QPoint current_map_end_pos = QPoint(-1, -1);
public:
	QPoint current_start_pos = QPoint(0, 0);
	map_template *main_template = nullptr; //main template in which this one is located, if this is a subtemplate
	map_template *UpperTemplate = nullptr; //map template corresponding to this one in the upper layer
	map_template *LowerTemplate = nullptr; //map template corresponding to this one in the lower layer
	std::vector<const map_template *> AdjacentTemplates; //map templates adjacent to this one
private:
	std::vector<map_template *> dependent_adjacent_templates;
public:
	std::vector<const map_template *> NorthOfTemplates; //map templates to which this one is to the north of
	std::vector<const map_template *> SouthOfTemplates; //map templates to which this one is to the north of
	std::vector<const map_template *> WestOfTemplates; //map templates to which this one is to the west of
	std::vector<const map_template *> EastOfTemplates; //map templates to which this one is to the east of
private:
	std::vector<const map_template *> dependency_templates; //the other templates on which this one depends on to be applied as a subtemplate, e.g. its adjacent templates, north of templates, etc.
	wyrmgus::world *world = nullptr;
	wyrmgus::tileset *tileset = nullptr;
	terrain_type *base_terrain_type = nullptr;
	terrain_type *base_overlay_terrain_type = nullptr;
	terrain_type *border_terrain_type = nullptr;
	terrain_type *border_overlay_terrain_type = nullptr;
	terrain_type *surrounding_terrain_type = nullptr;
	terrain_type *surrounding_overlay_terrain_type = nullptr;
	terrain_type *unusable_area_terrain_type = nullptr; //the terrain type for the template's unusable area, e.g. the area outside its circle if the template is a circle
	terrain_type *unusable_area_overlay_terrain_type = nullptr;
	terrain_type *default_water_terrain_type = nullptr;
	bool clear_terrain = false; //whether to clear terrain in the map template's area before applying
	bool clear_path_to_other_subtemplates = false; //whether to clear a path to other subtemplates from this subtemplate's clear edge points
	std::vector<map_template *> subtemplates;
	std::vector<std::unique_ptr<generated_terrain>> generated_terrains; //terrains generated in the map template
	std::vector<const site *> generated_sites;
	std::vector<const region *> generated_site_regions;
public:
	std::vector<std::pair<unit_type *, int>> GeneratedNeutralUnits; /// the first element of the pair is the resource's unit type, and the second is the quantity
	std::vector<std::pair<unit_type *, int>> PlayerLocationGeneratedNeutralUnits;
	std::map<std::pair<int, int>, std::tuple<unit_type *, int, unique_item *>> Resources; /// Resources (with unit type, resources held, and unique item pointer), mapped to the tile position
	std::vector<std::tuple<Vec2i, unit_type *, faction *, CDate, CDate, unique_item *>> Units; /// Units; first value is the tile position, and the last ones are start date and end date
	std::vector<std::tuple<Vec2i, character *, faction *, CDate, CDate>> Heroes; /// Heroes; first value is the tile position, and the last ones are start year and end year
	std::vector<std::tuple<Vec2i, unit_type *, wyrmgus::world *, unique_item *>> WorldConnectors; /// Layer connectors (with unit type, world pointer, and unique item pointer), mapped to the tile position
	std::map<std::pair<int, int>, std::string> TileLabels; /// labels to appear for certain tiles
private:
	std::vector<const site *> sites;
	point_map<const site *> sites_by_position;
	std::map<geocoordinate, const site *> sites_by_geocoordinate;
	std::map<geocoordinate, const site *> sites_by_astrocoordinate;
	std::vector<const site *> created_settlements; //sites for settlement sites which are created without a predefined site (e.g. when a map file details a settlement head without a settlement being provided); the settlements will be set to each settlement site in order
	bool random_created_settlements = false;
	bool create_starting_mine = false; //create a mine on the nearest copper deposit for each player
	point_map<const terrain_type *> tile_terrains;
	point_map<const terrain_type *> overlay_tile_terrains;
public:
	std::vector<std::tuple<Vec2i, terrain_type *, CDate>> HistoricalTerrains; //terrain changes
private:
	std::vector<std::unique_ptr<map_template_unit>> units;
	archimedes::map_projection *map_projection = nullptr;
	archimedes::georectangle georectangle;
	decimillesimal_int astrodistance_multiplier = decimillesimal_int(1);
	int astrodistance_additive_modifier = 0;
	map_template *default_astrocoordinate_reference_subtemplate = nullptr;
	decimillesimal_int orbit_distance_multiplier = decimillesimal_int(1);
	std::map<const terrain_type *, const terrain_type *> terrain_substitutions;
	std::map<const terrain_type *, const terrain_type *> default_base_terrains;
	std::map<char, std::unique_ptr<character_unit>> character_units;
	std::vector<std::unique_ptr<character_substitution>> character_substitutions; //substitutions applied to the terrain character map, in order
	qunique_ptr<dungeon_generation_settings> dungeon_generation;
	std::vector<const faction *> generated_factions;
	std::vector<const terrain_feature *> generated_terrain_features;
	std::unique_ptr<map_template_history> history;

	friend int ::CclDefineMapTemplate(lua_State *l);
};

}
