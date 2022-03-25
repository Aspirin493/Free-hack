#include "hooks.hpp"
#include <intrin.h>  

#include "render.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "helpers/input.hpp"
#include "helpers/utils.hpp"
#include "features/bhop.hpp"
#include "features/chams.hpp"
#include "features/visuals.hpp"
#include "features/glow.hpp"
#include "features/legitbot/aimbot.h"
#include "features/legitbot/backtrack.h"
#include "features/skins/changer.hpp"
#include "features/skins/knife_fix.hpp"
#include "features/prediction.hpp"
#include <mutex>
#pragma intrinsic(_ReturnAddress)  

namespace Hooks {

	void Initialize()
	{
		hlclient_hook.setup(g_CHLClient);
		direct3d_hook.setup(g_D3DDevice9);
		vguipanel_hook.setup(g_VGuiPanel);
		vguisurf_hook.setup(g_VGuiSurface);
		sound_hook.setup(g_EngineSound);
		mdlrender_hook.setup(g_MdlRender);
		clientmode_hook.setup(g_ClientMode);
		ConVar* sv_cheats_con = g_CVar->FindVar("sv_cheats");
		sv_cheats.setup(sv_cheats_con);
		gameevents_hook.setup(g_GameEvents);

		sequence_hook = new recv_prop_hook(C_BaseViewModel::m_nSequence(), hkRecvProxy);

		direct3d_hook.hook_index(index::EndScene, hkEndScene);
		direct3d_hook.hook_index(index::Reset, hkReset);
		hlclient_hook.hook_index(index::FrameStageNotify, hkFrameStageNotify);
		hlclient_hook.hook_index(index::CreateMove, hkCreateMove_Proxy);
		vguipanel_hook.hook_index(index::PaintTraverse, hkPaintTraverse);
		sound_hook.hook_index(index::EmitSound1, hkEmitSound1);
		vguisurf_hook.hook_index(index::LockCursor, hkLockCursor);
		vguisurf_hook.hook_index(index::LockCursor, hkSetDrawColor);
		mdlrender_hook.hook_index(index::DrawModelExecute, hkDrawModelExecute);
		clientmode_hook.hook_index(index::DoPostScreenSpaceEffects, hkDoPostScreenEffects);
		clientmode_hook.hook_index(index::OverrideView, hkOverrideView);
		sv_cheats.hook_index(index::SvCheatsGetBool, hkSvCheatsGetBool);
	}
	//--------------------------------------------------------------------------------
	void Shutdown()
	{
		hlclient_hook.unhook_all();
		direct3d_hook.unhook_all();
		vguipanel_hook.unhook_all();
		vguisurf_hook.unhook_all();
		mdlrender_hook.unhook_all();
		clientmode_hook.unhook_all();
		gameevents_hook.unhook_all();
		sequence_hook->~recv_prop_hook();
		sound_hook.unhook_all();
		sv_cheats.unhook_all();

		Glow::Get().Shutdown();
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkEndScene(IDirect3DDevice9* pDevice)
	{
		static auto oEndScene = direct3d_hook.get_original<decltype(&hkEndScene)>(index::EndScene);

		static auto viewmodel_fov = g_CVar->FindVar("viewmodel_fov");
		static auto mat_ambient_light_r = g_CVar->FindVar("mat_ambient_light_r");
		static auto mat_ambient_light_g = g_CVar->FindVar("mat_ambient_light_g");
		static auto mat_ambient_light_b = g_CVar->FindVar("mat_ambient_light_b");
		static auto crosshair_cvar = g_CVar->FindVar("crosshair");

		viewmodel_fov->m_fnChangeCallbacks.m_Size = 0;
		viewmodel_fov->SetValue(g_Options.viewmodel_fov);
		mat_ambient_light_r->SetValue(g_Options.mat_ambient_light_r);
		mat_ambient_light_g->SetValue(g_Options.mat_ambient_light_g);
		mat_ambient_light_b->SetValue(g_Options.mat_ambient_light_b);
		
		crosshair_cvar->SetValue(!(g_Options.esp_enabled && g_Options.esp_crosshair));

		DWORD colorwrite, srgbwrite;
		IDirect3DVertexDeclaration9* vert_dec = nullptr;
		IDirect3DVertexShader9* vert_shader = nullptr;
		DWORD dwOld_D3DRS_COLORWRITEENABLE = NULL;
		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &colorwrite);
		pDevice->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgbwrite);

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		//removes the source engine color correction
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

		pDevice->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->GetVertexDeclaration(&vert_dec);
		pDevice->GetVertexShader(&vert_shader);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		pDevice->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);

		
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


		auto esp_drawlist = Render::Get().RenderScene();

		Menu::Get().Render();
	

		ImGui::Render(esp_drawlist);

		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, colorwrite);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, srgbwrite);
		pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
		pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
		pDevice->SetVertexDeclaration(vert_dec);
		pDevice->SetVertexShader(vert_shader);

		return oEndScene(pDevice);
	}
	//--------------------------------------------------------------------------------
	long __stdcall hkReset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		static auto oReset = direct3d_hook.get_original<decltype(&hkReset)>(index::Reset);

		Menu::Get().OnDeviceLost();

		auto hr = oReset(device, pPresentationParameters);

		if (hr >= 0)
			Menu::Get().OnDeviceReset();

		return hr;
	}
	//--------------------------------------------------------------------------------
	void patch(PVOID address, const int type, const int bytes)
	{
		DWORD d, ds; //declared for future use.
		VirtualProtect(address, bytes, PAGE_EXECUTE_READWRITE, &d); //remove write protection!
		memset(address, type, bytes); //patch the data
		VirtualProtect(address, bytes, d, &ds); //set the write protection back to its normal state
	}
	//--------------------------------------------------------------------------------
	int max_choke_ticks() {

		int maxticks = (g_GameRules && g_GameRules->m_bIsValveDS()) ? 11 : 15;
		static int max_choke_ticks = 0;
		static int latency_ticks = 0;
		float fl_latency = g_EngineClient->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
		int latency = TIME_TO_TICKS(fl_latency);
		if (g_ClientState->m_nChokedCommands <= 0)
			latency_ticks = latency;
		else latency_ticks = std::max(latency, latency_ticks);

		if (fl_latency >= g_GlobalVars->interval_per_tick)
			max_choke_ticks = maxticks - latency_ticks;
		else max_choke_ticks = maxticks;
		return max_choke_ticks;
	}
	//--------------------------------------------------------------------------------
	void Fakelag(CUserCmd* cmd, bool& send_packet) {
		if (!g_Options.misc_fakelag_enabled)
			return;

		if (g_Options.misc_fakelag_unducking &&
			g_LocalPlayer->m_flDuckAmount() > 0.05f && g_LocalPlayer->m_flDuckAmount() < 0.95f) {
			send_packet = !(g_ClientState->m_nChokedCommands < max_choke_ticks());
			return;
		}

		if (g_Options.misc_fakelag_factor <= 0)
			return;

		int choke_factor = g_Options.misc_desync ? std::min(max_choke_ticks(), g_Options.misc_fakelag_factor) : g_Options.misc_fakelag_factor;

		auto LegitPeek = [choke_factor](CUserCmd* cmd, bool& send_packet) {
			static bool m_bIsPeeking = false;
			if (m_bIsPeeking) {
				send_packet = !(g_ClientState->m_nChokedCommands < choke_factor);
				if (send_packet)
					m_bIsPeeking = false;
				return;
			}

			auto speed = g_LocalPlayer->m_vecVelocity().Length();
			if (speed <= 70.0f)
				return;

			auto collidable = g_LocalPlayer->GetCollideable();

			Vector min, max;
			min = collidable->OBBMins();
			max = collidable->OBBMaxs();

			min += g_LocalPlayer->m_vecOrigin();
			max += g_LocalPlayer->m_vecOrigin();

			Vector center = (min + max) * 0.5f;

			for (int i = 1; i <= g_GlobalVars->maxClients; ++i) {
				auto player = C_BasePlayer::GetPlayerByIndex(i);
				if (!player || !player->IsAlive() || player->IsDormant())
					continue;
				if (player == g_LocalPlayer || g_LocalPlayer->m_iTeamNum() == player->m_iTeamNum())
					continue;

				auto weapon = player->m_hActiveWeapon().Get();
				if (!weapon || weapon->m_iClip1() <= 0)
					continue;

				auto weapon_data = weapon->GetCSWeaponData();
				if (!weapon_data || weapon_data->iWeaponType <= WEAPONTYPE_KNIFE || weapon_data->iWeaponType >= WEAPONTYPE_C4)
					continue;

				auto eye_pos = player->GetEyePos();

				Vector direction;
				Math::AngleVectors(player->m_angEyeAngles(), direction);
				direction.NormalizeInPlace();

				Vector hit_point;
				bool hit = Math::IntersectionBoundingBox(eye_pos, direction, min, max, &hit_point);
				if (hit && eye_pos.DistTo(hit_point) <= weapon_data->flRange) {
					Ray_t ray;
					trace_t tr;
					CTraceFilterSkipEntity filter((C_BasePlayer*)player);
					ray.Init(eye_pos, hit_point);

					g_EngineTrace->TraceRay(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &tr);
					if (tr.contents & CONTENTS_WINDOW) { // skip windows
																								// at this moment, we dont care about local player
						filter.pSkip = tr.hit_entity;
						ray.Init(tr.endpos, hit_point);
						g_EngineTrace->TraceRay(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &tr);
					}

					if (tr.fraction == 1.0f || tr.hit_entity == g_LocalPlayer) {
						m_bIsPeeking = true;
						break;
					}
				}
			}
		};

		auto speed = g_LocalPlayer->m_vecVelocity().Length();
		bool standing = speed <= 1.0f;
		if (!g_Options.misc_fakelag_standing && standing) {
			return;
		}

		if (!g_Options.misc_fakelag_moving && !standing) {
			return;
		}

		enum FakelagMode {
			FakelagStatic = 0,
			FakelagSwitch,
			FakelagAdaptive,
			FakelagRandom,
			FakelagLegitPeek
		};

		float UnitsPerTick = 0.0f;

		int WishTicks = 0;
		int AdaptiveTicks = 2;
		static int LastRandomNumber = 5;
		static int randomSeed = 12345;

		switch (g_Options.misc_fakelag_mode)
		{
		case FakelagSwitch:
			// apply same logic as static fakelag
			if (cmd->command_number % 30 > 15)
				break;
		case FakelagStatic:
			send_packet = !(g_ClientState->m_nChokedCommands < choke_factor);
			break;
		case FakelagAdaptive:
			if (standing) {
				send_packet = !(g_ClientState->m_nChokedCommands < choke_factor);
				break;
			}

			UnitsPerTick = g_LocalPlayer->m_vecVelocity().Length() * g_GlobalVars->interval_per_tick;
			while ((WishTicks * UnitsPerTick) <= 68.0f)
			{
				if (((AdaptiveTicks - 1) * UnitsPerTick) > 68.0f)
				{
					++WishTicks;
					break;
				}
				if ((AdaptiveTicks * UnitsPerTick) > 68.0f)
				{
					WishTicks += 2;
					break;
				}
				if (((AdaptiveTicks + 1) * UnitsPerTick) > 68.0f)
				{
					WishTicks += 3;
					break;
				}
				if (((AdaptiveTicks + 2) * UnitsPerTick) > 68.0f)
				{
					WishTicks += 4;
					break;
				}
				AdaptiveTicks += 5;
				WishTicks += 5;
				if (AdaptiveTicks > 16)
					break;
			}

			send_packet = !(g_ClientState->m_nChokedCommands < WishTicks);
			break;
		case FakelagRandom:
			if (g_ClientState->m_nChokedCommands < LastRandomNumber) {
				send_packet = false;
			}
			else {
				randomSeed = 0x41C64E6D * randomSeed + 12345;
				LastRandomNumber = (randomSeed / 0x10000 & 0x7FFFu) % choke_factor;
				send_packet = true;
			}
			break;
		case FakelagLegitPeek:
			LegitPeek(cmd, send_packet);
			break;
		}

		if (choke_factor < g_ClientState->m_nChokedCommands)
			send_packet = true;
	};
	//--------------------------------------------------------------------------------
	void Desync(CUserCmd* cmd, bool& bSendPacket)
	{
		if (!g_Options.misc_desync)
			return;
		if (cmd->buttons & (IN_ATTACK | IN_ATTACK2 | IN_USE) ||
			g_LocalPlayer->m_nMoveType() == MOVETYPE_LADDER || g_LocalPlayer->m_nMoveType() == MOVETYPE_NOCLIP
			|| !g_LocalPlayer->IsAlive())
			return;
		if (g_GameRules && (g_GameRules)->m_bFreezePeriod())
			return;
		auto weapon = g_LocalPlayer->m_hActiveWeapon().Get();
		if (!weapon)
			return;

		auto weapon_index = weapon->m_Item().m_iItemDefinitionIndex();
		if ((weapon_index == WEAPON_GLOCK || weapon_index == WEAPON_FAMAS) && weapon->m_flNextPrimaryAttack() >= g_GlobalVars->curtime)
			return;

		auto weapon_data = weapon->GetCSWeaponData();

		if (weapon_data->iWeaponType == WEAPONTYPE_GRENADE) {
			if (!weapon->m_bPinPulled()) {
				float throwTime = weapon->m_fThrowTime();
				if (throwTime > 0.f)
					return;
			}

			if ((cmd->buttons & IN_ATTACK) || (cmd->buttons & IN_ATTACK2)) {
				if (weapon->m_fThrowTime() > 0.f)
					return;
			}
		}

		int side = 1;
		side = GetKeyState(g_Options.misc_desync_key) ? -1 : 1;

		float minimal_move = g_LocalPlayer->m_fFlags() & IN_DUCK ? 3.0f : 1.0f;

		if (!bSendPacket) {
			cmd->viewangles.yaw += 58.f * side;
		}

		static bool flip = 1;
		flip = !flip;

		cmd->sidemove += flip ? minimal_move : -minimal_move;

		Math::FixAngles(cmd->viewangles);
	}
	//--------------------------------------------------------------------------------
	float next_lby = 0;
	//--------------------------------------------------------------------------------
	float AngleDiff(float destAngle, float srcAngle) {
		float delta;

		delta = fmodf(destAngle - srcAngle, 360.0f);
		if (destAngle > srcAngle) {
			if (delta >= 180)
				delta -= 360;
		}
		else {
			if (delta <= -180)
				delta += 360;
		}
		return delta;
	}
	//--------------------------------------------------------------------------------
	void updatelby(CCSGOPlayerAnimState* animstate)
	{
		if (animstate->speed_2d > 0.1f || std::fabsf(animstate->flUpVelocity)) {
			next_lby = g_GlobalVars->curtime + 0.22f;
		}
		else if (g_GlobalVars->curtime > next_lby) {
			if (std::fabsf(AngleDiff(animstate->m_flGoalFeetYaw, animstate->m_flEyeYaw)) > 35.0f) {
				next_lby = g_GlobalVars->curtime + 1.1f;
			}
		}
	}
	//--------------------------------------------------------------------------------
	int32_t unpredflags;
	//--------------------------------------------------------------------------------
	void __stdcall hkCreateMove(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket)
	{
		static auto oCreateMove = hlclient_hook.get_original<decltype(&hkCreateMove_Proxy)>(index::CreateMove);

		oCreateMove(g_CHLClient, 0, sequence_number, input_sample_frametime, active);

		auto cmd = g_Input->GetUserCmd(sequence_number);
		auto verified = g_Input->GetVerifiedCmd(sequence_number);

		if (!cmd || !cmd->command_number)
			return;
		if (!g_LocalPlayer)								
			return;

		if (Menu::Get().IsVisible())
			cmd->buttons &= ~IN_ATTACK;

		static float SpawnTime = 0.0f;
		if (g_LocalPlayer->IsAlive() && g_LocalPlayer->m_flSpawnTime() != SpawnTime) {
			g_AnimState.pBaseEntity = g_LocalPlayer;
			g_LocalPlayer->ResetAnimationState(&g_AnimState);
			SpawnTime = g_LocalPlayer->m_flSpawnTime();
		}

		unpredangle = cmd->viewangles;
		unpredflags = g_LocalPlayer->m_fFlags();

		if (g_Options.misc_bhop)
			BunnyHop::OnCreateMove(cmd);

		CPredictionSystem::Get().StartPrediction(g_LocalPlayer, cmd); {
			if (cmd->command_number % 2 == 1 && bSendPacket && g_Options.misc_desync)
				bSendPacket = false;

			if (g_Options.misc_edgejump && GetAsyncKeyState(g_Options.misc_edgejump_key))
			{
				if ((unpredflags & FL_ONGROUND) && !(g_LocalPlayer->m_fFlags() & FL_ONGROUND))
					cmd->buttons |= IN_JUMP;
			}

			if (g_Options.misc_autostrafe)
				BunnyHop::AutoStrafe(cmd, unpredangle);

			g_Aimbot.OnMove(cmd);
			g_Backtrack.OnMove(cmd);

			if (g_Options.misc_desync && g_ClientState->m_nChokedCommands >= max_choke_ticks()) {
				bSendPacket = true;
			}

			if (g_Options.misc_desync && std::fabsf(g_LocalPlayer->m_flSpawnTime() - g_GlobalVars->curtime) > 1.0f)
				Desync(cmd, bSendPacket);

			if (!g_Options.misc_antiuntrusted && g_Options.misc_noduckcooldown)
				cmd->buttons |= IN_BULLRUSH;

			auto anim_state = g_LocalPlayer->GetPlayerAnimState();
			if (anim_state) {
				CCSGOPlayerAnimState anim_state_backup = *anim_state;
				*anim_state = g_AnimState;
				g_LocalPlayer->GetVAngles() = cmd->viewangles;
				g_LocalPlayer->UpdateClientSideAnimation();

				updatelby(anim_state);

				g_AnimState = *anim_state;
				*anim_state = anim_state_backup;
			}
			if (g_LocalPlayer->m_nMoveType() != MOVETYPE_LADDER)
				Math::MovementFix(cmd, unpredangle);
		}
		CPredictionSystem::Get().EndPrediction(g_LocalPlayer, cmd);

		// https://github.com/spirthack/CSGOSimple/issues/69
		if (g_Options.misc_showranks && cmd->buttons & IN_SCORE) // rank revealer will work even after unhooking, idk how to "hide" ranks  again
			g_CHLClient->DispatchUserMessage(CS_UM_ServerRankRevealAll, 0, 0, nullptr);

		if (!g_EngineClient->IsVoiceRecording())
			Fakelag(cmd, bSendPacket);

		if (g_Options.misc_antiuntrusted)
			Math::FixAngles(cmd->viewangles);

		verified->m_cmd = *cmd;
		verified->m_crc = cmd->GetChecksum();
	}
	//--------------------------------------------------------------------------------
	__declspec(naked) void __fastcall hkCreateMove_Proxy(void* _this, int, int sequence_number, float input_sample_frametime, bool active)
	{
		__asm
		{
			push ebp
			mov  ebp, esp
			push ebx; not sure if we need this
			push esp
			push dword ptr[active]
			push dword ptr[input_sample_frametime]
			push dword ptr[sequence_number]
			call Hooks::hkCreateMove
			pop  ebx
			pop  ebp
			retn 0Ch
		}
	}
	//--------------------------------------------------------------------------------
	bool consoleCurrentlyDrawn = false;
	//--------------------------------------------------------------------------------
	void CustomConsole(vgui::VPANEL nPanel)
	{
		if (!g_Options.misc_customconsole)
			return;
		
		static bool bShouldRecolorConsole;
		static IMaterial* cMaterial[5];
		if (!cMaterial[0] || !cMaterial[1] || !cMaterial[2] || !cMaterial[3] || !cMaterial[4])
		{
			for (MaterialHandle_t i = g_MatSystem->FirstMaterial(); i != g_MatSystem->InvalidMaterial(); i = g_MatSystem->NextMaterial(i))
			{
				auto pMaterial = g_MatSystem->GetMaterial(i);
				if (!pMaterial) continue;

				if (strstr(pMaterial->GetName(), "vgui_white"))
					cMaterial[0] = pMaterial;
				else if (strstr(pMaterial->GetName(), "800corner1"))
					cMaterial[1] = pMaterial;
				else if (strstr(pMaterial->GetName(), "800corner2"))
					cMaterial[2] = pMaterial;
				else if (strstr(pMaterial->GetName(), "800corner3"))
					cMaterial[3] = pMaterial;
				else if (strstr(pMaterial->GetName(), "800corner4"))
					cMaterial[4] = pMaterial;
			}
		} // Couldn't use find material because for some reason "vgui_white" doesn't exist...
		else
		{
			if (g_VGuiPanel->GetName(nPanel) != "MatSystemTopPanel") // You should check for some other panels name that shouldn't be recolored. Not bother fixing it for all.
			{
				if (g_EngineClient->Con_IsVisible())
				{
					bShouldRecolorConsole = true;
					if (bShouldRecolorConsole)
					{
						for (int num = 0; num < 5; num++)
						{
							cMaterial[num]->ColorModulate(g_Options.color_misc_customconsole[0], g_Options.color_misc_customconsole[1], g_Options.color_misc_customconsole[2]);
							cMaterial[num]->AlphaModulate(g_Options.color_misc_customconsole[3]);
						}
					}
				}
			}
			else if (bShouldRecolorConsole)
			{
				for (int num = 0; num < 5; num++)
				{
					cMaterial[num]->ColorModulate(1.f, 1.f, 1.f);
					cMaterial[num]->AlphaModulate(1.0f);
				}
				bShouldRecolorConsole = false;
			}
		}
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkPaintTraverse(void* _this, int edx, vgui::VPANEL panel, bool forceRepaint, bool allowForce)
	{
		static auto panelId = vgui::VPANEL{ 0 };
		static auto oPaintTraverse = vguipanel_hook.get_original<decltype(&hkPaintTraverse)>(index::PaintTraverse);

		oPaintTraverse(g_VGuiPanel, edx, panel, forceRepaint, allowForce);

		if (!panelId) {
			const auto panelName = g_VGuiPanel->GetName(panel);
			if (!strcmp(panelName, "FocusOverlayPanel")) {
				panelId = panel;
			}
		}
		else if (panelId == panel) 
		{
			//Ignore 50% cuz it called very often
			static bool bSkip = false;
			bSkip = !bSkip;

			if (bSkip)
				return;

			Render::Get().BeginScene();
		}

		CustomConsole(panel);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkEmitSound1(void* _this, int edx, IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char *pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk) {
		static auto ofunc = sound_hook.get_original<decltype(&hkEmitSound1)>(index::EmitSound1);


		if (!strcmp(pSoundEntry, "UIPanorama.popup_accept_match_beep")) {
			static auto fnAccept = reinterpret_cast<bool(__stdcall*)(const char*)>(Utils::PatternScan(GetModuleHandleA("client.dll"), "55 8B EC 83 E4 F8 8B 4D 08 BA ? ? ? ? E8 ? ? ? ? 85 C0 75 12"));

			if (fnAccept) {

				fnAccept("");

				//This will flash the CSGO window on the taskbar
				//so we know a game was found (you cant hear the beep sometimes cause it auto-accepts too fast)
				FLASHWINFO fi;
				fi.cbSize = sizeof(FLASHWINFO);
				fi.hwnd = InputSys::Get().GetMainWindow();
				fi.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
				fi.uCount = 0;
				fi.dwTimeout = 0;
				FlashWindowEx(&fi);
			}
		}

		ofunc(g_EngineSound, edx, filter, iEntIndex, iChannel, pSoundEntry, nSoundEntryHash, pSample, flVolume, nSeed, flAttenuation, iFlags, iPitch, pOrigin, pDirection, pUtlVecOrigins, bUpdatePositions, soundtime, speakerentity, unk);

	}
	//--------------------------------------------------------------------------------
	int __fastcall hkDoPostScreenEffects(void* _this, int edx, int a1)
	{
		static auto oDoPostScreenEffects = clientmode_hook.get_original<decltype(&hkDoPostScreenEffects)>(index::DoPostScreenSpaceEffects);

		if (g_LocalPlayer && g_Options.glow_enabled)
			Glow::Get().Run();

		return oDoPostScreenEffects(g_ClientMode, edx, a1);
	}
	//--------------------------------------------------------------------------------
	static bool map_changed = false;
	//--------------------------------------------------------------------------------
	void __fastcall hkFrameStageNotify(void* _this, int edx, ClientFrameStage_t stage)
	{
		static auto ofunc = hlclient_hook.get_original<decltype(&hkFrameStageNotify)>(index::FrameStageNotify);
		if (g_EngineClient->IsInGame()) {
			if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
				g_SkinChanger->OnFrameStageNotify(false);
			else if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END)
				g_SkinChanger->OnFrameStageNotify(true);
		}

		if (stage == FRAME_RENDER_START)
		{
			if (g_GameRules == nullptr)
				g_GameRules = *(CGameRules**)(Utils::PatternScan(GetModuleHandleA("client.dll"), "A1 ? ? ? ? 8B 0D ? ? ? ? 6A 00 68 ? ? ? ? C6") + 0x1);
			//if (g_Input->m_fCameraInThirdPerson)
			//{
			//	static auto deadflag = NetvarSys::Get().GetOffset("DT_BasePlayer", "deadflag");
			//	*(Vector*)((DWORD)g_LocalPlayer + deadflag + 0x4) = “‚ÓÈ”„ÓÎ»Á¿¿;
			//}
		}

		// may be u will use it lol
		ofunc(g_CHLClient, edx, stage);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkOverrideView(void* _this, int edx, CViewSetup* vsView)
	{
		static auto ofunc = clientmode_hook.get_original<decltype(&hkOverrideView)>(index::OverrideView);

		if (g_EngineClient->IsInGame() && vsView)
			Visuals::Get().ThirdPerson();

		ofunc(g_ClientMode, edx, vsView);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkLockCursor(void* _this)
	{
		static auto ofunc = vguisurf_hook.get_original<decltype(&hkLockCursor)>(index::LockCursor);

		if (Menu::Get().IsVisible()) {
			g_VGuiSurface->UnlockCursor();
			g_InputSystem->ResetInputState();
			return;
		}
		ofunc(g_VGuiSurface);

	}
	//--------------------------------------------------------------------------------
	void __fastcall hkDrawModelExecute(void* _this, int edx, IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& pInfo, matrix3x4_t* pCustomBoneToWorld)
	{
		static auto ofunc = mdlrender_hook.get_original<decltype(&hkDrawModelExecute)>(index::DrawModelExecute);

		if (g_MdlRender->IsForcedMaterialOverride() &&
			!strstr(pInfo.pModel->szName, "arms") &&
			!strstr(pInfo.pModel->szName, "weapons/v_")) {
			return ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);
		}

		Chams::Get().OnDrawModelExecute(ctx, state, pInfo, pCustomBoneToWorld);

		ofunc(_this, edx, ctx, state, pInfo, pCustomBoneToWorld);

		g_MdlRender->ForcedMaterialOverride(nullptr);
	}

	//--------------------------------------------------------------------------------
	bool __fastcall hkSvCheatsGetBool(PVOID pConVar, void* edx)
	{
		static auto dwCAM_Think = Utils::PatternScan(GetModuleHandleW(L"client.dll"), "85 C0 75 30 38 86");
		static auto ofunc = sv_cheats.get_original<bool(__thiscall *)(PVOID)>(13);
		if (!ofunc)
			return false;

		if (reinterpret_cast<DWORD>(_ReturnAddress()) == reinterpret_cast<DWORD>(dwCAM_Think))
			return true;
		return ofunc(pConVar);
	}
	//--------------------------------------------------------------------------------
	void hkRecvProxy(const CRecvProxyData* pData, void* entity, void* output)
	{
		static auto oRecv = sequence_hook->get_original_function();
		if (g_LocalPlayer && g_LocalPlayer->IsAlive()) {
			const auto proxy_data = const_cast<CRecvProxyData*>(pData);
			const auto view_model = static_cast<C_BaseViewModel*>(entity);

			if (view_model && view_model->m_hOwner() && view_model->m_hOwner().IsValid()) {
				const auto owner = static_cast<C_BasePlayer*>(g_EntityList->GetClientEntityFromHandle(view_model->m_hOwner()));
				if (owner == g_EntityList->GetClientEntity(g_EngineClient->GetLocalPlayer())) {
					const auto view_model_weapon_handle = view_model->m_hWeapon();
					if (view_model_weapon_handle.IsValid()) {
						const auto view_model_weapon = static_cast<C_BaseAttributableItem*>(g_EntityList->GetClientEntityFromHandle(view_model_weapon_handle));
						if (view_model_weapon) {
							if (WeaponInfo.count(view_model_weapon->m_Item().m_iItemDefinitionIndex())) {
								auto original_sequence = proxy_data->m_Value.m_Int;
								const auto override_model = WeaponInfo.at(view_model_weapon->m_Item().m_iItemDefinitionIndex()).model;
								proxy_data->m_Value.m_Int = g_KnifeAnimFix->Fix(override_model, proxy_data->m_Value.m_Int);
							}
						}
					}
				}
			}

		}

		oRecv(pData, entity, output);
	}
	//--------------------------------------------------------------------------------
	void __fastcall hkSetDrawColor(void* ecx, void* edx, int r, int g, int b, int a) {
		static auto originalSetDrawColor = vguisurf_hook.get_original<decltype(&hkSetDrawColor)>(index::SetDrawColor);
		if (consoleCurrentlyDrawn && g_Options.misc_customconsole) {
			// New colors
			Color newcolor = Color(g_Options.color_misc_customconsole);
			int newR = newcolor.r();
			int newG = newcolor.g();
			int newB = newcolor.b();
			int newA = newcolor.a();
			originalSetDrawColor(ecx, edx, newR, newG, newB, newA);
			return;
		}

		// Your other set draw color stuff

		originalSetDrawColor(ecx, edx, r, g, b, a);
	}
}
