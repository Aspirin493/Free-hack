#include "changer.hpp"
#include "../../helpers/utils.hpp"
#include "../../config.hpp"

Skins* g_SkinChanger = new Skins();

static auto is_knife(const int i) -> bool {
	return (i >= WEAPON_KNIFE_BAYONET && i < GLOVE_STUDDED_BLOODHOUND) || i == WEAPON_KNIFE_T || i == WEAPON_KNIFE;
}

static auto get_wearable_create_fn() -> CreateClientClassFn {
	auto clazz = g_CHLClient->GetAllClasses();

	while (strcmp(clazz->m_pNetworkName, "CEconWearable"))
		clazz = clazz->m_pNext;
	return clazz->m_pCreateFn;
}

void Skins::erase_override_if_exists_by_index(const int definition_index) {
	if (WeaponInfo.count(definition_index)) {
		auto& icon_override_map = g_Options.IconOverrides;
		const auto& original_item = WeaponInfo.at(definition_index);
		if (original_item.icon && icon_override_map.count(original_item.icon))
			icon_override_map.erase(icon_override_map.at(original_item.icon));
	}
}
void Skins::apply_config_on_attributable_item(C_BaseAttributableItem* item, const item_setting* config, const unsigned xuid_low) {
	item->m_Item().m_iItemIDHigh() = -1;
	item->m_Item().m_iAccountID() = xuid_low;

	if (config->name[0] != '\0')
		strcpy_s(item->m_Item().m_iCustomName(), config->name);

	if (config->paint_kit_index)
		item->m_nFallbackPaintKit() = config->paint_kit_index;

	if (config->seed)
		item->m_nFallbackSeed() = config->seed;

	if (config->stat_trak + 1 && config->enable_stat_track)
		item->m_nFallbackStatTrak() = config->stat_trak;

	
	item->m_flFallbackWear() = config->wear;
	auto& definition_index = item->m_Item().m_iItemDefinitionIndex();
	auto& icon_override_map = g_Options.IconOverrides;
	if (config->definition_override_index && config->definition_override_index != definition_index && WeaponInfo.count(config->definition_override_index)) {
		const auto old_definition_index = definition_index;
		definition_index = config->definition_override_index;
		const auto& replacement_item = WeaponInfo.at(config->definition_override_index);
		item->m_nModelIndex() = g_MdlInfo->GetModelIndex(replacement_item.model);
		item->SetModelIndex(g_MdlInfo->GetModelIndex(replacement_item.model));
		item->GetClientNetworkable()->PreDataUpdate(0);
		if (old_definition_index && WeaponInfo.count(old_definition_index)) {
			const auto& original_item = WeaponInfo.at(old_definition_index);
			if (original_item.icon && replacement_item.icon) {
				icon_override_map[original_item.icon] = replacement_item.icon;
			}
		}
	}
	else {
		erase_override_if_exists_by_index(definition_index);
	}
}

void Skins::OnFrameStageNotify(bool frame_end) {
	const auto local_index = g_EngineClient->GetLocalPlayer();
	const auto local = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntity(local_index));
	if (!local)
		return;

	player_info_t player_info;
	if (!g_EngineClient->GetPlayerInfo(local_index, &player_info))
		return;

	if (frame_end) {
		const auto wearables = local->m_hMyWearables();
		const auto glove_config = &g_Options.Items[GLOVE_T_SIDE];

		static auto glove_handle = CBaseHandle(0);
		auto glove = reinterpret_cast<C_BaseAttributableItem*>(g_EntityList->GetClientEntityFromHandle(wearables[0]));
		if (!glove) {
			const auto our_glove = reinterpret_cast<C_BaseAttributableItem*>(g_EntityList->GetClientEntityFromHandle(glove_handle));
			if (our_glove) // Our glove still exists
			{
				wearables[0] = glove_handle;
				glove = our_glove;
			}
		}

		if (!local->IsAlive()) {
			if (glove) {
				glove->GetClientNetworkable()->SetDestroyedOnRecreateEntities();
				glove->GetClientNetworkable()->Release();
			}
			return;
		}

		if (glove_config && glove_config->definition_override_index) {
			if (!glove) {
				static auto create_wearable_fn = get_wearable_create_fn();
				const auto entry = g_EntityList->GetHighestEntityIndex() + 1;
				const auto serial = rand() % 0x1000;

				//glove = static_cast<C_BaseAttributableItem*>(create_wearable_fn(entry, serial));
				create_wearable_fn(entry, serial);
				glove = reinterpret_cast<C_BaseAttributableItem*>(g_EntityList->GetClientEntity(entry));

				assert(glove);
				{
					static auto set_abs_origin_addr = Utils::PatternScan(GetModuleHandleW(L"client.dll"), "55 8B EC 83 E4 F8 51 53 56 57 8B F1");
					const auto set_abs_origin_fn = reinterpret_cast<void(__thiscall*)(void*, const std::array<float, 3>&)>(set_abs_origin_addr);
					static constexpr std::array<float, 3> new_pos = { 10000.f, 10000.f, 10000.f };
					set_abs_origin_fn(glove, new_pos);
				}

				const auto wearable_handle = reinterpret_cast<CBaseHandle*>(&wearables[0]);
				*wearable_handle = entry | serial << 16;
				glove_handle = wearables[0];
			}

			// Thanks, Beakers
			glove->SetGloveModelIndex(-1);
			apply_config_on_attributable_item(glove, glove_config, player_info.xuid_low);
		}
	}
	else {
		auto weapons = local->m_hMyWeapons();
		for (int i = 0; weapons[i].IsValid(); i++) {
			C_BaseAttributableItem* weapon = (C_BaseAttributableItem*)g_EntityList->GetClientEntityFromHandle(weapons[i]);
			if (!weapon)
				continue;
			auto& definition_index = weapon->m_Item().m_iItemDefinitionIndex();
			const auto active_conf = &g_Options.Items[is_knife(definition_index) ? WEAPON_KNIFE : definition_index];
			apply_config_on_attributable_item(weapon, active_conf, player_info.xuid_low);
		}

		const auto view_model_handle = local->m_hViewModel();
		if (!view_model_handle.IsValid())
			return;

		const auto view_model = static_cast<C_BaseViewModel*>(g_EntityList->GetClientEntityFromHandle(view_model_handle));
		if (!view_model)
			return;

		const auto view_model_weapon_handle = view_model->m_hWeapon();
		if (!view_model_weapon_handle.IsValid())
			return;

		const auto view_model_weapon = static_cast<C_BaseCombatWeapon*>(g_EntityList->GetClientEntityFromHandle(view_model_weapon_handle));
		if (!view_model_weapon)
			return;

		if (WeaponInfo.count(view_model_weapon->m_Item().m_iItemDefinitionIndex())) {
			const auto override_model = WeaponInfo.at(view_model_weapon->m_Item().m_iItemDefinitionIndex()).model;
			auto override_model_index = g_MdlInfo->GetModelIndex(override_model);
			view_model->m_nModelIndex() = override_model_index;
			auto world_model_handle = view_model_weapon->m_hWeaponWorldModel();
			if (!world_model_handle.IsValid())
				return;
			const auto world_model = static_cast<C_BaseWeaponWorldModel*>(g_EntityList->GetClientEntityFromHandle(world_model_handle));
			if (!world_model)
				return;
			world_model->m_nModelIndex() = override_model_index + 1;
		}

	}
}