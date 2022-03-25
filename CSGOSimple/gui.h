#pragma once
#include "imgui\imgui.h"
#include "imgui\impl\imgui_impl_dx9.h"
#include "imgui\impl\imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>
class gui
{
public:
    void set_style()
    {
        ImGuiStyle style;
        style.Colors[ImGuiCol_WindowBg] = ImColor(30, 30, 30);
        style.Colors[ImGuiCol_CheckMark] = ImColor(158, 0, 255);
        style.Colors[ImGuiCol_FrameBg] = ImColor(43, 43, 45);
        style.Colors[ImGuiCol_FrameBgHovered] = ImColor(46, 45, 48);
        style.Colors[ImGuiCol_FrameBgActive] = ImColor(49, 48, 49);
        style.Colors[ImGuiCol_Header] = ImColor(43, 43, 45);
        style.Colors[ImGuiCol_HeaderHovered] = ImColor(46, 45, 48);
        style.Colors[ImGuiCol_HeaderActive] = ImColor(49, 48, 49);
        style.Colors[ImGuiCol_Button] = ImColor(43, 43, 45);
        style.Colors[ImGuiCol_ButtonHovered] = ImColor(45, 45, 45);
        style.Colors[ImGuiCol_ButtonActive] = ImColor(47, 46, 48);
        style.Colors[ImGuiCol_ScrollbarBg] = ImColor(29, 29, 29);
        style.Colors[ImGuiCol_ScrollbarGrab] = ImColor(43, 43, 45);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(45, 45, 47);
        style.Colors[ImGuiCol_ScrollbarGrabActive] = ImColor(47, 47, 49);

        style.WindowBorderSize = 0;
        style.WindowRounding = 12;
        style.FrameBorderSize = 0;
        style.WindowBorderSize = 0;
        ImGui::GetStyle() = style;
    }
    void text_with_font(ImFont* font, const char* fmt, ...)
    {
        ImGui::PushFont(font);
        ImGui::Text(fmt);
        ImGui::PopFont();
    }
    void text_with_font(ImFont* font, ImColor color, const char* fmt, ...)
    {
        ImGui::PushFont(font);
        ImGui::TextColored(color, fmt);
        ImGui::PopFont();
    }
    void text_colored_with_font(ImFont* font, ImColor color, const char* fmt, ...)
    {
        ImGui::PushFont(font);
        ImGui::TextColored(color, fmt);
        ImGui::PopFont();
    }
    ImFont* SCALUX;
    ImFont* TABS;
    ImFont* VERY_SMALL;
    ImFont* BINDS_TITLE;
    ImFont* BINDS;
    ImFont* SUBTAB;
    ImFont* CONTAINER;
    IDirect3DTexture9* PLAYER_MODEL;
    IDirect3DTexture9* HELP;
    IDirect3DTexture9* AVATAR;
    IDirect3DTexture9* RAGE;
    IDirect3DTexture9* LEGIT;
    IDirect3DTexture9* VISUALS;
    IDirect3DTexture9* SKINS;
    IDirect3DTexture9* MISC;
    IDirect3DTexture9* CONFIGS;
    IDirect3DTexture9* RAGE_SELECTED;
    IDirect3DTexture9* LEGIT_SELECTED;
    IDirect3DTexture9* VISUALS_SELECTED;
    IDirect3DTexture9* SKINS_SELECTED;
    IDirect3DTexture9* MISC_SELECTED;
    IDirect3DTexture9* CONFIGS_SELECTED;
};
extern gui c_gui;
