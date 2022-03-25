//
// Created by chrshnv on 6/29/2021.
//

#include "imgui_extended.h"
#include "..\menu.hpp"
#include <Windows.h>

namespace imgui_ex {
    std::vector<keybind_t> binds;
}

void imgui_ex::wrapper()
{
    for (auto& bnd : binds)
    {
        if (bnd.key && bnd.bind_mode == 0)
            bnd.is_active = GetAsyncKeyState(bnd.key);
        else if (bnd.key && bnd.bind_mode == 1)
            bnd.is_active = GetKeyState(bnd.key);
    }
}

bool imgui_ex::insert_checkbox( const char *name, bool *var, const char* unique_id ) {
    auto ret = ImGui::Checkbox2(name, var);
    if (ImGui::BeginPopup(unique_id, ImGuiWindowFlags_NoMove)) {
        bind_popup(FnvHash("bool"), var, name);
        ImGui::EndPopup();
    }

    if (ImGui::IsItemHovered() && ImGui::GetIO().MouseClicked[1])
        ImGui::OpenPopup(unique_id);

    for (auto& bnd : binds)
    {
        if (bnd.config_name == name)
        {
            if (bnd.is_active)
                var = reinterpret_cast<bool*>(&bnd.data);
        }
    }

    return ret;
}

static bool waiting_key = false;
void imgui_ex::key_bind( const std::string& name, keybind_t *bind ) {
    if( !bind )
        return;

    int key = bind->key;

    std::string status = waiting_key ? "awaiting key" : "click on me to bind";
    std::string id = status + "##" + name;

    auto clicked = ImGui::Button( id.c_str(  ) );
    if( clicked && !waiting_key )
        waiting_key = true;

    std::string popup_id = name + "_popup";

    if( ImGui::BeginPopup( popup_id.c_str(  ), ImGuiWindowFlags_NoMove ) )
    {
        if( ImGui::Selectable( "hold", bind->bind_mode == 0, ImGuiSelectableFlags_DontClosePopups ) )
            bind->bind_mode = 0;

        if( ImGui::Selectable( "toggle", bind->bind_mode == 1, ImGuiSelectableFlags_DontClosePopups ) )
            bind->bind_mode = 1;

        ImGui::EndPopup(  );
    }

    if( !waiting_key && ImGui::IsItemHovered(  ) && ImGui::GetIO(  ).MouseClicked[1] )
        ImGui::OpenPopup( popup_id.c_str(  ) );

    if( waiting_key )
    {
        for ( auto i = 0; i < 5; i++ ) {
            if ( ImGui::GetIO().MouseDown[i] ) {
                switch ( i ) {
                    case 0:
                        key = VK_LBUTTON;
                        break;
                    case 1:
                        key = VK_RBUTTON;
                        break;
                    case 2:
                        key = VK_MBUTTON;
                        break;
                    case 3:
                        key = VK_XBUTTON1;
                        break;
                    case 4:
                        key = VK_XBUTTON2;
                        break;
                    default:
                        key = 0;
                }
                waiting_key = false;
            }
        }

        if( waiting_key )
        {
            for ( auto i = VK_BACK; i <= VK_RMENU; i++ ) {
                if ( ImGui::GetIO(  ).KeysDown[i] ) {
                    key = i;
                    waiting_key = false;
                }
            }
        }
    }

    if ( ImGui::GetIO(  ).KeysDown[VK_ESCAPE] ) {
        bind->key = 0;
    }
    else {
        bind->key = key;
    }
}

static int selected_bind = 0;
void imgui_ex::bind_popup( unsigned int next_type, void* data, const char *config_name ) {
    std::vector<keybind_t*> config_binds;

    for( auto& bnd : binds )
    {
        if (bnd.config_name == config_name)
            config_binds.push_back(&bnd);
    }

    std::string combo_preview = config_binds.empty(  ) ? "empty" : std::string( key_names[config_binds.at( selected_bind )->key] ) + " | " + std::string( bind_types[config_binds.at( selected_bind )->bind_mode] );
    if( ImGui::BeginCombo( "configs", combo_preview.c_str(  ) ) )
    {
        for( int i = 0; i < config_binds.size(  ); i++ )
        {
            auto bnd = config_binds.at( i );

            std::string cfg_text = std::string( key_names[bnd->key] ) + "|" + std::string( bind_types[bnd->bind_mode] ) + "##" + std::to_string( i );
            if( ImGui::Selectable( cfg_text.c_str(  ), selected_bind == i ) )
                selected_bind = i;
        }

        if( ImGui::Selectable( "Add config" ) )
        {
            switch (next_type)
            {
                case FnvHash( "bool" ):
                    waiting_key = false;
                    binds.push_back( keybind_t{ 0, reinterpret_cast<void*>( false ), data, 0, false, FnvHash( "bool" ), config_name } );
                    break;
            }
        }
        ImGui::EndCombo(  );
    }

    if ( config_binds.empty() )
        return;

    auto* bind = config_binds.at( selected_bind );
    if( !bind )
        return;

    key_bind( std::string( config_name ) + std::to_string( selected_bind ), bind );
    switch ( bind->next_type ) {
        case FnvHash( "bool" ):
            ImGui::Checkbox2( "Next Value", reinterpret_cast<bool*>( &bind->data ) );
            break;
    }
}
