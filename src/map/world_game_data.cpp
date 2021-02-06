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
//      (c) Copyright 2021 by Andrettin
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

#include "map/world_game_data.h"

#include "database/sml_data.h"
#include "map/map.h"
#include "map/map_layer.h"
#include "map/minimap.h"
#include "map/terrain_type.h"
#include "map/tile.h"
#include "map/world.h"
#include "time/season_schedule.h"
#include "time/time_of_day.h"
#include "time/time_of_day_schedule.h"
#include "unit/unit.h"
#include "unit/unit_find.h"

namespace wyrmgus {

void world_game_data::do_per_in_game_hour_loop()
{
	this->decrement_remaining_season_hours();
	this->decrement_remaining_time_of_day_hours();
}

void world_game_data::decrement_remaining_time_of_day_hours()
{
	if (this->world->get_time_of_day_schedule() == nullptr) {
		return;
	}

	this->remaining_time_of_day_hours -= this->world->get_time_of_day_schedule()->HourMultiplier;

	if (this->remaining_time_of_day_hours <= 0) {
		this->increment_time_of_day();
	}
}

void world_game_data::increment_time_of_day()
{
	unsigned current_time_of_day_id = this->time_of_day->ID;
	current_time_of_day_id++;
	if (current_time_of_day_id >= this->world->get_time_of_day_schedule()->ScheduledTimesOfDay.size()) {
		current_time_of_day_id = 0;
	}

	this->set_time_of_day(this->world->get_time_of_day_schedule()->ScheduledTimesOfDay[current_time_of_day_id]);
	this->remaining_time_of_day_hours += this->time_of_day->GetHours(this->get_season());
}

/**
**	@brief	Set the time of day corresponding to an amount of hours
*/
void world_game_data::set_time_of_day_by_hours(const unsigned long long hours)
{
	if (!this->world->get_time_of_day_schedule()) {
		return;
	}

	int remaining_hours = hours % this->world->get_time_of_day_schedule()->get_total_hours();
	this->set_time_of_day(this->world->get_time_of_day_schedule()->ScheduledTimesOfDay.front());
	this->remaining_time_of_day_hours = this->time_of_day->GetHours(this->get_season());
	this->remaining_time_of_day_hours -= remaining_hours;

	while (this->remaining_time_of_day_hours <= 0) {
		this->increment_time_of_day();
	}
}

void world_game_data::set_time_of_day(const scheduled_time_of_day *time_of_day)
{
	if (this->time_of_day == time_of_day) {
		return;
	}

	const scheduled_time_of_day *old_time_of_day = this->time_of_day;
	this->time_of_day = time_of_day;

	const bool is_day_changed = (this->time_of_day && this->time_of_day->TimeOfDay->is_day()) != (old_time_of_day && old_time_of_day->TimeOfDay->is_day());
	const bool is_night_changed = (this->time_of_day && this->time_of_day->TimeOfDay->is_night()) != (old_time_of_day && old_time_of_day->TimeOfDay->is_night());

	//update the sight of all units
	if (is_day_changed || is_night_changed) {
		std::vector<CUnit *> units;
		Select(this->map_rect.topLeft(), this->map_rect.bottomRight(), units, map_layer->ID);

		for (CUnit *unit : units) {
			const tile *center_tile = unit->get_center_tile();
			if (center_tile != nullptr && center_tile->get_world() != this->world) {
				continue;
			}

			if (
				unit->IsAlive()
				&& (
					//if has day sight bonus and is entering or exiting day
					(is_day_changed && unit->Variable[DAYSIGHTRANGEBONUS_INDEX].Value != 0)
					//if has night sight bonus and is entering or exiting night
					|| (is_night_changed && unit->Variable[NIGHTSIGHTRANGEBONUS_INDEX].Value != 0)
					)
				) {
				MapUnmarkUnitSight(*unit);
				UpdateUnitSightRange(*unit);
				MapMarkUnitSight(*unit);
			}
		}
	}
}

const time_of_day *world_game_data::get_time_of_day() const
{
	if (this->time_of_day == nullptr) {
		return nullptr;
	}

	return this->time_of_day->TimeOfDay;
}

void world_game_data::decrement_remaining_season_hours()
{
	if (!this->world->get_season_schedule()) {
		return;
	}

	this->remaining_season_hours -= this->world->get_season_schedule()->HourMultiplier;

	if (this->remaining_season_hours <= 0) {
		this->increment_season();
	}
}

void world_game_data::increment_season()
{
	unsigned current_season_id = this->season->ID;
	current_season_id++;
	if (current_season_id >= this->world->get_season_schedule()->ScheduledSeasons.size()) {
		current_season_id = 0;
	}

	this->set_season(this->world->get_season_schedule()->ScheduledSeasons[current_season_id]);
	this->remaining_season_hours += this->season->Hours;
}

/**
**	@brief	Set the season corresponding to an amount of hours
**
**	@param	hours	The quantity of hours
*/
void world_game_data::set_season_by_hours(const unsigned long long hours)
{
	if (!this->world->get_season_schedule()) {
		return;
	}

	int remaining_hours = hours % this->world->get_season_schedule()->get_total_hours();
	this->set_season(this->world->get_season_schedule()->ScheduledSeasons.front());
	this->remaining_season_hours = this->season->Hours;
	this->remaining_season_hours -= remaining_hours;

	while (this->remaining_season_hours <= 0) {
		this->increment_season();
	}
}

void world_game_data::set_season(const scheduled_season *season)
{
	if (season == this->season) {
		return;
	}

	wyrmgus::season *old_season = this->season ? this->season->Season : nullptr;
	wyrmgus::season *new_season = season ? season->Season : nullptr;

	this->season = season;

	//update world tiles affected by the season change
	for (int x = this->map_rect.x(); x <= this->map_rect.right(); ++x) {
		for (int y = this->map_rect.y(); y <= this->map_rect.bottom(); ++y) {
			const QPoint tile_pos(x, y);
			const tile *tile = this->map_layer->Field(tile_pos);

			if (tile->get_world() != this->world) {
				continue;
			}

			//check if the tile's terrain graphics have changed due to the new season and if so, update the minimap
			if (
				(tile->player_info->SeenTerrain && tile->player_info->SeenTerrain->get_graphics(old_season) != tile->player_info->SeenTerrain->get_graphics(new_season))
				|| (tile->player_info->SeenOverlayTerrain && tile->player_info->SeenOverlayTerrain->get_graphics(old_season) != tile->player_info->SeenOverlayTerrain->get_graphics(new_season))
			) {
				UI.get_minimap()->UpdateXY(tile_pos, this->map_layer->ID);
			}
		}
	}

	//update units which may have had their variation become invalid due to the season change
	std::vector<CUnit *> units;
	Select(this->map_rect.topLeft(), this->map_rect.bottomRight(), units, map_layer->ID);

	for (CUnit *unit : units) {
		const tile *center_tile = unit->get_center_tile();
		if (center_tile != nullptr && center_tile->get_world() != this->world) {
			continue;
		}

		if (unit->IsAlive()) {
			const wyrmgus::unit_type_variation *variation = unit->GetVariation();
			if (variation != nullptr && !unit->can_have_variation(variation)) {
				unit->ChooseVariation(); //choose a new variation, as the old one has become invalid due to the season change
			}
		}
	}
}

const wyrmgus::season *world_game_data::get_season() const
{
	if (!this->season) {
		return nullptr;
	}

	return this->season->Season;
}

}
