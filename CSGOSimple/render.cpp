#include "render.hpp"

#include <mutex>

#include "features/visuals.hpp"
#include "valve_sdk/csgostructs.hpp"
#include "helpers/input.hpp"
#include "menu.hpp"
#include "options.hpp"
#include "fonts/fonts.hpp"
#include "helpers/math.hpp"
#include "gui.h"
#include "fonts/AtypDisplay-Regular.h"
#include "fonts/OpenSans-Bold.c"
#include "fonts/OpenSans-SemiBold.c"
#include "fonts/Tahoma.h"
#include "fonts/logo.h"
#include "hooks.hpp"
#include "fonts/undefeated.h"
#include "fonts/diggeo.h"
#include "fonts/avatar.h"
#include "fonts/icons/rage.h"
#include "fonts/icons/rage_selected.h"
#include "fonts/icons/legit.h"
#include "fonts/icons/legit_selected.h"
#include "fonts/icons/visuals.h"
#include "fonts/icons/visuals_selected.h"
#include "fonts/icons/misc.h"
#include "fonts/icons/misc_selected.h"
#include "fonts/icons/skins.h"
#include "fonts/icons/skins_selected.h"
#include "fonts/icons/configs.h"
#include "fonts/icons/configs_selected.h"
#include "fonts/icons/clarity_help-line.h"
#include "features/backdrop.hpp"

gui c_gui;

ImFont* g_pDefaultFont;
ImFont* g_pDefaultEspFont;
ImFont* g_pDefaultSmallFont;
ImFont* g_pDefaultMediumFont;
ImFont* g_pDefaultBigFont;
ImFont* g_pSecondFont;
ImFont* g_IconsFont;
ImFont* g_IconsBigFont;
ImFont* g_SegoeUI;
ImDrawListSharedData _data;

std::mutex render_mutex;

void Render::Initialize()
{
	ImGui::CreateContext();


	ImGui_ImplWin32_Init(InputSys::Get().GetMainWindow());
	ImGui_ImplDX9_Init(g_D3DDevice9);

	draw_list = new ImDrawList(ImGui::GetDrawListSharedData());
	draw_list_act = new ImDrawList(ImGui::GetDrawListSharedData());
	draw_list_rendering = new ImDrawList(ImGui::GetDrawListSharedData());

	GetFonts();
}

void Render::GetFonts() {
	// menu font
	ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
		Fonts::Droid_compressed_data,
		Fonts::Droid_compressed_size,
		14.f);

	// esp font
	g_pDefaultFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(OpenSans_Bold, sizeof(OpenSans_Bold), 14.f);
	g_pDefaultEspFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(OpenSans_Bold, sizeof(OpenSans_Bold), 12.f);
	g_pDefaultSmallFont = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(OpenSans_Bold, sizeof(OpenSans_Bold), 12.f);
	g_pDefaultMediumFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\verdana.ttf", 12);
	g_pDefaultBigFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\calibrib.ttf", 25.f);
	g_SegoeUI = ImGui::GetIO().Fonts->AddFontFromFileTTF(std::string("C:/Windows/Fonts/" + (std::string)"segoeui" + ".ttf").c_str(), 14, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	// font for watermark; just example
	g_pSecondFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
		Fonts::Cousine_compressed_data,
		Fonts::Cousine_compressed_size,
		18.f);

	g_IconsFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
		undefeated_compressed_data,
		undefeated_compressed_size,
		20.f, 
		0, 
		ImGui::GetIO().Fonts->GetGlyphRangesCyrillic()
	);

	g_IconsBigFont = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
		undefeated_compressed_data,
		undefeated_compressed_size,
		28.f, 
		0, 
		ImGui::GetIO().Fonts->GetGlyphRangesCyrillic()
	);

	if (c_gui.AVATAR == nullptr)
		D3DXCreateTextureFromFileInMemory(g_D3DDevice9
			, &Ellipse1, sizeof(Ellipse1), &c_gui.AVATAR);

	D3DXCreateTextureFromFileInMemory(g_D3DDevice9
		, &clarityhelpline, sizeof(clarityhelpline), &c_gui.HELP);

	c_gui.SCALUX = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(AtypDisplayRegular, sizeof(AtypDisplayRegular), 20);
	c_gui.TABS = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(AtypDisplayRegular, sizeof(AtypDisplayRegular), 12);
	c_gui.VERY_SMALL = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(AtypDisplayRegular, sizeof(AtypDisplayRegular), 10);
	c_gui.BINDS_TITLE = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(AtypDisplayRegular, sizeof(AtypDisplayRegular), 13);
	c_gui.BINDS = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(AtypDisplayRegular, sizeof(AtypDisplayRegular), 12);
	c_gui.SUBTAB = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(AtypDisplayRegular, sizeof(AtypDisplayRegular), 18);
	c_gui.CONTAINER = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(AtypDisplayRegular, sizeof(AtypDisplayRegular), 12);

	D3DXCreateTextureFromFileInMemory(g_D3DDevice9
		, &rage, sizeof(rage), &c_gui.RAGE);

	D3DXCreateTextureFromFileInMemory(g_D3DDevice9
		, &rageselected, sizeof(rageselected), &c_gui.RAGE_SELECTED);

	D3DXCreateTextureFromFileInMemory(g_D3DDevice9
		, &legit, sizeof(legit), &c_gui.LEGIT);

	D3DXCreateTextureFromFileInMemory(g_D3DDevice9
		, &legitselected, sizeof(legitselected), &c_gui.LEGIT_SELECTED);

	D3DXCreateTextureFromFileInMemory(g_D3DDevice9
		, &visuals, sizeof(visuals), &c_gui.VISUALS);

	D3DXCreateTextureFromFileInMemory(g_D3DDevice9
		, &visualsselected, sizeof(visualsselected), &c_gui.VISUALS_SELECTED);

	D3DXCreateTextureFromFileInMemory(g_D3DDevice9
		, &skins, sizeof(skins), &c_gui.SKINS);

	D3DXCreateTextureFromFileInMemory(g_D3DDevice9
		, &skinsselected, sizeof(skinsselected), &c_gui.SKINS_SELECTED);

	D3DXCreateTextureFromFileInMemory(g_D3DDevice9
		, &misc, sizeof(misc), &c_gui.MISC);

	D3DXCreateTextureFromFileInMemory(g_D3DDevice9
		, &miscselected, sizeof(miscselected), &c_gui.MISC_SELECTED);

	D3DXCreateTextureFromFileInMemory(g_D3DDevice9
		, &configs, sizeof(configs), &c_gui.CONFIGS);

	D3DXCreateTextureFromFileInMemory(g_D3DDevice9
		, &configsselected, sizeof(configsselected), &c_gui.CONFIGS_SELECTED);
}

void Render::ClearDrawList() {
	render_mutex.lock();
	draw_list_act->Clear();
	render_mutex.unlock();
}

void Render::BeginScene() {
	draw_list->Clear();
	draw_list->PushClipRectFullScreen();

	if (g_Options.misc_watermark)
		Render::Get().RenderText("amnesia.host", 10, 5, 18.f, g_Options.color_watermark, false, true, g_pSecondFont);

	int w, h;
	g_EngineClient->GetScreenSize(w, h);

	static int alpha = 0;
	if (Menu::Get().IsVisible() && alpha < 130)
		alpha += 5;
	else if (!Menu::Get().IsVisible() && alpha > 0)
		alpha -= 5;

	bool should_draw = false;
	if (Menu::Get().IsVisible())
		should_draw = true;
	else if (alpha <= 0)
		should_draw = false;

	if (should_draw)
		draw_list->AddRectFilled(ImVec2(0, 0), ImVec2(w, h), ImColor(0, 0, 0, alpha));

	if (Menu::Get().IsVisible() && alpha > 65)
		BackDrop::DrawBackDrop();

	if (g_EngineClient->IsInGame() && g_LocalPlayer && g_Options.esp_enabled)
		Visuals::Get().AddToDrawList();

	render_mutex.lock();
	*draw_list_act = *draw_list;
	render_mutex.unlock();
}

ImDrawList* Render::RenderScene() {

	if (render_mutex.try_lock()) {
		*draw_list_rendering = *draw_list_act;
		render_mutex.unlock();
	}

	return draw_list_rendering;
}


float Render::RenderText(const std::string& text, ImVec2 pos, float size, Color color, bool center, bool outline, ImFont* pFont)
{
	ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());
	if (!pFont->ContainerAtlas) return 0.f;
	draw_list->PushTextureID(pFont->ContainerAtlas->TexID);

	Color outline_clr = Color(0, 0, 0, 76.5);
	auto outline_clr_u32 = GetU32(outline_clr);

	if (center)
		pos.x -= textSize.x / 2.0f;

	if (outline) {
		draw_list->AddText(pFont, size, ImVec2(pos.x + 1, pos.y - 1), outline_clr_u32, text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x - 1, pos.y + 1), outline_clr_u32, text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x - 1, pos.y - 1), outline_clr_u32, text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x + 1, pos.y + 1), outline_clr_u32, text.c_str());

		draw_list->AddText(pFont, size, ImVec2(pos.x, pos.y + 1), outline_clr_u32, text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x, pos.y - 1), outline_clr_u32, text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x + 1, pos.y), outline_clr_u32, text.c_str());
		draw_list->AddText(pFont, size, ImVec2(pos.x - 1, pos.y), outline_clr_u32, text.c_str());
	}

	draw_list->AddText(pFont, size, pos, GetU32(color), text.c_str());

	draw_list->PopTextureID();

	return pos.y + textSize.y;
}

void Render::RenderCircle3D(Vector position, float points, float radius, Color color)
{
	float step = (float)M_PI * 2.0f / points;

	for (float a = 0; a < (M_PI * 2.0f); a += step)
	{
		Vector start(radius * cosf(a) + position.x, radius * sinf(a) + position.y, position.z);
		Vector end(radius * cosf(a + step) + position.x, radius * sinf(a + step) + position.y, position.z);

		Vector start2d, end2d;
		if (g_DebugOverlay->ScreenPosition(start, start2d) || g_DebugOverlay->ScreenPosition(end, end2d))
			return;

		RenderLine(start2d.x, start2d.y, end2d.x, end2d.y, color);
	}
}
