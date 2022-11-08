#include <menu/menu.h>

namespace frick {
    void Menu::Init() {
        //std::cout << "Menu Init" << std::endl;
        char path[MAX_PATH];

        if (GetModuleFileName(vars->hModule, path, sizeof(path)) == 0) {
            printf("Couldnt get filename");
        }

        std::string fPath = path;
        std::string temp = "";
        auto posEnd = fPath.find("SOTFOV.dll");

        menu.initialized = true;
    }

    void Menu::Loop() {
        
        if (ImGui::Begin("FOV        Made by - Valasco#6113", &menu.opened, ImGuiWindowFlags_AlwaysAutoResize)) {
            
            ImGui::Text("FOV");
            ImGui::SameLine(75);
            if (ImGui::SliderFloat("##FOV", &vars->FOV, 0.f, 180.f, 1)) {
                if ((int)vars->FOV - 13 == 60 || (int)vars->FOV - 13 == 30 || (int)vars->FOV - 13 == 17 || (int)vars->FOV - 13 == 70 || (int)vars->FOV - 13 == 90)
                    vars->FOV++;

                vars->playerCharacter->SetTargetFOV(vars->AACharacter, vars->FOV);
            }

            if (ImGui::Button(vars->SettingHotkey ? "<Press Any Key>" : "Hot Key"))
                vars->SettingHotkey = true;

            ImGui::SameLine(75);
            ImGui::Checkbox("Sprinting FOV", &vars->sprintFOV);
            
        }
        ImGui::End();
        
    }
}