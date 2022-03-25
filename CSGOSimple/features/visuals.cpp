#include <algorithm>

#include "visuals.hpp"

#include "../options.hpp"
#include "../helpers/math.hpp"
#include "../helpers/utils.hpp"


RECT GetBBox(C_BaseEntity* ent)
{
	RECT rect{};
	auto collideable = ent->GetCollideable();

	if (!collideable)
		return rect;

	auto min = collideable->OBBMins();
	auto max = collideable->OBBMaxs();

	const matrix3x4_t& trans = ent->m_rgflCoordinateFrame();

	Vector points[] = {
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector pointsTransformed[8];
	for (int i = 0; i < 8; i++) {
		Math::VectorTransform(points[i], trans, pointsTransformed[i]);
	}

	Vector screen_points[8] = {};

	for (int i = 0; i < 8; i++) {
		if (!Math::WorldToScreen(pointsTransformed[i], screen_points[i]))
			return rect;
	}

	auto left = screen_points[0].x;
	auto top = screen_points[0].y;
	auto right = screen_points[0].x;
	auto bottom = screen_points[0].y;

	for (int i = 1; i < 8; i++) {
		if (left > screen_points[i].x)
			left = screen_points[i].x;
		if (top < screen_points[i].y)
			top = screen_points[i].y;
		if (right < screen_points[i].x)
			right = screen_points[i].x;
		if (bottom > screen_points[i].y)
			bottom = screen_points[i].y;
	}
	return RECT{ (long)left, (long)top, (long)right, (long)bottom };
}

Visuals::Visuals()
{
	InitializeCriticalSection(&cs);
}

Visuals::~Visuals() {
	DeleteCriticalSection(&cs);
}

//--------------------------------------------------------------------------------
void Visuals::Render() {
}
//--------------------------------------------------------------------------------
bool Visuals::Player::Begin(C_BasePlayer* pl)
{
	if (!pl->IsAlive())
	{
		ctx.alpha = 0;
		Visuals::Get().flPlayerAlpha[pl->EntIndex()] = 0;
		return false;
	}
	if (pl->IsDormant() && !g_Options.esp_dormant_esp)
		return false;

	if (pl->IsDormant() && Visuals::Get().flPlayerAlpha[pl->EntIndex()] > 0)
		Visuals::Get().flPlayerAlpha[pl->EntIndex()] -= 0.2f;
	else if (!pl->IsDormant())
		if (Visuals::Get().flPlayerAlpha[pl->EntIndex()] < 255)
			Visuals::Get().flPlayerAlpha[pl->EntIndex()] += 3.0f;

	int alpha = (int)Visuals::Get().flPlayerAlpha[pl->EntIndex()];
	std::clamp(alpha, 0, 255);
	if (alpha < 0)
		alpha = 0;
	if (alpha > 255)
		alpha = 255;
	ctx.alpha = alpha;

	ctx.pl = pl;
	ctx.is_enemy = g_LocalPlayer->m_iTeamNum() != pl->m_iTeamNum();
	ctx.is_visible = g_LocalPlayer->CanSeePlayer(pl, HITBOX_CHEST);

	if (!ctx.is_enemy && g_Options.esp_enemies_only)
		return false;

	if (!ctx.is_visible && g_Options.esp_visible_only)
		return true;

	ctx.clr = ctx.is_enemy ? Color((ctx.is_visible ? g_Options.color_esp_enemy_visible : g_Options.color_esp_enemy_occluded)) : Color((ctx.is_visible ? g_Options.color_esp_ally_visible : g_Options.color_esp_ally_occluded));
	ctx.clr.SetAlpha(ctx.alpha);

	if (ctx.pl->IsDormant() && ctx.alpha < 25)
		return false;

	auto head = pl->GetHitboxPos(HITBOX_HEAD);
	auto origin = pl->m_vecOrigin();

	head.z += 15;

	if (!Math::WorldToScreen(head, ctx.head_pos) ||
		!Math::WorldToScreen(origin, ctx.feet_pos))
		return false;

	auto h = fabs(ctx.head_pos.y - ctx.feet_pos.y);
	auto w = h / 1.65f;

	ctx.bbox.left = static_cast<long>(ctx.feet_pos.x - w * 0.5f);
	ctx.bbox.right = static_cast<long>(ctx.bbox.left + w);
	ctx.bbox.bottom = static_cast<long>(ctx.feet_pos.y);
	ctx.bbox.top = static_cast<long>(ctx.head_pos.y);

	return true;
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderBox() {
	Render::Get().RenderBoxByType(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, ctx.pl->IsDormant() ? Color(200, 200, 200, ctx.alpha) : Color(g_Options.color_esp_box), 1);
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderName()
{
	player_info_t info = ctx.pl->GetPlayerInfo();

	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, info.szName);

	Render::Get().RenderText(info.szName, ctx.feet_pos.x - sz.x / 2, ctx.head_pos.y - sz.y, 14.f, ctx.pl->IsDormant() ? Color(200, 200, 200, ctx.alpha) : Color(g_Options.color_esp_names));
}
//--------------------------------------------------------------------------------
constexpr float SPEED_FREQ = 255 / 1.0f;
//--------------------------------------------------------------------------------
void Visuals::Player::RenderHealth()
{
	auto hp = ctx.pl->m_iHealth();

	static float prev_player_hp[65];

	if (!prev_player_hp[ctx.pl->EntIndex()])
		prev_player_hp[ctx.pl->EntIndex()] = 0;

	if (prev_player_hp[ctx.pl->EntIndex()] > hp)
		prev_player_hp[ctx.pl->EntIndex()] -= SPEED_FREQ * g_GlobalVars->frametime;
	else
		prev_player_hp[ctx.pl->EntIndex()] = hp;

	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
	float box_width = (fabs(ctx.bbox.bottom - ctx.bbox.top) * prev_player_hp[ctx.pl->EntIndex()]) / 100;

	float off = 8;

	int height = (box_h * hp) / 100;

	int green = int(hp * 2.55f);
	int red = 255 - green;

	int x = ctx.bbox.left - off;
	int y = ctx.bbox.top;
	int w = 4;
	int h = box_h;

	Color fade = Color(5, 5, 5);
	fade.SetAlpha(ctx.alpha);

	Render::Get().RenderBox(x, y, x + w, y + h, fade, 1.f, true);
	Render::Get().RenderBox(x + 1, y + 1, x + w - 1, y + h, Color(10, 10, 10, ctx.alpha), 1.f, true);
	Render::Get().RenderBox(x + 1, y + h - (int)box_width, x + w - 1, y + h, ctx.pl->IsDormant() ? Color(200, 200, 200, ctx.alpha) : Color(red, green, 0, ctx.alpha), 1.f, true);
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderArmour()
{
	auto armour = ctx.pl->m_ArmorValue();

	if (armour < 1)
		return;

	static float prev_player_armour[65];

	if (!prev_player_armour[ctx.pl->EntIndex()])
		prev_player_armour[ctx.pl->EntIndex()] = 0;

	if (prev_player_armour[ctx.pl->EntIndex()] > armour)
		prev_player_armour[ctx.pl->EntIndex()] -= SPEED_FREQ * g_GlobalVars->frametime;
	else
		prev_player_armour[ctx.pl->EntIndex()] = armour;

	float box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);

	const int off = 2;

	const int height = (prev_player_armour[ctx.pl->EntIndex()]) * box_h / 100;

	const int x = ctx.bbox.right + off;
	const int y = ctx.bbox.top;
	const int w = 4;
	const int h = box_h;

	Color fade = Color(5, 5, 5);
	fade.SetAlpha(ctx.clr.a());

	Render::Get().RenderBox(x, y, x + w, y + h, fade, 1.f, true);
	Render::Get().RenderBoxFilled(x + 1, y + 1, x + w, y + h, Color(10, 10, 10, ctx.alpha));
	Render::Get().RenderBoxFilled(x + 1, y + h - height, x + w - 1, y + h, Color(70, 130, 180, ctx.alpha));
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderWeaponName()
{
	auto weapon = ctx.pl->m_hActiveWeapon().Get();

	if (!weapon) return;
	if (!weapon->GetCSWeaponData()) return;

	auto text = weapon->GetCSWeaponData()->szWeaponName + 7;
	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, text);
	Render::Get().RenderText(text, ctx.feet_pos.x, ctx.feet_pos.y, 14.f, ctx.pl->IsDormant() ? Color(200, 200, 200, ctx.alpha) : Color(g_Options.color_esp_weapon), true,
		g_pDefaultFont);
}
//--------------------------------------------------------------------------------
void Visuals::Player::RenderSnapline()
{

	int screen_w, screen_h;
	g_EngineClient->GetScreenSize(screen_w, screen_h);

	Render::Get().RenderLine(screen_w / 2.f, (float)screen_h,
		ctx.feet_pos.x, ctx.feet_pos.y, ctx.pl->IsDormant() ? Color(200, 200, 200, ctx.alpha) : Color(g_Options.color_esp_snaplines));
}
//--------------------------------------------------------------------------------
#define FLAG_MACRO std::pair<std::string, Color> // :joy:
#define FLAG(string, draw_when, color) if (draw_when) vecFlags.push_back(FLAG_MACRO(string, color)) 
//--------------------------------------------------------------------------------
void Visuals::Player::RenderFlags() /*Not the best way to do this, tbh*/
{
	if (!g_LocalPlayer || !g_EngineClient->IsConnected() || !g_EngineClient->IsInGame())
		return;

	if (!g_Options.esp_player_flags)
		return;

	auto& flags_info = FlagsInfo[ctx.pl->EntIndex()];
	flags_info.clear();

	bool have_armor = ctx.pl->m_ArmorValue() > 0 && g_Options.esp_player_flags_armor;
	bool have_bomb_or_kits = ctx.pl->m_bHasDefuser() || ctx.pl->HasC4() && g_Options.esp_player_flags_bomb_kits;
	bool is_scoped = ctx.pl->m_bIsScoped() && g_Options.esp_player_flags_scoped;
	bool is_flashed = ctx.pl->IsFlashed() && g_Options.esp_player_flags_flashed;
	bool have_taser = ctx.pl->m_hMyWeapons()->Get()->IsZeus() && g_Options.esp_player_flags_have_taser;

	bool use_icons = g_Options.esp_player_flags_use_icons;

	flags_info.emplace_back(Flags_t(ctx.pl->m_bHasHelmet() ? use_icons ? "q" : "HK" : use_icons ? "p" : "K",
		have_armor, Color(255,255,255, ctx.alpha), use_icons));

	flags_info.emplace_back(Flags_t(ctx.pl->HasC4() ? "o" : "r",
		have_bomb_or_kits, Color(255, 255, 255, ctx.alpha), use_icons));

	flags_info.emplace_back(Flags_t(use_icons ? "s" : "Scoped",
		is_scoped, Color(255, 255, 255, ctx.alpha), use_icons));

	flags_info.emplace_back(Flags_t(use_icons ? "i" : "Flashed",
		is_flashed, Color(255, 255, 255, ctx.alpha), use_icons));

	flags_info.emplace_back(Flags_t(use_icons ? "p" : "Taser",
		have_taser, Color(255, 255, 255, ctx.alpha), use_icons));

	// тут уже пошел рендер флагов

	float step = 0.f;
	static float modifier[65];

	auto easeOutQuad = [](float x) {
		return 1 - (1 - x) * (1 - x);
	};

	float animation_speed = 0.5;

	for (int j = 0; j < flags_info.size(); j++) {

		auto& current_flags = flags_info[j];
		if (current_flags.toggled && modifier[j] < 1.f)
			modifier[j] += animation_speed * 1.5f;
		else if (!current_flags.toggled && modifier[j] > 0.f)
			modifier[j] -= animation_speed * 1.5f;
		modifier[j] = std::clamp(modifier[j], 0.f, 1.f);

		float cur_alpha = std::clamp(
			current_flags.clr.a() * easeOutQuad(modifier[j]),
			0.f, 255.f);

		float add = current_flags.icon ? 19.f : 10.f;

		float cur_step = std::clamp(
			add * easeOutQuad(modifier[j]),
			0.f, add);

		if (cur_alpha > 0.f) {
			current_flags.clr.SetAlpha(cur_alpha);

			Render::Get().RenderText(current_flags.name, ImVec2(ctx.bbox.right + 4 + ((ctx.pl->m_ArmorValue() > 0 && g_Options.esp_player_armour) ? 5 : 0), ctx.bbox.top - 2 + step), 14.f, ctx.pl->IsDormant() ? Color(200, 200, 200, ctx.alpha) : current_flags.clr, false, true, current_flags.icon ? g_IconsBigFont : g_pDefaultFont);
			step += cur_step;
		}
	}
}
//--------------------------------------------------------------------------------
void Visuals::RenderCrosshair()
{
	int w, h;

	g_EngineClient->GetScreenSize(w, h);

	int cx = w / 2;
	int cy = h / 2;
	Render::Get().RenderLine(cx - 25, cy, cx + 25, cy, Color(g_Options.color_esp_crosshair));
	Render::Get().RenderLine(cx, cy - 25, cx, cy + 25, Color(g_Options.color_esp_crosshair));
}
//--------------------------------------------------------------------------------
void Visuals::RenderWeapon(C_BaseCombatWeapon* ent)
{
	auto clean_item_name = [](const char* name) -> const char* {
		if (name[0] == 'C')
			name++;

		auto start = strstr(name, "Weapon");
		if (start != nullptr)
			name = start + 6;

		return name;
	};

	// We don't want to Render weapons that are being held
	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	Render::Get().RenderBox(bbox, Color(g_Options.color_esp_weapons));


	auto name = clean_item_name(ent->GetClientClass()->m_pNetworkName);

	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name);
	int w = bbox.right - bbox.left;


	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, Color(g_Options.color_esp_weapons));
}
//--------------------------------------------------------------------------------
void Visuals::RenderDefuseKit(C_BaseEntity* ent)
{
	if (ent->m_hOwnerEntity().IsValid())
		return;

	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;

	Render::Get().RenderBox(bbox, Color(g_Options.color_esp_defuse));

	auto name = "Defuse Kit";
	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name);
	int w = bbox.right - bbox.left;
	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, Color(g_Options.color_esp_defuse));
}
//--------------------------------------------------------------------------------
void Visuals::RenderPlantedC4(C_BaseEntity* ent)
{
	auto bbox = GetBBox(ent);

	if (bbox.right == 0 || bbox.bottom == 0)
		return;


	Render::Get().RenderBox(bbox, Color(g_Options.color_esp_c4));


	int bombTimer = std::ceil(ent->m_flC4Blow() - g_GlobalVars->curtime);
	std::string timer = std::to_string(bombTimer);

	auto name = (bombTimer < 0.f) ? "Bomb" : timer;
	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name.c_str());
	int w = bbox.right - bbox.left;

	Render::Get().RenderText(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, Color(g_Options.color_esp_c4));
}
//--------------------------------------------------------------------------------
void Visuals::RenderItemEsp(C_BaseEntity* ent)
{
	std::string itemstr = "Undefined";
	const model_t * itemModel = ent->GetModel();
	if (!itemModel)
		return;
	studiohdr_t * hdr = g_MdlInfo->GetStudiomodel(itemModel);
	if (!hdr)
		return;
	itemstr = hdr->szName;
	if (ent->GetClientClass()->m_ClassID == ClassId_CBumpMine)
		itemstr = "";
	else if (itemstr.find("case_pistol") != std::string::npos)
		itemstr = "Pistol Case";
	else if (itemstr.find("case_light_weapon") != std::string::npos)
		itemstr = "Light Case";
	else if (itemstr.find("case_heavy_weapon") != std::string::npos)
		itemstr = "Heavy Case";
	else if (itemstr.find("case_explosive") != std::string::npos)
		itemstr = "Explosive Case";
	else if (itemstr.find("case_tools") != std::string::npos)
		itemstr = "Tools Case";
	else if (itemstr.find("random") != std::string::npos)
		itemstr = "Airdrop";
	else if (itemstr.find("dz_armor_helmet") != std::string::npos)
		itemstr = "Full Armor";
	else if (itemstr.find("dz_helmet") != std::string::npos)
		itemstr = "Helmet";
	else if (itemstr.find("dz_armor") != std::string::npos)
		itemstr = "Armor";
	else if (itemstr.find("upgrade_tablet") != std::string::npos)
		itemstr = "Tablet Upgrade";
	else if (itemstr.find("briefcase") != std::string::npos)
		itemstr = "Briefcase";
	else if (itemstr.find("parachutepack") != std::string::npos)
		itemstr = "Parachute";
	else if (itemstr.find("dufflebag") != std::string::npos)
		itemstr = "Cash Dufflebag";
	else if (itemstr.find("ammobox") != std::string::npos)
		itemstr = "Ammobox";
	else if (itemstr.find("dronegun") != std::string::npos)
		itemstr = "Turrel";
	else if (itemstr.find("exojump") != std::string::npos)
		itemstr = "Exojump";
	else if (itemstr.find("healthshot") != std::string::npos)
		itemstr = "Healthshot";
	else {
		/*May be you will search some missing items..*/
		/*static std::vector<std::string> unk_loot;
		if (std::find(unk_loot.begin(), unk_loot.end(), itemstr) == unk_loot.end()) {
			Utils::ConsolePrint(itemstr.c_str());
			unk_loot.push_back(itemstr);
		}*/
		return;
	}
	
	auto bbox = GetBBox(ent);
	if (bbox.right == 0 || bbox.bottom == 0)
		return;
	auto sz = g_pDefaultFont->CalcTextSizeA(14.f, FLT_MAX, 0.0f, itemstr.c_str());
	int w = bbox.right - bbox.left;


	//Render::Get().RenderBox(bbox, Color(g_Options.color_esp_item);
	Render::Get().RenderText(itemstr, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, Color(g_Options.color_esp_item));
}
//--------------------------------------------------------------------------------
void Visuals::ThirdPerson() {
	if (!g_LocalPlayer)
		return;

	if (g_Options.misc_thirdperson && g_LocalPlayer->IsAlive())
		g_Input->m_fCameraInThirdPerson = true;
	else
		g_Input->m_fCameraInThirdPerson = false;
}


void Visuals::AddToDrawList() {
	for (auto i = 1; i <= g_EntityList->GetHighestEntityIndex(); ++i) {
		auto entity = C_BaseEntity::GetEntityByIndex(i);

		if (!entity)
			continue;
		
		if (entity == g_LocalPlayer && !g_Input->m_fCameraInThirdPerson)
			continue;

		if (i <= g_GlobalVars->maxClients) {
			auto player = Player();
			if (player.Begin((C_BasePlayer*)entity)) {
				if (g_Options.esp_player_snaplines) player.RenderSnapline();
				if (g_Options.esp_player_boxes)     player.RenderBox();
				if (g_Options.esp_player_weapons)   player.RenderWeaponName();
				if (g_Options.esp_player_names)     player.RenderName();
				if (g_Options.esp_player_health)    player.RenderHealth();
				if (g_Options.esp_player_armour)    player.RenderArmour();
				if (g_Options.esp_player_flags)		player.RenderFlags();
			}
		}
		else if (g_Options.esp_dropped_weapons && entity->IsWeapon())
			RenderWeapon(static_cast<C_BaseCombatWeapon*>(entity));
		else if (g_Options.esp_dropped_weapons && entity->IsDefuseKit())
			RenderDefuseKit(entity);
		else if (entity->IsPlantedC4() && g_Options.esp_planted_c4)
			RenderPlantedC4(entity);
		else if (entity->IsLoot() && g_Options.esp_items)
			RenderItemEsp(entity);
	}


	if (g_Options.esp_crosshair)
		RenderCrosshair();

}
