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
//      (c) Copyright 2019-2022 by Andrettin
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

#include "database/preferences.h"

#include "database/database.h"
#include "database/defines.h"
#include "database/gsml_data.h"
#include "database/gsml_parser.h"
#include "game/difficulty.h"
#include "sound/music_player.h"
#include "sound/sound_server.h"
#include "translator.h"
#include "ui/cursor.h"
#include "ui/hotkey_setup.h"
#include "util/exception_util.h"
#include "util/log_util.h"
#include "util/string_conversion_util.h"
#include "util/string_util.h"
#include "video/video.h"

namespace wyrmgus {

std::filesystem::path preferences::get_path()
{
	return database::get_user_data_path() / "preferences.txt";
}

std::filesystem::path preferences::get_fallback_path()
{
	return database::get_documents_path() / "preferences.txt";
}

preferences::preferences() : difficulty(difficulty::normal), hotkey_setup(hotkey_setup::default_setup)
{
}

void preferences::load()
{
	this->load_file();
	this->initialize();
}

void preferences::load_file()
{
	std::filesystem::path preferences_path = preferences::get_path();

	if (!std::filesystem::exists(preferences_path)) {
		preferences_path = preferences::get_fallback_path();

		if (!std::filesystem::exists(preferences_path)) {
			return;
		}
	}

	gsml_parser parser;
	gsml_data data;

	try {
		data = parser.parse(preferences_path);
	} catch (const std::exception &exception) {
		exception::report(exception);
		log::log_error("Failed to parse preferences file.");
	}

	database::process_gsml_data(this, data);
}

void preferences::save() const
{
	const std::filesystem::path preferences_path = preferences::get_path();

	gsml_data data(preferences_path.filename().stem().string());

	if (!this->get_locale().empty()) {
		data.add_property("locale", "\"" + this->get_locale() + "\"");
	}

	data.add_property("scale_factor", this->get_scale_factor().to_string());
	data.add_property("fullscreen", string::from_bool(this->is_fullscreen()));
	data.add_child(gsml_data::from_size(this->get_window_size(), "window_size"));
	data.add_property("window_maximized", string::from_bool(this->window_maximized));
	data.add_property("game_speed", std::to_string(this->get_game_speed()));
	data.add_property("difficulty", enum_converter<wyrmgus::difficulty>::to_string(this->get_difficulty()));
	data.add_property("sound_effects_enabled", string::from_bool(this->are_sound_effects_enabled()));
	data.add_property("sound_effects_volume", std::to_string(this->get_sound_effects_volume()));
	data.add_property("music_enabled", string::from_bool(this->is_music_enabled()));
	data.add_property("music_volume", std::to_string(this->get_music_volume()));
	data.add_property("hotkey_setup", enum_converter<wyrmgus::hotkey_setup>::to_string(this->get_hotkey_setup()));
	data.add_property("autosave", string::from_bool(this->is_autosave_enabled()));
	data.add_property("hero_symbol", string::from_bool(this->is_hero_symbol_enabled()));
	data.add_property("pathlines", string::from_bool(this->are_pathlines_enabled()));
	data.add_property("player_color_circle", string::from_bool(this->is_player_color_circle_enabled()));
	data.add_property("hp_bar", string::from_bool(this->is_hp_bar_enabled()));
	data.add_property("resource_bar", string::from_bool(this->is_resource_bar_enabled()));
	data.add_property("show_hotkeys", string::from_bool(this->is_show_hotkeys_enabled()));
	data.add_property("show_messages", string::from_bool(this->is_show_messages_enabled()));
	data.add_property("show_tips", string::from_bool(this->is_show_tips_enabled()));
	data.add_property("key_scroll_speed", std::to_string(this->get_key_scroll_speed()));
	data.add_property("mouse_scroll_speed", std::to_string(this->get_mouse_scroll_speed()));
	data.add_property("reverse_mousewheel_scrolling", string::from_bool(this->is_reverse_mousewheel_scrolling_enabled()));

	if (!this->get_local_player_name().empty()) {
		data.add_property("local_player_name", "\"" + string::escaped(this->get_local_player_name()) + "\"");
	}

	try {
		data.print_to_file(preferences_path);
	} catch (const std::exception &exception) {
		exception::report(exception);
		log::log_error("Failed to save preferences file.");
	}
}

void preferences::process_gsml_property(const gsml_property &property)
{
	//use a try-catch for the properties, as the property or its value could no longer exist
	try {
		database::get()->process_gsml_property_for_object(this, property);
	} catch (const std::runtime_error &exception) {
		exception::report(exception);
	}
}

void preferences::process_gsml_scope(const gsml_data &scope)
{
	try {
		database::get()->process_gsml_scope_for_object(this, scope);
	} catch (const std::runtime_error &exception) {
		exception::report(exception);
	}
}

void preferences::initialize()
{
	if (this->get_local_player_name().empty()) {
		this->set_local_player_name_from_env();
	}

	this->update_video_sync_speed();
}

void preferences::set_locale(const std::string &locale)
{
	if (locale == this->get_locale()) {
		return;
	}

	this->locale = locale;
	translator::get()->set_locale(locale);
}

void preferences::set_scale_factor(const centesimal_int &factor)
{
	if (factor == this->get_scale_factor()) {
		return;
	}

	this->scale_factor = factor;

	CGraphic::unload_all();
	cursor::on_current_cursor_changed();

	emit scale_factor_changed();
}

void preferences::update_video_sync_speed()
{
	VideoSyncSpeed = this->get_game_speed() * 100 / CYCLES_PER_SECOND;
	SetVideoSync();
}

void preferences::set_sound_effects_volume(int volume)
{
	volume = std::clamp(volume, 0, MaxVolume);

	if (volume == this->get_sound_effects_volume()) {
		return;
	}

	this->sound_effects_volume = volume;
	emit sound_effects_volume_changed();
}

void preferences::set_music_enabled(const bool enabled)
{
	if (enabled == this->is_music_enabled()) {
		return;
	}

	this->music_enabled = enabled;

	if (enabled) {
		music_player::get()->play();
	} else {
		music_player::get()->stop();
	}

	emit music_enabled_changed();
}

void preferences::set_music_volume(int volume)
{
	volume = std::clamp(volume, 0, MaxVolume);

	if (volume == this->get_music_volume()) {
		return;
	}

	this->music_volume = volume;
	emit music_volume_changed();
}

void preferences::set_local_player_name_from_env()
{
	const char *userName = nullptr;

#ifdef USE_WIN32
	userName = getenv("USERNAME");
#else
	userName = getenv("USER");
#endif

	if (userName != nullptr && userName[0]) {
		this->set_local_player_name(userName);
	} else {
		this->set_local_player_name("Anonymous");
	}
}

}

bool is_show_tips_enabled()
{
	return preferences::get()->is_show_tips_enabled();
}
