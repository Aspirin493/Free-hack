#pragma once
#include "../../options.hpp"
#include "../../valve_sdk/sdk.hpp"
#include "../../valve_sdk/csgostructs.hpp"
#include <set>
#include <string>
#include <map>

class Skins {
private:
	auto get_icon_override(const std::string original) const -> const char* {
		return g_Options.IconOverrides.count(original) ? g_Options.IconOverrides.at(original).data() : nullptr;
	};
private:
	void erase_override_if_exists_by_index(const int definition_index);
	void apply_config_on_attributable_item(C_BaseAttributableItem* item, const item_setting* config, const unsigned xuid_low);
public:
	void OnFrameStageNotify(bool frame_end);
};

extern Skins* g_SkinChanger;