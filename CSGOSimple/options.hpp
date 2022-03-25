#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include "valve_sdk/Misc/Color.hpp"
#include <map>
#include <set>

#define A( s ) #s
#define OPTION(type, var, val) type var = val

struct weapon_name_t;
struct weapon_info_t;

extern std::map<int, const char*> K_weapon_names;
extern const std::map<size_t, weapon_info_t> WeaponInfo;
extern const std::vector<weapon_name_t> K_KnifeNames;
extern const std::vector<weapon_name_t> K_GloveNames;
extern std::vector<weapon_name_t> WeaponNamesFull;

struct aimbot_settings {
	bool enabled = false;
	bool deathmatch = false;
	bool autopistol = false;
	bool check_smoke = false;
	bool check_flash = false;
	bool autowall = false;
	bool silent = false;
	bool rcs = false;
	bool rcs_fov_enabled = false;
	bool rcs_smooth_enabled = false;
	bool humanize = false;
	struct {
		bool enabled = false;
		int ticks = 6;
	} backtrack;
	bool only_in_zoom = true;
	int aim_type = 1;
	int priority = 0;
	int fov_type = 0;
	int rcs_type = 0;
	int hitbox = 1;
	float fov = 0.f;
	float silent_fov = 0.f;
	float rcs_fov = 0.f;
	float smooth = 1;
	float rcs_smooth = 1;
	int shot_delay = 0;
	int kill_delay = 0;
	int rcs_x = 100;
	int rcs_y = 100;
	int rcs_start = 1;
	int min_damage = 1;
};
struct weapon_name_t {
	constexpr weapon_name_t(int32_t definition_index, const char* name) :
		definition_index(definition_index),
		name(name) {
	}

	int32_t definition_index = 0;
	const char* name = nullptr;
};
struct weapon_info_t {
	constexpr weapon_info_t(const char* model, const char* icon = nullptr) :
		model(model),
		icon(icon)
	{}

	const char* model;
	const char* icon;
};

struct item_setting {
	char name[32] = "";
	int definition_vector_index = 0;
	int definition_index = 1;
	int paint_kit_vector_index = 0;
	int paint_kit_index = 0;
	int definition_override_vector_index = 0;
	int definition_override_index = 0;
	int seed = 0;
	bool enable_stat_track = false;
	int stat_trak = 0;
	float wear = 0.0f;
};
struct skinInfo {
	int seed = -1;
	int paintkit;
	int rarity;
	std::string tagName;
	std::string     shortname; // shortname
	std::string     name;      // full skin name
};

class Options
{
public:
		// 
		// ESP
		// 
		OPTION(bool, esp_enabled, false);
		OPTION(bool, esp_enemies_only, false);
		OPTION(bool, esp_dormant_esp, false);
		OPTION(bool, esp_visible_only, false);
		OPTION(bool, esp_player_boxes, false);
		OPTION(bool, esp_player_names, false);
		OPTION(bool, esp_player_health, false);
		OPTION(bool, esp_player_armour, false);
		OPTION(bool, esp_player_weapons, false);
		OPTION(bool, esp_player_snaplines, false);
		OPTION(bool, esp_player_flags, false);
		OPTION(bool, esp_player_flags_use_icons, false);
		OPTION(bool, esp_player_flags_armor, false);
		OPTION(bool, esp_player_flags_scoped, false);
		OPTION(bool, esp_player_flags_flashed, false);
		OPTION(bool, esp_player_flags_was_dormant, false);
		OPTION(bool, esp_player_flags_bomb_kits, false);
		OPTION(bool, esp_player_flags_have_taser, false);
		OPTION(bool, esp_crosshair, false);
		OPTION(bool, esp_dropped_weapons, false);
		OPTION(bool, esp_defuse_kit, false);
		OPTION(bool, esp_planted_c4, false);
		OPTION(bool, esp_items, false);
		OPTION(bool, esp_spectators, false);

		// 
		// GLOW
		// 
		OPTION(bool, glow_enabled, false);
		OPTION(bool, glow_enemies_only, false);
		OPTION(bool, glow_players, false);
		OPTION(bool, glow_chickens, false);
		OPTION(bool, glow_c4_carrier, false);
		OPTION(bool, glow_planted_c4, false);
		OPTION(bool, glow_defuse_kits, false);
		OPTION(bool, glow_weapons, false);

		//
		// CHAMS
		//
		OPTION(bool, chams_player_enabled, false);
		OPTION(bool, chams_player_enemies_only, false);
		OPTION(bool, chams_player_behind_wall, false);
		OPTION(bool, chams_player_overlay, false);
		OPTION(int, chams_player_type, 0);

		OPTION(bool, chams_arms_enabled, false);
		OPTION(int, chams_arms_type, 0);

		//
		// MISC
		//
		OPTION(bool, misc_bhop, false);
		OPTION(bool, misc_no_hands, false);
		OPTION(bool, misc_thirdperson, false);
		OPTION(bool, misc_showranks, true);
		OPTION(bool, misc_watermark, true);
		OPTION(float, misc_thirdperson_dist, 50.f);
		OPTION(int, viewmodel_fov, 68);
		OPTION(float, mat_ambient_light_r, 0.0f);
		OPTION(float, mat_ambient_light_g, 0.0f);
		OPTION(float, mat_ambient_light_b, 0.0f);
		OPTION(bool, misc_autostrafe, false);
		OPTION(int, misc_autostrafe_type, 0);
		OPTION(int, misc_autostrafe_retrack, 0);
		OPTION(bool, misc_antiuntrusted, true);
		OPTION(bool, misc_fakelag_enabled, false);
		OPTION(bool, misc_fakelag_standing, false);
		OPTION(bool, misc_fakelag_moving, false);
		OPTION(bool, misc_fakelag_unducking, false);
		OPTION(int, misc_fakelag_mode, 0);
		OPTION(int, misc_fakelag_factor, 0);
		OPTION(bool, misc_noduckcooldown, false);
		OPTION(bool, misc_desync, false);
		OPTION(int, misc_desync_key, 0);
		OPTION(bool, misc_edgejump, false);
		OPTION(int, misc_edgejump_key, 0);
		OPTION(bool, misc_customconsole, false);

		// 
		// COLORS
		// 
		float color_esp_ally_visible[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_esp_enemy_visible[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_esp_ally_occluded[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_esp_enemy_occluded[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_esp_weapons[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_esp_defuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_esp_c4[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_esp_item[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_esp_crosshair[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		float color_esp_box[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_esp_names[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_esp_weapon[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_esp_snaplines[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		float color_glow_ally[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_glow_enemy[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_glow_chickens[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_glow_c4_carrier[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_glow_planted_c4[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_glow_defuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_glow_weapons[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		float color_chams_player_ally_visible[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_chams_player_ally_occluded[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_chams_player_enemy_visible[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_chams_player_enemy_occluded[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		float color_chams_arms[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		float color_misc_customconsole[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		OPTION(Color, color_watermark, Color(0, 128, 255)); // no menu config cuz its useless

		std::map<int, aimbot_settings> aimbot = {};
		std::unordered_map<std::string, std::set<std::string>> weaponSkins;
		std::unordered_map<std::string, skinInfo> skinMap;
		std::unordered_map<std::string, std::string> skinNames;
		std::map<int, item_setting> Items;
		std::unordered_map<std::string, std::string> IconOverrides;
};

extern Options g_Options;
extern bool   g_Unload;
