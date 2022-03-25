
#include "Config.hpp"
#include "Options.hpp"

#include "Helpers/Math.hpp"
#include "Helpers/Utils.hpp"
#include "Helpers/Input.hpp"
#include "Menu.hpp"

void CConfig::SetupValue(int& value, int def, std::string category, std::string name) { value = def; ints.push_back(new ConfigValue< int >(category, name, &value, def)); }
void CConfig::SetupValue(char* value, char* def, std::string category, std::string name) { value = def; chars.push_back(new ConfigValue< char >(category, name, value, *def)); }
void CConfig::SetupValue(float& value, float def, std::string category, std::string name) { value = def; floats.push_back(new ConfigValue< float >(category, name, &value, def)); }
void CConfig::SetupValue(bool& value, bool def, std::string category, std::string name) { value = def; bools.push_back(new ConfigValue< bool >(category, name, &value, def)); }
void CConfig::SetupColor(float value[4], std::string name)
{
    SetupValue(value[0], value[0], ("colors"), name + "_r");
    SetupValue(value[1], value[1], ("colors"), name + "_g");
    SetupValue(value[2], value[2], ("colors"), name + "_b");
    SetupValue(value[3], value[3], ("colors"), name + "_a");
}

void CConfig::SetupVisuals()
{

}

void CConfig::SetupMisc()
{

}

void CConfig::SetupColors()
{

}

void CConfig::Setup() {

    CConfig::SetupVisuals();
    CConfig::SetupMisc();
    CConfig::SetupColors();
}

void CConfig::Save(const std::string& name) {
    if (name.empty())
        return;

    CreateDirectoryA(u8"C:\\Amnesia\\Configs\\", NULL); //Тут название папки где будут кфг ( \\ нужны обязательно )
    std::string file = u8"C:\\Amnesia\\Configs\\" + name; //Тут название папки где будут кфг ( \\ нужны обязательно )

    for (auto value : ints) {
        WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());
    }

    for (auto value : floats) WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), std::to_string(*value->value).c_str(), file.c_str());
    for (auto value : bools) WritePrivateProfileStringA(value->category.c_str(), value->name.c_str(), *value->value ? "true" : "false", file.c_str());
}

void CConfig::Load(const std::string& name) {
    if (name.empty())
        return;

    g_ClientState->ForceFullUpdate();

    CreateDirectoryA(u8"C:\\Amnesia\\Configs\\", NULL); //Тут название папки где будут кфг ( \\ нужны обязательно )
    std::string file = u8"C:\\Amnesia\\Configs\\" + name; //Тут название папки где будут кфг ( \\ нужны обязательно )

    char value_l[32] = { '\0' };
    for (auto value : ints) {
        GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "0", value_l, 32, file.c_str()); *value->value = atoi(value_l);
    }

    for (auto value : floats) {
        GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "0.0f", value_l, 32, file.c_str()); *value->value = atof(value_l);
    }

    for (auto value : bools) {
        GetPrivateProfileStringA(value->category.c_str(), value->name.c_str(), "false", value_l, 32, file.c_str()); *value->value = !strcmp(value_l, "true");
    }
}

CConfig* Config = new CConfig();