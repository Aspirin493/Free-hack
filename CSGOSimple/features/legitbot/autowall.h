#pragma once
#include "../../valve_sdk/sdk.hpp"
#include "../../valve_sdk/csgostructs.hpp"
namespace Autowall
{
	struct FireBulletData
	{
		Vector src;
		trace_t enter_trace;
		Vector direction;
		CTraceFilter filter;
		float trace_length;
		float trace_length_remaining;
		float current_damage;
		int penetrate_count;
	};
	float GetDamage(const Vector& vecPoint);
	bool SimulateFireBullet(C_BaseCombatWeapon* pWeapon, FireBulletData& data);
	bool HandleBulletPenetration(CCSWeaponInfo* weaponInfo, FireBulletData& data);
	bool TraceToExit(Vector& end, trace_t* enter_trace, Vector start, Vector dir, trace_t* exit_trace);
	bool DidHitNonWorldEntity(C_BasePlayer* player);
	void ScaleDamage(int hitbox, C_BasePlayer* enemy, float weapon_armor_ratio, float& current_damage);
	float GetHitgroupDamageMultiplier(int hitbox);
}