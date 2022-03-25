#include "chams.hpp"
#include <fstream>

#include "../valve_sdk/csgostructs.hpp"
#include "../options.hpp"
#include "../hooks.hpp"
#include "../helpers/input.hpp"
#include "../helpers/KeyValues.hpp"

Chams::Chams() {
	materialRegular = g_MatSystem->FindMaterial("debug/debugambientcube");
	materialFlat = g_MatSystem->FindMaterial("debug/debugdrawflat");
	materialPlatinum = g_MatSystem->FindMaterial("models/inventory_items/trophy_majors/gloss");
	materialGlass = g_MatSystem->FindMaterial("models/inventory_items/cologne_prediction/cologne_prediction_glass");
	materialCrystal = g_MatSystem->FindMaterial("models/inventory_items/trophy_majors/crystal_clear");
	materialGold = g_MatSystem->FindMaterial("models/inventory_items/trophy_majors/gold");
	KeyValues* GlowChams = new KeyValues("GlowChams");
	GlowChams->LoadFromBuffer(GlowChams, "GlowChams", R"#("VertexLitGeneric" {
    "$additive" "1"
    "$envmap" "models/effects/cube_white"
    "$envmaptint" "[1 1 1]"
    "$envmapfresnel" "1"
    "$envmapfresnelminmaxexp" "[0 1 2]"
    "$alpha" "0.6"
    })#");
	materialGlow = g_MatSystem->CreateMaterial("GlowChams", GlowChams);
	materialGlow->IncrementReferenceCount();
}

Chams::~Chams() {
}

void Chams::OverrideMaterial(bool ignorez, int type, const Color& rgba)
{
	IMaterial* material = nullptr;
	switch (type)
	{
	case 0: material = materialRegular; break;
	case 1: material = materialFlat; break;
	case 2: material = materialPlatinum; break;
	case 3: material = materialGlass; break;
	case 4: material = materialCrystal; break;
	case 5: material = materialGold; break;
	case 6: material = materialGlow; break;
	}
	material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, ignorez);
	material->ColorModulate(
		rgba.r() / 255.0f,
		rgba.g() / 255.0f,
		rgba.b() / 255.0f);
	bool bFound = false;
	auto pVar = material->FindVar("$envmaptint", &bFound);
	if (bFound)
		(*(void(__thiscall**)(int, float, float, float))(*(DWORD*)pVar + 44))((uintptr_t)pVar, rgba.r() / 255.f, rgba.g() / 255.f, rgba.b() / 255.f);
	g_MdlRender->ForcedMaterialOverride(material);
}

void Chams::OnDrawModelExecute(
	IMatRenderContext* ctx,
	const DrawModelState_t& state,
	const ModelRenderInfo_t& info,
	matrix3x4_t* matrix)
{
	static auto fnDME = Hooks::mdlrender_hook.get_original<decltype(&Hooks::hkDrawModelExecute)>(index::DrawModelExecute);

	const auto mdl = info.pModel;

	bool is_arm = strstr(mdl->szName, "arms") != nullptr;
	bool is_player = strstr(mdl->szName, "models/player") != nullptr;
	bool is_sleeve = strstr(mdl->szName, "sleeve") != nullptr;
	//bool is_weapon = strstr(mdl->szName, "weapons/v_")  != nullptr;

	if (is_player && g_Options.chams_player_enabled) {
		// 
		// Draw player Chams.
		// 
		auto ent = C_BasePlayer::GetPlayerByIndex(info.entity_index);

		if (ent && g_LocalPlayer && ent->IsAlive()) {
			const auto enemy = ent->m_iTeamNum() != g_LocalPlayer->m_iTeamNum();
			if (!enemy && g_Options.chams_player_enemies_only)
				return;

			const auto clr_front = enemy ? Color(g_Options.color_chams_player_enemy_visible) : Color(g_Options.color_chams_player_ally_visible);
			const auto clr_back = enemy ? Color(g_Options.color_chams_player_enemy_occluded) : Color(g_Options.color_chams_player_ally_occluded);

			if (g_Options.chams_player_overlay)
			{
				IMaterial* material = nullptr;
				material->ColorModulate(
					clr_front.r() / 255.0f,
					clr_front.g() / 255.0f,
					clr_front.b() / 255.0f);
				g_MdlRender->ForcedMaterialOverride(material);
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
			}

			if (g_Options.chams_player_behind_wall) {
				OverrideMaterial(true, g_Options.chams_player_type, clr_back);
				fnDME(g_MdlRender, 0, ctx, state, info, matrix);
				OverrideMaterial(false, g_Options.chams_player_type, clr_front);
			}
			else {
				OverrideMaterial(false, g_Options.chams_player_type, clr_front);
			}
		}
	}
	else if (is_sleeve && g_Options.chams_arms_enabled) {
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;
		// 
		// Remove sleeves when drawing Chams.
		// 
		material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
		g_MdlRender->ForcedMaterialOverride(material);
	}
	else if (is_arm) {
		auto material = g_MatSystem->FindMaterial(mdl->szName, TEXTURE_GROUP_MODEL);
		if (!material)
			return;
		if (g_Options.misc_no_hands) {
			// 
			// No hands.
			// 
			material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);
			g_MdlRender->ForcedMaterialOverride(material);
		}
		else if (g_Options.chams_arms_enabled) {
			OverrideMaterial(false, g_Options.chams_arms_type, Color(g_Options.color_chams_arms));
		}
	}
}