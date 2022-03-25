#include "bhop.hpp"

#include "../valve_sdk/csgostructs.hpp"
#include "../options.hpp"
#include "../helpers/math.hpp"

#include <algorithm>

#define CheckIfNonValidNumber(x) (fpclassify(x) == FP_INFINITE || fpclassify(x) == FP_NAN || fpclassify(x) == FP_SUBNORMAL)
QAngle unpredangle;

void BunnyHop::OnCreateMove(CUserCmd* cmd)
{
  static bool jumped_last_tick = false;
  static bool should_fake_jump = false;

  if (!g_LocalPlayer)
	  return;

  if (!g_LocalPlayer->IsAlive())
	  return;

  if (g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP)
	  return;

  if (g_LocalPlayer->m_fFlags() & FL_INWATER)
	  return;

  if(!jumped_last_tick && should_fake_jump) {
    should_fake_jump = false;
    cmd->buttons |= IN_JUMP;
  } else if(cmd->buttons & IN_JUMP) {
    if(g_LocalPlayer->m_fFlags() & FL_ONGROUND) {
      jumped_last_tick = true;
      should_fake_jump = true;
    } else {
      cmd->buttons &= ~IN_JUMP;
      jumped_last_tick = false;
    }
  } else {
    jumped_last_tick = false;
    should_fake_jump = false;
  }
}

void BunnyHop::AutoStrafe(CUserCmd* cmd, QAngle va) {
	if (!g_LocalPlayer || !g_LocalPlayer->IsAlive())
		return;

	if (g_Options.misc_autostrafe_type == 0)
	{
        if (g_LocalPlayer->m_nMoveType() != MOVETYPE_WALK)
            return;

		if (!g_InputSystem->IsButtonDown(ButtonCode_t::KEY_SPACE) ||
			g_InputSystem->IsButtonDown(ButtonCode_t::KEY_A) ||
			g_InputSystem->IsButtonDown(ButtonCode_t::KEY_D) ||
			g_InputSystem->IsButtonDown(ButtonCode_t::KEY_S) ||
			g_InputSystem->IsButtonDown(ButtonCode_t::KEY_W))
			return;

		bool on_ground = (g_LocalPlayer->m_fFlags() & FL_ONGROUND) && !(cmd->buttons & IN_JUMP);
		if (on_ground) {
			return;
		}

		static auto side = 1.0f;
		side = -side;

		auto velocity = g_LocalPlayer->m_vecVelocity();
		velocity.z = 0.0f;

		QAngle wish_angle = cmd->viewangles;

		auto speed = velocity.Length2D();
		auto ideal_strafe = std::clamp(RAD2DEG(atan(15.f / speed)), 0.0f, 90.0f);

		if (cmd->forwardmove > 0.0f)
			cmd->forwardmove = 0.0f;

		static auto cl_sidespeed = g_CVar->FindVar("cl_sidespeed");

		static float old_yaw = 0.f;
		auto yaw_delta = std::remainderf(wish_angle.yaw - old_yaw, 360.0f);
		auto abs_angle_delta = abs(yaw_delta);
		old_yaw = wish_angle.yaw;

		if (abs_angle_delta <= ideal_strafe || abs_angle_delta >= 30.0f) {
			QAngle velocity_direction;
			Math::VectorAngles(velocity, velocity_direction);
			auto velocity_delta = std::remainderf(wish_angle.yaw - velocity_direction.yaw, 360.0f);
			auto retrack = std::clamp(RAD2DEG(atan(30.0f / speed)), 0.0f, 90.0f) * g_Options.misc_autostrafe_retrack;
			if (velocity_delta <= retrack || speed <= 15.0f) {
				if (-(retrack) <= velocity_delta || speed <= 15.0f) {
					wish_angle.yaw += side * ideal_strafe;
					cmd->sidemove = cl_sidespeed->GetFloat() * side;
				}
				else {
					wish_angle.yaw = velocity_direction.yaw - retrack;
					cmd->sidemove = cl_sidespeed->GetFloat();
				}
			}
			else {
				wish_angle.yaw = velocity_direction.yaw + retrack;
				cmd->sidemove = -cl_sidespeed->GetFloat();
			}

			Math::MovementFix(cmd, wish_angle, cmd->viewangles);
		}
		else if (yaw_delta > 0.0f) {
			cmd->sidemove = -cl_sidespeed->GetFloat();
		}
		else if (yaw_delta < 0.0f) {
			cmd->sidemove = cl_sidespeed->GetFloat();
		}
	}
	else
	{
		if (cmd->buttons & IN_SPEED || g_LocalPlayer->m_vecVelocity().Length2D() < 10.f)
			return; // doesn't allow strafe when you hold shift and you're not moving
		if (g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP)
			return;
		if (g_LocalPlayer->m_fFlags() & FL_INWATER)
			return;

		static float yaw_add = 0.f;
		static const auto cl_sidespeed = g_CVar->FindVar("cl_sidespeed");
		if (!(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
		{
			bool back = cmd->buttons & IN_BACK;
			bool forward = cmd->buttons & IN_FORWARD;
			bool right = cmd->buttons & IN_MOVELEFT;
			bool left = cmd->buttons & IN_MOVERIGHT;

			if (back) {
				yaw_add = -180.f;
				if (right)
					yaw_add -= 45.f;
				else if (left)
					yaw_add += 45.f;
			}
			else if (right) {
				yaw_add = 90.f;
				if (back)
					yaw_add += 45.f;
				else if (forward)
					yaw_add -= 45.f;
			}
			else if (left) {
				yaw_add = -90.f;
				if (back)
					yaw_add -= 45.f;
				else if (forward)
					yaw_add += 45.f;
			}
			else {
				yaw_add = 0.f;
			}

			unpredangle.yaw += yaw_add;
			cmd->forwardmove = 0.f;
			cmd->sidemove = 0.f;

			const auto delta = Math::NormalizeYaw(unpredangle.yaw - RAD2DEG(atan2(g_LocalPlayer->m_vecVelocity().y, g_LocalPlayer->m_vecVelocity().x)));

			cmd->sidemove = delta > 0.f ? -cl_sidespeed->GetFloat() : cl_sidespeed->GetFloat();

			unpredangle.yaw = Math::NormalizeYaw(unpredangle.yaw - delta);
		}
	}
}