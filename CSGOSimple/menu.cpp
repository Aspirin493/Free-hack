#include "Menu.hpp"
#define NOMINMAX
#include <Windows.h>
#include <chrono>

#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "options.hpp"
#include "ui.hpp"
#include "config.hpp"

#include <d3d9.h>
#include <d3dx9.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui_internal.h"
#include "imgui/impl/imgui_impl_dx9.h"
#include "imgui/impl/imgui_impl_win32.h"

#include <filesystem>
#define fs std::filesystem


// =========================================================
// 
// These are the tabs on the sidebar
// 
// =========================================================
static char* sidebar_tabs[] = {
    "ESP",
    "AIM",
    "MISC",
    "CONFIG"
};

static int tab;
static int subtab;
LPDIRECT3DTEXTURE9 m_skin_texture = nullptr;

constexpr static float get_sidebar_item_width() { return 150.0f; }
constexpr static float get_sidebar_item_height() { return  50.0f; }

enum {
	TAB_ESP,
	TAB_AIMBOT,
	TAB_MISC,
	TAB_CONFIG
};

namespace ImGuiEx
{
    inline bool ColorEdit4(const char* label, float* v, bool first = true, bool show_alpha = true)
    {
        if (!first)
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 5);
        }
        else
        {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 7);
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);
        if (ImGui::ColorEdit4(label, v, show_alpha | ImGuiColorEditFlags_NoInputs)) {
            ImGui::PopStyleVar();
            return true;
        }
        ImGui::PopStyleVar();
        return false;
    }
    inline bool ColorEdit3(const char* label, float* v)
    {
        return ColorEdit4(label, v, false);
    }
}

template<size_t N>
void render_tabs(char* (&names)[N], int& activetab, float w, float h, bool sameline)
{
    bool values[N] = { false };

    values[activetab] = true;

    for(auto i = 0; i < N; ++i) {
        if(ImGui::ToggleButton(names[i], &values[i], ImVec2{ w, h })) {
            activetab = i;
        }
        if(sameline && i < N - 1)
            ImGui::SameLine();
    }
}

ImVec2 get_sidebar_size()
{
    constexpr float padding = 10.0f;
    constexpr auto size_w = padding * 2.0f + get_sidebar_item_width();
    constexpr auto size_h = padding * 2.0f + (sizeof(sidebar_tabs) / sizeof(char*)) * get_sidebar_item_height();

    return ImVec2{ size_w, ImMax(325.0f, size_h) };
}

int get_fps()
{
    using namespace std::chrono;
    static int count = 0;
    static auto last = high_resolution_clock::now();
    auto now = high_resolution_clock::now();
    static int fps = 0;

    count++;

    if(duration_cast<milliseconds>(now - last).count() > 1000) {
        fps = count;
        count = 0;
        last = now;
    }

    return fps;
}

void RenderEspTab()
{
    static char* esp_tab_names[] = { "ESP", "GLOW", "CHAMS" };
    static int   active_esp_tab = 0;

    bool placeholder_true = true;

    auto& style = ImGui::GetStyle();
    float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    {
        render_tabs(esp_tab_names, active_esp_tab, group_w / _countof(esp_tab_names), 25.0f, true);
    }
    ImGui::PopStyleVar();
    ImGui::BeginGroupBox("##body_content");
    {
        if(active_esp_tab == 0) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
            ImGui::Columns(3, nullptr, false);
            ImGui::SetColumnOffset(1, group_w / 3.0f);
            ImGui::SetColumnOffset(2, 2 * group_w / 3.0f);
            ImGui::SetColumnOffset(3, group_w);

            ImGui::Checkbox("Enabled", &g_Options.esp_enabled);
            ImGui::Checkbox("Team check", &g_Options.esp_enemies_only);
            ImGui::Checkbox("Boxes", &g_Options.esp_player_boxes);
            ImGui::Checkbox("Names", &g_Options.esp_player_names);
            ImGui::Checkbox("Health", &g_Options.esp_player_health);
            ImGui::Checkbox("Armour", &g_Options.esp_player_armour);
            ImGui::Checkbox("Weapon", &g_Options.esp_player_weapons);
            ImGui::Checkbox("Snaplines", &g_Options.esp_player_snaplines);

            ImGui::NextColumn();

            ImGui::Checkbox("Crosshair", &g_Options.esp_crosshair);
            ImGui::Checkbox("Dropped Weapons", &g_Options.esp_dropped_weapons);
            ImGui::Checkbox("Defuse Kit", &g_Options.esp_defuse_kit);
            ImGui::Checkbox("Planted C4", &g_Options.esp_planted_c4);
			ImGui::Checkbox("Item Esp", &g_Options.esp_items);

            ImGui::NextColumn();

            ImGui::PushItemWidth(100);
            ImGuiEx::ColorEdit4("Allies Visible", g_Options.color_esp_ally_visible);
            ImGuiEx::ColorEdit4("Enemies Visible", g_Options.color_esp_enemy_visible);
            ImGuiEx::ColorEdit4("Allies Occluded", g_Options.color_esp_ally_occluded);
            ImGuiEx::ColorEdit4("Enemies Occluded", g_Options.color_esp_enemy_occluded);
            ImGuiEx::ColorEdit4("Crosshair", g_Options.color_esp_crosshair);
            ImGuiEx::ColorEdit4("Dropped Weapons", g_Options.color_esp_weapons);
            ImGuiEx::ColorEdit4("Defuse Kit", g_Options.color_esp_defuse);
            ImGuiEx::ColorEdit4("Planted C4", g_Options.color_esp_c4);
			ImGuiEx::ColorEdit4("Item Esp", g_Options.color_esp_item);
            ImGui::PopItemWidth();

            ImGui::Columns(1, nullptr, false);
            ImGui::PopStyleVar();
        } else if(active_esp_tab == 1) {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
            ImGui::Columns(3, nullptr, false);
            ImGui::SetColumnOffset(1, group_w / 3.0f);
            ImGui::SetColumnOffset(2, 2 * group_w / 3.0f);
            ImGui::SetColumnOffset(3, group_w);

            ImGui::Checkbox("Enabled", &g_Options.glow_enabled);
            ImGui::Checkbox("Team check", &g_Options.glow_enemies_only);
            ImGui::Checkbox("Players", &g_Options.glow_players);
            ImGui::Checkbox("Chickens", &g_Options.glow_chickens);
            ImGui::Checkbox("C4 Carrier", &g_Options.glow_c4_carrier);
            ImGui::Checkbox("Planted C4", &g_Options.glow_planted_c4);
            ImGui::Checkbox("Defuse Kits", &g_Options.glow_defuse_kits);
            ImGui::Checkbox("Weapons", &g_Options.glow_weapons);

            ImGui::NextColumn();

            ImGui::PushItemWidth(100);
            ImGuiEx::ColorEdit4("Ally", g_Options.color_glow_ally);
            ImGuiEx::ColorEdit4("Enemy", g_Options.color_glow_enemy);
            ImGuiEx::ColorEdit4("Chickens", g_Options.color_glow_chickens);
            ImGuiEx::ColorEdit4("C4 Carrier", g_Options.color_glow_c4_carrier);
            ImGuiEx::ColorEdit4("Planted C4", g_Options.color_glow_planted_c4);
            ImGuiEx::ColorEdit4("Defuse Kits", g_Options.color_glow_defuse);
            ImGuiEx::ColorEdit4("Weapons", g_Options.color_glow_weapons);
            ImGui::PopItemWidth();

            ImGui::NextColumn();

            ImGui::Columns(1, nullptr, false);
            ImGui::PopStyleVar();
        }
    }
    ImGui::EndGroupBox();
}

void RenderMiscTab()
{
    bool placeholder_true = true;

    auto& style = ImGui::GetStyle();
    float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::ToggleButton("MISC", &placeholder_true, ImVec2{ group_w, 25.0f });
    ImGui::PopStyleVar();

    ImGui::BeginGroupBox("##body_content");
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ style.WindowPadding.x, style.ItemSpacing.y });
        ImGui::Columns(3, nullptr, false);
        ImGui::SetColumnOffset(1, group_w / 3.0f);
        ImGui::SetColumnOffset(2, 2 * group_w / 3.0f);
        ImGui::SetColumnOffset(3, group_w);

        ImGui::Checkbox("Bunny hop", &g_Options.misc_bhop);
		ImGui::Checkbox("Third Person", &g_Options.misc_thirdperson);
		if(g_Options.misc_thirdperson)
			ImGui::SliderFloat("Distance", &g_Options.misc_thirdperson_dist, 0.f, 200.f);
        ImGui::Checkbox("No hands", &g_Options.misc_no_hands);
		ImGui::Checkbox("Rank reveal", &g_Options.misc_showranks);
		ImGui::Checkbox("Watermark##hc", &g_Options.misc_watermark);
        //ImGui::PushItemWidth(-1.0f);
		ImGui::NextColumn();
        ImGui::SliderInt("viewmodel_fov:", &g_Options.viewmodel_fov, 68, 120);
		ImGui::Text("Postprocessing:");
        ImGui::SliderFloat("Red", &g_Options.mat_ambient_light_r, 0, 1);
        ImGui::SliderFloat("Green", &g_Options.mat_ambient_light_g, 0, 1);
        ImGui::SliderFloat("Blue", &g_Options.mat_ambient_light_b, 0, 1);
        //ImGui::PopItemWidth();

        ImGui::Columns(1, nullptr, false);
        ImGui::PopStyleVar();
    }
    ImGui::EndGroupBox();
}

void RenderEmptyTab()
{
	auto& style = ImGui::GetStyle();
	float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

	bool placeholder_true = true;

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
	ImGui::ToggleButton("AIM", &placeholder_true, ImVec2{ group_w, 25.0f });
	ImGui::PopStyleVar();

	ImGui::BeginGroupBox("##body_content");
	{
		auto message = "There's nothing here. Add something you want!";

		 auto pos = ImGui::GetCurrentWindow()->Pos;
		 auto wsize = ImGui::GetCurrentWindow()->Size;

		 pos = pos + wsize / 2.0f;

		 ImGui::RenderText(pos - ImGui::CalcTextSize(message) / 2.0f, message);
	}
	ImGui::EndGroupBox();
}

void RenderConfigTab()
{
    auto& style = ImGui::GetStyle();
    float group_w = ImGui::GetCurrentWindow()->Size.x - style.WindowPadding.x * 2;

    bool placeholder_true = true;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
    ImGui::ToggleButton("CONFIG", &placeholder_true, ImVec2{ group_w, 25.0f });
    ImGui::PopStyleVar();

    ImGui::BeginGroupBox("##body_content");
    {
		if (ImGui::Button("Save cfg")) {
			//Config::Get().Save();
		}
		if (ImGui::Button("Load cfg")) {
			//Config::Get().Load();
		}
    }
    ImGui::EndGroupBox();
}

void Menu::Initialize()
{
	CreateStyle();

    _visible = true;
}

void Menu::Shutdown()
{
    ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Menu::OnDeviceLost()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
}

void Menu::OnDeviceReset()
{
    ImGui_ImplDX9_CreateDeviceObjects();
}

#include "gui.h"
using namespace std;

void make_subtab(ImVec2 position, int index, const char* fmt, ...)
{
    ImGui::SetCursorPos(position);
    c_gui.text_with_font(c_gui.TABS, fmt);
    if (ImGui::IsItemClicked(0))
        subtab = index;
}

static int weapon_index = 7;

void RenderCurrentWeaponButton()
{
    if (!g_EngineClient->IsConnected() || !g_LocalPlayer || !g_LocalPlayer->IsAlive())
    {
        return;
    }
    auto weapon = g_LocalPlayer->m_hActiveWeapon();
    if (!weapon) {
        return;
    }
    if (ImGui::Button("Current")) {
        weapon_index = weapon->m_Item().m_iItemDefinitionIndex();
    }
}

static auto is_it_knife(const int i) -> bool {
    return (i >= WEAPON_KNIFE_BAYONET && i < GLOVE_STUDDED_BLOODHOUND) || i == WEAPON_KNIFE_T || i == WEAPON_KNIFE;
}

template <typename T>
void PickCurrentWeapon(int* idx, int* vec_idx, std::vector<T> arr) {
    if (!g_LocalPlayer) return;
    if (!g_EngineClient->IsInGame()) return;
    auto weapon = g_LocalPlayer->m_hActiveWeapon();
    if (!weapon) return;
    short wpn_idx = weapon->m_Item().m_iItemDefinitionIndex();
    if (is_it_knife(wpn_idx)) {
        *idx = WeaponNamesFull.at(0).definition_index;
        *vec_idx = 0;
        return;
    }
    auto wpn_it = std::find_if(arr.begin(), arr.end(), [wpn_idx](const T& a) {
        return a.definition_index == wpn_idx;
        });
    if (wpn_it != arr.end()) {
        *idx = wpn_idx;
        *vec_idx = std::abs(std::distance(arr.begin(), wpn_it));
    }
}


const char* GetWeaponNameById(int id)
{
    switch (id)
    {
    case 1:
        return "deagle";
    case 2:
        return "elite";
    case 3:
        return "fiveseven";
    case 4:
        return "glock";
    case 7:
        return "ak47";
    case 8:
        return "aug";
    case 9:
        return "awp";
    case 10:
        return "famas";
    case 11:
        return "g3sg1";
    case 13:
        return "galilar";
    case 14:
        return "m249";
    case 60:
        return "m4a1_silencer";
    case 16:
        return "m4a1";
    case 17:
        return "mac10";
    case 19:
        return "p90";
    case 23:
        return "mp5sd";
    case 24:
        return "ump45";
    case 25:
        return "xm1014";
    case 26:
        return "bizon";
    case 27:
        return "mag7";
    case 28:
        return "negev";
    case 29:
        return "sawedoff";
    case 30:
        return "tec9";
    case 32:
        return "hkp2000";
    case 33:
        return "mp7";
    case 34:
        return "mp9";
    case 35:
        return "nova";
    case 36:
        return "p250";
    case 38:
        return "scar20";
    case 39:
        return "sg556";
    case 40:
        return "ssg08";
    case 61:
        return "usp_silencer";
    case 63:
        return "cz75a";
    case 64:
        return "revolver";
    case 508:
        return "knife_m9_bayonet";
    case 500:
        return "bayonet";
    case 505:
        return "knife_flip";
    case 506:
        return "knife_gut";
    case 507:
        return "knife_karambit";
    case 509:
        return "knife_tactical";
    case 512:
        return "knife_falchion";
    case 514:
        return "knife_survival_bowie";
    case 515:
        return "knife_butterfly";
    case 516:
        return "knife_push";

    case 519:
        return "knife_ursus";
    case 520:
        return "knife_gypsy_jackknife";
    case 522:
        return "knife_stiletto";
    case 523:
        return "knife_widowmaker";

    case WEAPON_KNIFE_CSS:
        return "knife_css";

    case WEAPON_KNIFE_CORD:
        return "knife_cord";

    case WEAPON_KNIFE_CANIS:
        return "knife_canis";

    case WEAPON_KNIFE_OUTDOOR:
        return "knife_outdoor";

    case WEAPON_KNIFE_SKELETON:
        return "knife_skeleton";

    case 5027:
        return "studded_bloodhound_gloves";
    case 5028:
        return "t_gloves";
    case 5029:
        return "ct_gloves";
    case 5030:
        return "sporty_gloves";
    case 5031:
        return "slick_gloves";
    case 5032:
        return "leather_handwraps";
    case 5033:
        return "motorcycle_gloves";
    case 5034:
        return "specialist_gloves";
    case 5035:
        return "studded_hydra_gloves";

    default:
        return "";
    }
}

#include <algorithm>

namespace tabs
{
    void legitbot()
    {
        make_subtab(ImVec2(275, 47), 0, "Aim");
        make_subtab(ImVec2(365, 47), 1, "Trigger");
        if (subtab == 0)
        {
            auto settings = &g_Options.aimbot[weapon_index];

            ImGui::SetCursorPos(ImVec2(20, 112));
            c_gui.text_with_font(c_gui.SUBTAB, "Main");

            ImGui::SetCursorPos(ImVec2(20, ImGui::GetCursorPosY() + 10));
            ImGui::BeginChild("child_container", ImVec2(220, 437));
            {
                ImGui::PushFont(c_gui.CONTAINER);
                ImGui::Text("Weapons");
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6);
                if (ImGui::BeginCombo("", "Select weapon"))
                {
                    for (auto weapon : K_weapon_names) {
                        if (ImGui::Selectable(weapon.second, weapon_index == weapon.first))
                            weapon_index = weapon.first;
                    }
                    ImGui::EndCombo();
                }
                ImGui::PopStyleVar();
                RenderCurrentWeaponButton();

                ImGui::Checkbox("Enabled", &settings->enabled);
                ImGui::Checkbox("Friendly fire", &settings->deathmatch);
                if (weapon_index == WEAPON_P250 ||
                    weapon_index == WEAPON_USP_SILENCER ||
                    weapon_index == WEAPON_GLOCK ||
                    weapon_index == WEAPON_FIVESEVEN ||
                    weapon_index == WEAPON_TEC9 ||
                    weapon_index == WEAPON_DEAGLE ||
                    weapon_index == WEAPON_ELITE ||
                    weapon_index == WEAPON_HKP2000) {
                    ImGui::Checkbox("Autopistol", &settings->autopistol);
                }
                ImGui::Checkbox("Check Smoke", &settings->check_smoke);
                ImGui::Checkbox("Check Flash", &settings->check_flash);
                ImGui::Checkbox("Autowall", &settings->autowall);
                ImGui::Checkbox("Backtrack", &settings->backtrack.enabled);
                ImGui::Checkbox("Silent", &settings->silent);
                ImGui::Checkbox("Humanize", &settings->humanize);
                if (weapon_index == WEAPON_AWP || weapon_index == WEAPON_SSG08) {
                    ImGui::Checkbox("Only in zoom", &settings->only_in_zoom);
                }
                ImGui::PopFont();
            }
            ImGui::EndChild();

            ImGui::SetCursorPos(ImVec2(395, 112));
            c_gui.text_with_font(c_gui.SUBTAB, "Misc");
            ImGui::SetCursorPos(ImVec2(395, ImGui::GetCursorPosY() + 10));
            ImGui::BeginChild("child_container##zaebis", ImVec2(220, 437));
            {
                ImGui::PushFont(c_gui.CONTAINER);
                static char* priorities[] = {
                "Fov",
                "Health",
                "Damage",
                "Distance"
                };
                static char* aim_types[] = {
                "Hitbox",
                "Nearest"
                };
                static char* fov_types[] = {
                "Static",
                "Dynamic"
                };
                static char* smooth_types[] = {
                "Static",
                "Dynamic"
                };
                static char* hitbox_list[] = {
                "Head",
                "Neck",
                "Lower Neck",
                "Body",
                "Thorax",
                "Chest",
                "Right Thing",
                "Left Thing", // 7
                };
                ImGui::Text("Aim Type:");
                ImGui::Combo("##aimbot.aim_type", &settings->aim_type, aim_types, IM_ARRAYSIZE(aim_types));
                if (settings->aim_type == 0) {
                    ImGui::Text("Hitbox:");
                    ImGui::Combo("##aimbot.hitbox", &settings->hitbox, hitbox_list, IM_ARRAYSIZE(hitbox_list));
                }
                ImGui::Text("Priority:");
                ImGui::Combo("##aimbot.priority", &settings->priority, priorities, IM_ARRAYSIZE(priorities));
                ImGui::Text("Fov type:");
                ImGui::Combo("##aimbot.fov_type", &settings->fov_type, fov_types, IM_ARRAYSIZE(fov_types));
                ImGui::SliderFloat("Fov", &settings->fov, 0, 20);
                if (settings->silent) {
                    ImGui::SliderFloat("Silent fov", &settings->silent_fov, 0, 20);
                }
                ImGui::SliderFloat("Smooth", &settings->smooth, 1, 15);
                if (!settings->silent) {
                    ImGui::SliderInt("Shot delay", &settings->shot_delay, 0, 100);
                }
                ImGui::SliderInt("Kill delay", &settings->kill_delay, 0, 1000);
                if (settings->backtrack.enabled) {
                    ImGui::SliderInt("Backtrack ticks", &settings->backtrack.ticks, 0, 12);
                }
                if (settings->autowall) {
                    ImGui::SliderInt("Mindamage", &settings->min_damage, 1, 100);
                }
                ImGui::Checkbox("RCS##aimbot.rcs", &settings->rcs);
                ImGui::Text("RCS Type:");
                static char* rcs_types[] = {
                "Standalone",
                "Aim"
                };
                ImGui::Combo("##aimbot.rcs_type", &settings->rcs_type, rcs_types, IM_ARRAYSIZE(rcs_types));
                ImGui::Checkbox("RCS Fov", &settings->rcs_fov_enabled);
                if (settings->rcs_fov_enabled) {
                    ImGui::SliderFloat("##fovrcs", &settings->rcs_fov, 0, 20);
                }
                ImGui::Checkbox("RCS Smooth", &settings->rcs_smooth_enabled);
                if (settings->rcs_smooth_enabled) {
                    ImGui::SliderFloat("##smoothrcs", &settings->rcs_smooth, 1, 15);
                }
                ImGui::SliderInt("RCS X", &settings->rcs_x, 0, 100);
                ImGui::SliderInt("RCS Y", &settings->rcs_y, 0, 100);
                ImGui::SliderInt("RCS Start", &settings->rcs_start, 1, 30);
                ImGui::PopFont();
            }
            ImGui::EndChild();
        }
    }
    void esp()
    {
        make_subtab(ImVec2(275, 47), 0, "Main");
        make_subtab(ImVec2(330, 47), 1, "Models");
        make_subtab(ImVec2(395, 47), 2, "Other");

        if (subtab == 0)
        {
            ImGui::SetCursorPos(ImVec2(20, 112));
            c_gui.text_with_font(c_gui.SUBTAB, "Player");

            ImGui::SetCursorPos(ImVec2(20, ImGui::GetCursorPosY() + 10));
            ImGui::BeginChild("child_container", ImVec2(220, 437));
            {
                ImGui::PushFont(c_gui.CONTAINER);
                ImGui::Checkbox("Enable", &g_Options.esp_enabled);
                if (g_Options.esp_enabled)
                {
                    ImGui::Checkbox("Only enemies", &g_Options.esp_enemies_only);
                    ImGui::Checkbox("Only visible", &g_Options.esp_visible_only);
                    ImGui::Checkbox("Dormant", &g_Options.esp_dormant_esp);
                    ImGui::Checkbox("Boxes", &g_Options.esp_player_boxes);
                    if (g_Options.esp_player_boxes)
                    {
                        ImGui::SameLine();
                        ImGuiEx::ColorEdit4("##boxes", g_Options.color_esp_box);
                    }
                    ImGui::Checkbox("Names", &g_Options.esp_player_names);
                    if (g_Options.esp_player_names)
                    {
                        ImGui::SameLine();
                        ImGuiEx::ColorEdit4("##names", g_Options.color_esp_names);
                    }
                    ImGui::Checkbox("Health", &g_Options.esp_player_health);
                    ImGui::Checkbox("Armor", &g_Options.esp_player_armour);
                    ImGui::Checkbox("Weapon", &g_Options.esp_player_weapons);
                    if (g_Options.color_esp_weapon)
                    {
                        ImGui::SameLine();
                        ImGuiEx::ColorEdit4("##weapon", g_Options.color_esp_weapon);
                    }
                    ImGui::Checkbox("Snaplines", &g_Options.esp_player_snaplines);
                    if (g_Options.esp_player_snaplines)
                    {
                        ImGui::SameLine();
                        ImGuiEx::ColorEdit4("##snaplines", g_Options.color_esp_snaplines);
                    }
                    ImGui::Checkbox("Flags", &g_Options.esp_player_flags);
                    if (g_Options.esp_player_flags)
                    {
                        ImGui::Checkbox("Use icons", &g_Options.esp_player_flags_use_icons);
                        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6);
                        if (ImGui::BeginCombo("##flags", "select here", ImGuiComboFlags_NoArrowButton))
                        {
                            ImGui::Selectable("Armor", &g_Options.esp_player_flags_armor, ImGuiSelectableFlags_DontClosePopups);
                            ImGui::Selectable("Bomb or defuse kits", &g_Options.esp_player_flags_bomb_kits, ImGuiSelectableFlags_DontClosePopups);
                            ImGui::Selectable("Scoped", &g_Options.esp_player_flags_scoped, ImGuiSelectableFlags_DontClosePopups);
                            ImGui::Selectable("Flashed", &g_Options.esp_player_flags_flashed, ImGuiSelectableFlags_DontClosePopups);
                            ImGui::Selectable("Have zeus", &g_Options.esp_player_flags_have_taser, ImGuiSelectableFlags_DontClosePopups);
                            ImGui::EndCombo();
                        }
                        ImGui::PopStyleVar();
                    }
                    ImGui::Checkbox("Glow", &g_Options.glow_enabled);
                    if (g_Options.glow_enabled)
                    {
                        ImGui::SameLine();
                        ImGuiEx::ColorEdit4("##glow", g_Options.color_glow_enemy);
                        g_Options.glow_players = g_Options.glow_enabled;
                    }
                }
                ImGui::PopFont();
            }
            ImGui::EndChild();
        }
        else if (subtab == 1)
        {
            ImGui::SetCursorPos(ImVec2(20, 112));
            c_gui.text_with_font(c_gui.SUBTAB, "Main");

            ImGui::SetCursorPos(ImVec2(20, ImGui::GetCursorPosY() + 10));
            ImGui::BeginChild("child_container", ImVec2(220, 437));
            {
                ImGui::PushFont(c_gui.CONTAINER);
                static int index = 0;
                ImGui::Text("Chams");
                ImGui::Combo("##chams", &index, "Player\0Hands\0\0");
                const char* materials = "Metallic\0Flat\0Platinum\0Glass\0Crystal\0Gold\0Glow\0\0";
                switch (index)
                {
                case 0:
                    ImGui::Checkbox("Enabled", &g_Options.chams_player_enabled);
                    if (g_Options.chams_player_enabled)
                    {
                        ImGui::SameLine();
                        ImGuiEx::ColorEdit4("##chamsplayer", g_Options.color_chams_player_enemy_visible);
                        ImGui::Checkbox("Only enemy", &g_Options.chams_player_enemies_only);
                        ImGui::Checkbox("Overlay", &g_Options.chams_player_overlay);
                        ImGui::Checkbox("Behind wall", &g_Options.chams_player_behind_wall);
                        if (g_Options.chams_player_behind_wall)
                        {
                            ImGui::SameLine();
                            ImGuiEx::ColorEdit4("##chamsplayerbehindwall", g_Options.color_chams_player_enemy_occluded);
                        }
                        ImGui::Text("Type");
                        ImGui::Combo("##chamstypeplayer", &g_Options.chams_player_type, materials);
                    }
                    break;
                case 1:
                    ImGui::Checkbox("Enabled", &g_Options.chams_arms_enabled);
                    if (g_Options.chams_player_enabled)
                    {
                        ImGui::SameLine();
                        ImGuiEx::ColorEdit4("##chamsarms", g_Options.color_chams_arms);
                        ImGui::Text("Type");
                        ImGui::Combo("##chamstypearms", &g_Options.chams_arms_type, materials);
                    }
                    break;
                }
                ImGui::PopFont();
            }
            ImGui::EndChild();
        }
        else if (subtab == 2)
        {
            ImGui::SetCursorPos(ImVec2(20, 112));
            c_gui.text_with_font(c_gui.SUBTAB, "Main");

            ImGui::SetCursorPos(ImVec2(20, ImGui::GetCursorPosY() + 10));
            ImGui::BeginChild("child_container", ImVec2(220, 437));
            {
                ImGui::PushFont(c_gui.CONTAINER);
                ImGui::Checkbox("Watermark", &g_Options.misc_watermark);
                ImGui::Checkbox("Spectator list", &g_Options.esp_spectators);
                ImGui::Checkbox("ThirdPerson", &g_Options.misc_thirdperson);
                if (g_Options.misc_thirdperson)
                    ImGui::SliderFloat("Distance", &g_Options.misc_thirdperson_dist, 0.f, 200.f);
                ImGui::Checkbox("Custom console", &g_Options.misc_customconsole);
                if (g_Options.misc_customconsole)
                {
                    ImGui::SameLine();
                    ImGuiEx::ColorEdit4("##customconsole", g_Options.color_misc_customconsole);
                }
                ImGui::PopFont();
            }
            ImGui::EndChild();
        }
    }
    void skins()
    {
        make_subtab(ImVec2(275, 47), 0, "Weapon");
        make_subtab(ImVec2(365, 47), 1, "Skin");

        static char search_bar[32];

        static int weapon_index_skins = 7;
        static int weapon_vector_index_skins = 0;

        auto& selected_entry = g_Options.Items[WeaponNamesFull[weapon_vector_index_skins].definition_index];
        selected_entry.definition_index = weapon_index_skins;
        selected_entry.definition_vector_index = weapon_vector_index_skins;
        bool is_gloves = false;

        if (subtab == 0)
        {
            ImGui::SetCursorPos(ImVec2(20, 112));
            c_gui.text_with_font(c_gui.SUBTAB, "Main");

            ImGui::SetCursorPos(ImVec2(20, ImGui::GetCursorPosY() + 10));
            ImGui::BeginChild("child_container", ImVec2(220, 437));
            {
                ImGui::PushFont(c_gui.CONTAINER);
                if (ImGui::Button("Current", ImVec2(-1, 19)) || Menu::Get().SkinsFirstTimeRender)
                    PickCurrentWeapon(&weapon_index_skins, &weapon_vector_index_skins, WeaponNamesFull);

                int iter = 0;
                for (auto& weapon : WeaponNamesFull) {
                    if (ImGui::ButtonT(weapon.name, ImVec2(-1, 19), weapon_index_skins, weapon.definition_index, false, false)) {
                        weapon_index_skins = weapon.definition_index;
                        weapon_vector_index_skins = iter;
                    }
                    iter++;
                }
                ImGui::PopFont();
            }
            ImGui::EndChild();

            ImGui::SetCursorPos(ImVec2(395, 112));
            c_gui.text_with_font(c_gui.SUBTAB, "Misc");
            ImGui::SetCursorPos(ImVec2(395, ImGui::GetCursorPosY() + 10));
            ImGui::BeginChild("child_container##zaebis", ImVec2(220, 437));
            {
                ImGui::PushFont(c_gui.CONTAINER);
                ImGui::InputInt("Seed", &selected_entry.seed);
                ImGui::Checkbox("Enable StatTrack", &selected_entry.enable_stat_track);
                ImGui::InputInt("StatTrak", &selected_entry.stat_trak);
                ImGui::InputText("Name##Skins", selected_entry.name, sizeof(selected_entry.name));
                if (ImGui::SliderFloat("Wear", &selected_entry.wear, FLT_MIN, 1.f, "%.10f", 5))
                    selected_entry.wear = clamp(selected_entry.wear, FLT_MIN, 1.f);
                if (selected_entry.definition_index == WEAPON_KNIFE) {
                    ImGui::Combo("Knife", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text) {
                        *out_text = K_KnifeNames.at(idx).name;
                        return true;
                        }, nullptr, K_KnifeNames.size(), 10);
                    selected_entry.definition_override_index = K_KnifeNames.at(selected_entry.definition_override_vector_index).definition_index;
                }
                else if (selected_entry.definition_index == GLOVE_T_SIDE) {
                    ImGui::Combo("Glove", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text) {
                        *out_text = K_GloveNames.at(idx).name;
                        return true;
                        }, nullptr, K_GloveNames.size(), 10);
                    selected_entry.definition_override_index = K_GloveNames.at(selected_entry.definition_override_vector_index).definition_index;
                    is_gloves = true;
                }
                else
                    selected_entry.definition_override_vector_index = 0;

                static float next_enb_time = 0;
                float time_to_next_up = g_GlobalVars->curtime;

                time_to_next_up = std::clamp(next_enb_time - g_GlobalVars->curtime, 0.f, 1.f);

                std::string name = "Update (";
                name += std::to_string(time_to_next_up);
                name.erase(12, 16);
                name += ")";

                if (ImGui::Button(name.c_str(), ImVec2(-1, 19))) {
                    if (next_enb_time <= g_GlobalVars->curtime) {
                        Utils::ForceFullUpdate();
                        next_enb_time = g_GlobalVars->curtime + 1.f;
                    }
                }
                ImGui::PopFont();
            }
            ImGui::EndChild();
        }
        else if (subtab == 1)
        {
            ImGui::SetCursorPos(ImVec2(20, 112));
            c_gui.text_with_font(c_gui.SUBTAB, "Preview");

            ImGui::SetCursorPos(ImVec2(20, ImGui::GetCursorPosY() + 10));
            ImGui::BeginChild("child_container", ImVec2(220, 437));
            {
                ImGui::PushFont(c_gui.CONTAINER);
                if (m_skin_texture) {
                    ImGui::Image(m_skin_texture, ImVec2(130,90));
                }
                ImGui::PopFont();
            }
            ImGui::EndChild();

            ImGui::SetCursorPos(ImVec2(395, 112));
            c_gui.text_with_font(c_gui.SUBTAB, "Misc");
            ImGui::SetCursorPos(ImVec2(395, ImGui::GetCursorPosY() + 10));
            ImGui::PushFont(c_gui.CONTAINER);
            ImGui::BeginChild("child_container##zaebis", ImVec2(220, 437));
            {
                static bool showAllWeapons = false;
                ImGui::Checkbox("All paint kits", &showAllWeapons);
                ImGui::InputText("Filter##search_bar_skins", search_bar, sizeof(search_bar));

                if (ImGui::ListBoxHeader("##paintkits", ImVec2(-1, -1)))
                {
                    if (ImGui::Selectable("Default", selected_entry.paint_kit_index == 0)) {
                        selected_entry.paint_kit_index = 0;
                    }

                    static auto GetColRar = [&](int rar) -> ImVec4 {
                        switch (rar)
                        {
                        case 1:
                            return ImColor(150, 150, 150, 255);
                            break;
                        case 2:
                            return ImColor(100, 100, 255, 255);
                            break;
                        case 3:
                            return ImColor(50, 50, 255, 255);
                            break;
                        case 4:
                            return ImColor(255, 64, 242, 255);
                            break;
                        case 5:
                            return ImColor(255, 50, 50, 255);
                            break;
                        case 6:
                            return ImColor(255, 50, 50, 255);
                            break;
                        case 7:
                            return ImColor(255, 196, 46, 255);
                            break;
                        default:
                            return ImColor(150, 150, 150, 255);
                            break;
                        }
                    };

                    bool is_knife = selected_entry.definition_index == WEAPON_KNIFE || selected_entry.definition_index == WEAPON_KNIFE_T;
                    int defindex = (is_knife || selected_entry.definition_index == GLOVE_T_SIDE || selected_entry.definition_index == GLOVE_CT_SIDE) ? selected_entry.definition_override_index : selected_entry.definition_index;
                    std::string skinName = GetWeaponNameById(defindex);
                    if (skinName.compare("") != 0 || showAllWeapons)
                    {
                        std::string filter = std::string(search_bar);

                        int curItem = -1;
                        int s = 0;

                        auto set = g_Options.weaponSkins[skinName];
                        std::vector<std::string> list(set.size());
                        std::vector<std::string> anti_repeat;

                        std::copy(set.begin(), set.end(), list.begin());

                        if (showAllWeapons) {
                            list = {};
                            for (auto [key, value] : g_Options.weaponSkins) {
                                for (auto skin : value) {
                                    list.push_back(skin);
                                }
                            }
                        }

                        auto fnAntiRepeat = [&](std::string tx) -> bool {
                            auto ret = std::find(anti_repeat.begin(), anti_repeat.end(), tx) == anti_repeat.end();
                            if (ret)
                                anti_repeat.push_back(tx);
                            return ret;
                        };

                        for (auto skin : list) {
                            int pk = g_Options.skinMap[skin].paintkit;
                            if (pk == selected_entry.paint_kit_index)
                                curItem = s;

                            bool passed_filter = true;

                            if (!filter.empty()) {
                                std::string tempName = g_Options.skinNames[g_Options.skinMap[skin].tagName];
                                std::string tempQuery = filter;
                                std::transform(tempName.begin(), tempName.end(), tempName.begin(), ::tolower);
                                std::transform(tempQuery.begin(), tempQuery.end(), tempQuery.begin(), ::tolower);
                                if (tempName.find(tempQuery) == std::string::npos)
                                    passed_filter = false;
                            }

                            if (passed_filter && fnAntiRepeat(g_Options.skinNames[g_Options.skinMap[skin].tagName])) {
                                ImGui::PushStyleColor(ImGuiCol_Text, is_knife ? GetColRar(6) : GetColRar(g_Options.skinMap[skin].rarity));
                                if (ImGui::Selectable((g_Options.skinNames[g_Options.skinMap[skin].tagName] + "##" + skinName).c_str(), pk == selected_entry.paint_kit_index)) {
                                    selected_entry.paint_kit_index = pk;
                                }
                                ImGui::PopStyleColor();
                            }

                            s++;
                        }

                        static int saved_pk_index = -1;
                        static int saved_w_index = -1;
                        static int saved_wo_index = -1;
                        if (saved_pk_index != selected_entry.paint_kit_index || saved_w_index != selected_entry.definition_index || saved_wo_index != selected_entry.definition_override_index || Menu::Get().SkinsFirstTimeRender) {
                            saved_pk_index = selected_entry.paint_kit_index;
                            saved_w_index = selected_entry.definition_index;
                            saved_wo_index = selected_entry.definition_override_index;

                            std::string validFname = "";
                            if (saved_pk_index != 0)
                                for (auto s : g_Options.weaponSkins[skinName]) {
                                    auto gg = g_Options.skinMap[s];
                                    if (gg.paintkit == selected_entry.paint_kit_index) {
                                        validFname = s;
                                        break;
                                    }
                                }

                            std::string full_path;

                            if (validFname == "") {
                                full_path = "resource/flash/econ/weapons/base_weapons/";
                                if (!is_gloves)
                                    full_path += "weapon_";
                                full_path += skinName + ".png";
                            }
                            else {
                                full_path = ("resource/flash/econ/default_generated/");
                                if (!is_gloves)
                                    full_path += "weapon_";
                                full_path += skinName + "_" + validFname + "_light_large.png";
                            }


                            const auto handle = g_FileSys->open(full_path.c_str(), "r", "GAME");
                            if (handle) {
                                int file_len = g_FileSys->size(handle);
                                char* image = new char[file_len];

                                g_FileSys->read(image, file_len, handle);
                                g_FileSys->close(handle);

                                D3DXCreateTextureFromFileInMemory(g_D3DDevice9, image, file_len, &m_skin_texture);

                                delete[] image;
                            }
                            else
                                m_skin_texture = nullptr;
                        }
                    }
                    ImGui::ListBoxFooter();
                }
            }
            ImGui::EndChild();
            ImGui::PopFont();
        }
        Menu::Get().SkinsFirstTimeRender = false;
    }
    void misc()
    {
        ImGui::SetCursorPos(ImVec2(20, 112));
        c_gui.text_with_font(c_gui.SUBTAB, "Main");

        ImGui::SetCursorPos(ImVec2(20, ImGui::GetCursorPosY() + 10));
        ImGui::BeginChild("child_container", ImVec2(220, 437));
        {
            ImGui::PushFont(c_gui.CONTAINER);
            ImGui::Checkbox("Bunnyhop", &g_Options.misc_bhop);
            ImGui::Checkbox("Auto strafer", &g_Options.misc_autostrafe);
            if (g_Options.misc_autostrafe)
            {
                ImGui::Text("Type");
                ImGui::Combo("##autostrafetype", &g_Options.misc_autostrafe_type, "Viewangles\0Multidirectional\0\0");
                if (g_Options.misc_autostrafe_type == 0)
                {
                    ImGui::SliderInt("Retrack speed", &g_Options.misc_autostrafe_retrack, 1, 15);
                }
            }
            ImGui::Checkbox("Reveal ranks", &g_Options.misc_showranks);
            ImGui::Checkbox("Anti untrusted", &g_Options.misc_antiuntrusted);
            ImGui::Checkbox("Fakelag", &g_Options.misc_fakelag_enabled);
            if (g_Options.misc_fakelag_enabled)
            {
                ImGui::Checkbox("Standing", &g_Options.misc_fakelag_standing);
                ImGui::Checkbox("Moving", &g_Options.misc_fakelag_moving);
                ImGui::Checkbox("Unducking", &g_Options.misc_fakelag_unducking);
                ImGui::Combo("Mode", &g_Options.misc_fakelag_mode, "Factor\0Switch\0Adaptive\0Random\0Legit\0\0");
                ImGui::Text("Value");
                ImGui::SliderInt("##value", &g_Options.misc_fakelag_factor, 1, 15);
            }
            if (!g_Options.misc_antiuntrusted)
                ImGui::Checkbox("No duck cooldown", &g_Options.misc_noduckcooldown);
            ImGui::Checkbox("Desync", &g_Options.misc_desync);
            if (g_Options.misc_desync)
                ImGui::Hotkey("##Key", &g_Options.misc_desync_key);

            ImGui::PopFont();
        }
        ImGui::EndChild();
    }
    void cfg()
    {
        static char config_name[32];

        ImGui::SetCursorPos(ImVec2(20, 112));
        c_gui.text_with_font(c_gui.SUBTAB, "Main");

        ImGui::SetCursorPos(ImVec2(20, ImGui::GetCursorPosY() + 10));
        ImGui::PushFont(c_gui.CONTAINER);
        ImGui::BeginChild("child_container", ImVec2(220, 437));
        {
            static std::vector<std::string> configs;

            static auto load_configs = []() {
                std::vector<std::string> items = {};

                std::string path("C:\\SCALUX\\Configs"); // Тут изменяем ( \\ после текста нинада)
                if (!fs::is_directory(path))
                    fs::create_directories(path);

                for (auto& p : fs::directory_iterator(path))
                    items.push_back(p.path().string().substr(path.length() + 1));

                return items;
            };

            static auto is_configs_loaded = false;
            if (!is_configs_loaded) {
                is_configs_loaded = true;
                configs = load_configs();
            }

            static std::string current_config;

            ImGui::InputText("##config_name", config_name, sizeof(config_name));
            ImGui::SameLine();
            if (ImGui::Button("Create")) {
                current_config = std::string(config_name);

                Config->Save(current_config + ".ini");
                is_configs_loaded = false;
                memset(config_name, 0, 32);
            }

            if (ImGui::ListBoxHeader("##configs"))
            {
                for (auto& config : configs) {
                    if (ImGui::Selectable(config.c_str(), config == current_config)) {
                        current_config = config;
                    }
                }
                ImGui::ListBoxFooter();
            }

            if (!current_config.empty()) {

                if (ImGui::Button("Load"))
                    Config->Load(current_config);
                ImGui::SameLine();

                if (ImGui::Button("Save"))
                    Config->Save(current_config);
                ImGui::SameLine();

                if (ImGui::Button("Delete") && fs::remove("C:\\SCALUX\\Configs\\" + current_config)) { //Тут также изменяем на ваше, \\ обязательно
                    current_config.clear();
                    is_configs_loaded = false;
                }
                ImGui::SameLine();
            }
            if (ImGui::Button("Refresh"))
                is_configs_loaded = false;

        }
        ImGui::EndChild();
        ImGui::PopFont();
    }
}

namespace indicators
{
    void SpectatorList() {
        if (!g_Options.esp_spectators)
            return;
        int specs = 0;
        std::string spect = "";

        if (g_EngineClient->IsInGame() && g_EngineClient->IsConnected()) {
            int localIndex = g_EngineClient->GetLocalPlayer();
            C_BasePlayer* pLocalEntity = C_BasePlayer::GetPlayerByIndex(localIndex);
            if (pLocalEntity) {
                for (int i = 0; i < g_EngineClient->GetMaxClients(); i++) {
                    C_BasePlayer* pBaseEntity = C_BasePlayer::GetPlayerByIndex(i);
                    if (!pBaseEntity)										     continue;
                    if (pBaseEntity->m_iHealth() > 0)							 continue;
                    if (pBaseEntity == pLocalEntity)							 continue;
                    if (pBaseEntity->IsDormant())								 continue;
                    if (pBaseEntity->m_hObserverTarget() != pLocalEntity)		 continue;
                    player_info_t pInfo;
                    g_EngineClient->GetPlayerInfo(pBaseEntity->EntIndex(), &pInfo);
                    if (pInfo.ishltv) continue;

                    spect += pInfo.szName;
                    spect += "\n";
                    specs++;
                }
            }
        }
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f));
        if (ImGui::Begin("Spectator List", nullptr, ImVec2(0, 0), 0.4F, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) {
            if (specs > 0) spect += "\n";

            ImVec2 size = ImGui::CalcTextSize(spect.c_str());
            ImGui::SetWindowSize(ImVec2(200, 25 + size.y));
            ImGui::Text(spect.c_str());
        }
        ImGui::End();
        ImGui::PopStyleVar(2);
    }
}

void Menu::Render()
{
    ImGui::GetIO().MouseDrawCursor = _visible;

    c_gui.set_style();

    indicators::SpectatorList();

    const auto sidebar_size = get_sidebar_size();
    static int active_sidebar_tab = 0;

    if (_visible && alpha <= 1.f)
        alpha += 0.02;
    else if (!_visible && alpha >= 0.f)
        alpha -= 0.02;

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
    ImGui::Begin("AMNESIA", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);                          // Create a window called "Hello, world!" and append into it.
    {
        ImVec2 window = ImGui::GetWindowPos();
        int alpha_controls = (int)(alpha * 255);

        ImGui::SetWindowSize(ImVec2(749, 667));

        ImGui::SetCursorPos(ImVec2(20, 21));
        c_gui.text_with_font(c_gui.SCALUX, "AMNESIA");
        ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(window.x + 117, window.y + 15), ImVec2(window.x + 611, window.y + 43), ImColor(43, 43, 45, alpha_controls), 8);
        ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(window.x + 626, window.y + 15), ImVec2(window.x + 729, window.y + 43), ImColor(43, 43, 45, alpha_controls), 8);

        ImGui::GetWindowDrawList()->AddLine(ImVec2(window.x + 374, window.y + 112), ImVec2(window.x + 374, window.y + 112 + 555), ImColor(43, 43, 45));

        ImGui::SetCursorPos(ImVec2(641, 24));
        c_gui.text_with_font(c_gui.TABS, "99 days");
        ImGui::SetCursorPos(ImVec2(694, 11));
        ImGui::Image(c_gui.AVATAR, ImVec2(35, 35));

        ImGui::SetCursorPos(ImVec2(137.5, 21.53));
        ImGui::Image(tab == 0 ? c_gui.RAGE_SELECTED : c_gui.RAGE, ImVec2(15, 15));
        if (ImGui::IsItemClicked(0))
            tab = 0;

        ImGui::SetCursorPos(ImVec2(157, 23));
        c_gui.text_colored_with_font(c_gui.TABS, tab == 0 ? ImColor(255, 0, 0) : ImColor(255, 255, 255), "Ragebot");
        if (ImGui::IsItemClicked(0))
            tab = 0;

        ImGui::SetCursorPos(ImVec2(224.02, 23.85));
        ImGui::Image(tab == 1 ? c_gui.LEGIT_SELECTED : c_gui.LEGIT, ImVec2(17, 11));
        if (ImGui::IsItemClicked(0))
            tab = 1;

        ImGui::SetCursorPos(ImVec2(247, 23));
        c_gui.text_colored_with_font(c_gui.TABS, tab == 1 ? ImColor(255, 0, 0) : ImColor(255, 255, 255), "Legitbot");
        if (ImGui::IsItemClicked(0))
            tab = 1;

        ImGui::SetCursorPos(ImVec2(315, 21));
        ImGui::Image(tab == 2 ? c_gui.VISUALS_SELECTED : c_gui.VISUALS, ImVec2(13, 15));
        if (ImGui::IsItemClicked(0))
            tab = 2;

        ImGui::SetCursorPos(ImVec2(333, 23));
        c_gui.text_colored_with_font(c_gui.TABS, tab == 2 ? ImColor(255, 0, 0) : ImColor(255, 255, 255), "Visuals");
        if (ImGui::IsItemClicked(0))
            tab = 2;

        ImGui::SetCursorPos(ImVec2(391.61, 21));
        ImGui::Image(tab == 3 ? c_gui.SKINS_SELECTED : c_gui.SKINS, ImVec2(15, 16));
        if (ImGui::IsItemClicked(0))
            tab = 3;

        ImGui::SetCursorPos(ImVec2(412, 23));
        c_gui.text_colored_with_font(c_gui.TABS, tab == 3 ? ImColor(255, 0, 0) : ImColor(255, 255, 255), "Skins");
        if (ImGui::IsItemClicked(0))
            tab = 3;

        ImGui::SetCursorPos(ImVec2(461, 22));
        ImGui::Image(tab == 4 ? c_gui.MISC_SELECTED : c_gui.MISC, ImVec2(15, 11));
        if (ImGui::IsItemClicked(0))
            tab = 4;

        ImGui::SetCursorPos(ImVec2(481, 23));
        c_gui.text_colored_with_font(c_gui.TABS, tab == 4 ? ImColor(255, 0, 0) : ImColor(255, 255, 255), "Misc");
        if (ImGui::IsItemClicked(0))
            tab = 4;

        ImGui::SetCursorPos(ImVec2(527, 22));
        ImGui::Image(tab == 5 ? c_gui.CONFIGS_SELECTED : c_gui.CONFIGS, ImVec2(14, 14));
        if (ImGui::IsItemClicked(0))
            tab = 5;

        ImGui::SetCursorPos(ImVec2(546, 23));
        c_gui.text_colored_with_font(c_gui.TABS, tab == 5 ? ImColor(255, 0, 0) : ImColor(255, 255, 255), "Configs");
        if (ImGui::IsItemClicked(0))
            tab = 5;

        if (tab == 0 || tab == 1 || tab == 2 || tab == 3)
            ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(window.x + 250, window.y + 42), ImVec2(window.x + 437, window.y + 66), ImColor(43, 43, 45, alpha_controls), 8, ImDrawCornerFlags_Bot);
    
        switch (tab)
        {
        case 1:
            tabs::legitbot();
            break;
        case 2:
            tabs::esp();
            break;
        case 4:
            tabs::misc();
            break;
        case 3:
            tabs::skins();
            break;
        case 5:
            tabs::cfg();
            break;
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void Menu::Toggle()
{
    _visible = !_visible;
}

void Menu::CreateStyle()
{
	ImGui::StyleColorsDark();
	ImGui::SetColorEditOptions(ImGuiColorEditFlags_HEX);
	_style.FrameRounding = 0.f;
	_style.WindowRounding = 0.f;
	_style.ChildRounding = 0.f;
	_style.Colors[ImGuiCol_Button] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
	_style.Colors[ImGuiCol_Header] = ImVec4(0.260f, 0.590f, 0.980f, 0.670f);
	_style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.260f, 0.590f, 0.980f, 1.000f);
	//_style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.000f, 0.545f, 1.000f, 1.000f);
	//_style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.060f, 0.416f, 0.980f, 1.000f);
	_style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.30f, 1.0f);
	_style.Colors[ImGuiCol_WindowBg] = ImVec4(0.000f, 0.009f, 0.120f, 0.940f);
	_style.Colors[ImGuiCol_PopupBg] = ImVec4(0.076f, 0.143f, 0.209f, 1.000f);
	ImGui::GetStyle() = _style;
}

