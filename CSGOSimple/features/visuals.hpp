#pragma once

#include "../singleton.hpp"

#include "../render.hpp"
#include "../helpers/math.hpp"
#include "../valve_sdk/csgostructs.hpp"

struct Flags_t
{
	std::string name = "";
	bool toggled = false;
	bool icon = false;
	float modifier = 0.f;
	Color clr;

	Flags_t(std::string cur_name, bool toggle, Color color, bool is_icon = false) {
		name = cur_name;
		toggled = toggle;
		clr = color;
		icon = is_icon;
	}
};

class Visuals : public Singleton<Visuals>
{
	friend class Singleton<Visuals>;

	CRITICAL_SECTION cs;

	Visuals();
	~Visuals();
public:
	class Player
	{
	public:
		struct
		{
			C_BasePlayer* pl;
			bool          is_enemy;
			bool          is_visible;
			Color         clr;
			Vector        head_pos;
			Vector        feet_pos;
			RECT          bbox;
			int			  alpha;
		} ctx;

		std::vector<Flags_t> FlagsInfo[64] = {};

		bool Begin(C_BasePlayer * pl);
		void RenderBox();
		void RenderName();
		void RenderWeaponName();
		void RenderHealth();
		void RenderArmour();
		void RenderSnapline();
		void RenderFlags();
	};
	void RenderCrosshair();
	void RenderWeapon(C_BaseCombatWeapon* ent);
	void RenderDefuseKit(C_BaseEntity* ent);
	void RenderPlantedC4(C_BaseEntity* ent);
	void RenderItemEsp(C_BaseEntity* ent);
	void ThirdPerson();
	float flPlayerAlpha[65];
public:
	void AddToDrawList();
	void Render();
};
