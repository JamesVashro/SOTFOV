#include <menu/menu.h>

namespace frick {
    void Menu::Init() {
        if (!vars->HasNamesAndObjectsLoaded)
            return;

        char path[MAX_PATH];

        if (GetModuleFileName(vars->hModule, path, sizeof(path)) == 0) {
            printf("Couldnt get filename");
        }

        std::string fPath = path;
        std::string temp = "";
        auto posEnd = fPath.find("SOTFOV.dll");

        temp = fPath.substr(0, posEnd);

        std::replace(temp.begin(), temp.end(), '\\', '/');
        vars->cfgFileDir = temp;

        vars->cfgFile.open(vars->cfgFileDir + "Default.txt", std::ios::in);
        if (vars->cfgFile.is_open()) {
            std::string line;
            while (std::getline(vars->cfgFile, line)) {
                if (line.find("Normal") != std::string::npos) {
                    vars->FOV = std::stof(line);
                    vars->default_FOV = std::stof(line);
                }

                if (line.find("Sniper") != std::string::npos) {
                    vars->sniperFOV = std::stof(line);
                    vars->default_sniperFOV = std::stof(line);
                }

                if (line.find("Pistol") != std::string::npos) {
                    vars->pistolFOV = std::stof(line);
                    vars->default_pistolFOV = std::stof(line);
                }

                if (line.find("Sprinting") != std::string::npos) {
                    vars->sprintingFOV = std::stof(line);
                    vars->default_sprintingFOV = std::stof(line);
                }

                if (line.find("Blunder") != std::string::npos) {
                    vars->blunderFOV = std::stof(line);
                    vars->default_blunderFOV = std::stof(line);
                }

                if (line.find("Spyglass") != std::string::npos) {
                    vars->spyGlassFOV = std::stof(line);
                    vars->default_spyGlassFOV = std::stof(line);
                }

                if (line.find("CannonADS") != std::string::npos) {
                    vars->cannonFOVads = std::stof(line);
                    vars->default_cannonFOVads = std::stof(line);
                }

                if (line.find("Cannon") != std::string::npos) {
                    vars->cannonFOV = std::stof(line);
                    vars->default_cannonFOV = std::stof(line);
                }

                if (line.find("MapTable") != std::string::npos) {
                    int b = std::stoi(line);
                    if (b == 0) {
                        vars->mapFOV = false;
                        vars->default_MapFOV = false;
                    } else {
                        vars->mapFOV = true;
                        vars->default_MapFOV = true;

                    }
                }

                if (line.find("MenuHotkey") != -1) {
                    vars->HotKey = std::stoi(line);
                }
            }
        }

        vars->cfgFile.close();

        printf("%s\n", vars->cfgFileDir.c_str());

        vars->fileBrowser.updateDefaultPath(vars->cfgFileDir);
        menu.initialized = true;
    }

    void Menu::Loop() {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 10);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5);

        if (ImGui::Begin("            Made by - Valasco#6113", &menu.opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar)) {
            if (ImGui::BeginMenuBar()) {
                if (!vars->simpleMode) {
                    if (ImGui::BeginMenu("File")) {
                        if (ImGui::MenuItem("Save config", NULL))
                            vars->saveDialogOpen = true;

                        if (ImGui::MenuItem("Load Config", NULL))
                            vars->openDialogOpen = true;

                        ImGui::EndMenu();
                    }
                }
                
                ImGui::EndMenuBar();
            }

            if (vars->saveDialogOpen)
                ImGui::OpenPopup("Save File");

            if (vars->openDialogOpen)
                ImGui::OpenPopup("Open File");
                
            if (vars->fileBrowser.showFileDialog("Open File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(300, 300), ".txt")) {
                vars->cfgFile.open(vars->fileBrowser.selected_path, std::ios::in);
                if (vars->cfgFile.is_open()) {
                    std::string line;
                    while (std::getline(vars->cfgFile, line)) {
                        if (line.find("Normal") != std::string::npos) {
                            vars->FOV = std::stof(line);
                            vars->default_FOV = std::stof(line);
                        }

                        if (line.find("Sniper") != std::string::npos) {
                            vars->sniperFOV = std::stof(line);
                            vars->default_sniperFOV = std::stof(line);
                        }

                        if (line.find("Pistol") != std::string::npos) {
                            vars->pistolFOV = std::stof(line);
                            vars->default_pistolFOV = std::stof(line);
                        }

                        if (line.find("Sprinting") != std::string::npos) {
                            vars->sprintingFOV = std::stof(line);
                            vars->default_sprintingFOV = std::stof(line);
                        }

                        if (line.find("Blunder") != std::string::npos) {
                            vars->blunderFOV = std::stof(line);
                            vars->default_blunderFOV = std::stof(line);
                        }

                        if (line.find("Spyglass") != std::string::npos) {
                            vars->spyGlassFOV = std::stof(line);
                            vars->default_spyGlassFOV = std::stof(line);
                        }

                        if (line.find("CannonADS") != std::string::npos) {
                            vars->cannonFOVads = std::stof(line);
                            vars->default_cannonFOVads = std::stof(line);
                        }

                        if (line.find("Cannon") != std::string::npos) {
                            vars->cannonFOV = std::stof(line);
                            vars->default_cannonFOV = std::stof(line);
                        }

                        if (line.find("MapTable") != std::string::npos) {
                            int b = std::stoi(line);
                            if (b == 0) {
                                vars->mapFOV = false;
                                vars->default_MapFOV = false;
                            }
                            else {
                                vars->mapFOV = true;
                                vars->default_MapFOV = true;

                            }
                        }

                        if (line.find("MenuHotkey") != -1) {
                            vars->HotKey = std::stoi(line);
                        }
                    }

                        
                    vars->AACharacter->SetTargetFOV(vars->AACharacter, vars->FOV);
                }

                vars->cfgFile.close();
                vars->openDialogOpen = false;
            }
            else {
                vars->openDialogOpen = false;

            }

            if (vars->fileBrowser.showFileDialog("Save File", imgui_addons::ImGuiFileBrowser::DialogMode::SAVE, ImVec2(300, 300), ".txt")) {
                if (vars->fileBrowser.selected_fn.find(".txt") == std::string::npos) {
                    vars->cfgFile.open(vars->fileBrowser.selected_path.append(".txt"), std::ios::out);
                } else 
                        vars->cfgFile.open(vars->fileBrowser.selected_path, std::ios::out);

                if (vars->cfgFile.is_open()){
                    vars->cfgFile << std::to_string(vars->FOV) + " Normal\n";
                    vars->cfgFile << std::to_string(vars->sniperFOV) + " Sniper\n";
                    vars->cfgFile << std::to_string(vars->pistolFOV) + " Pistol\n";
                    vars->cfgFile << std::to_string(vars->sprintingFOV) + " Sprinting\n";
                    vars->cfgFile << std::to_string(vars->blunderFOV) + " Blunder\n";
                    vars->cfgFile << std::to_string(vars->spyGlassFOV) + " Spyglass\n";
                    vars->cfgFile << std::to_string(vars->cannonFOVads) + " CannonADS\n";
                    vars->cfgFile << std::to_string(vars->cannonFOV) + " Cannon\n";
                    if (vars->mapFOV)
                        vars->cfgFile << std::to_string(1) + " MapTable\n";
                    else
                        vars->cfgFile << std::to_string(0) + " MapTable\n";

                    vars->cfgFile << std::to_string(vars->HotKey) + " MenuHotkey\n";
                }

                vars->cfgFile.close();

                vars->saveDialogOpen = false;
            } else 
                vars->saveDialogOpen = false;


            ImGui::Text("FOV");
            ImGui::SameLine(125);
            if (ImGui::LineSliderFloat("##FOV", &vars->FOV, 0.f, 180.f, 1)) {
                if (vars->noZoom)
                {
                    vars->localPlayer->PlayerController->FOV(vars->FOV);
                }
                else
                {
                    if ((int)vars->FOV - 13 == 60 || (int)vars->FOV - 13 == 30 || (int)vars->FOV - 13 == 17 || (int)vars->FOV - 13 == 70 || (int)vars->FOV - 13 == 90)
                        vars->FOV++;

                    vars->AACharacter->SetTargetFOV(vars->AACharacter, vars->FOV);

                    if (vars->linked) {
                        vars->sniperFOV = vars->FOV - 55.f;
                        vars->pistolFOV = vars->FOV - 23.f;
                        vars->blunderFOV = vars->FOV;
                        vars->spyGlassFOV = vars->FOV - 77.f;
                        vars->sprintingFOV = vars->FOV + 10.f;
                        vars->cannonFOVads = vars->FOV - 23.f;
                        vars->cannonFOV = vars->FOV;
                    }
                }
            }

            ImGui::CheckboxSlider("Performance mode", &vars->performance);
            ImGui::CheckboxSlider("Link Values", &vars->linked);
            ImGui::SameLine(225);
            if (ImGui::CheckboxSlider("90 FOV Map", &vars->mapFOV)) {
                if (vars->isOnMap && !vars->mapFOV)
                    vars->AACharacter->SetTargetFOV(vars->AACharacter, vars->FOV);
            }

            if (ImGui::CheckboxSlider("No-Zoom", &vars->noZoom))
            {
                vars->localPlayer->PlayerController->FOV(vars->FOV);
            }

            if (ImGui::CollapsingHeader("Extras")) {
                ImGui::Text("Sprinting FOV");
                ImGui::SameLine(125);
                if (ImGui::LineSliderFloat("##sprintingFOV", &vars->sprintingFOV, 0.f, 180.f, 1)) {
                    if ((int)vars->sprintingFOV - 13 == 60 || (int)vars->sprintingFOV - 13 == 30 || (int)vars->sprintingFOV - 13 == 17 || (int)vars->sprintingFOV - 13 == 70 || (int)vars->sprintingFOV - 13 == 73 || (int)vars->sprintingFOV - 13 == 78 || (int)vars->sprintingFOV - 13 == 90)
                        vars->sprintingFOV++;
                }

                ImGui::Text("Sniper FOV");
                ImGui::SameLine(125);
                if (ImGui::LineSliderFloat("##sniperFOV", &vars->sniperFOV, 0.f, 180.f, 1)) {
                    if ((int)vars->sniperFOV - 13 == 60 || (int)vars->sniperFOV - 13 == 30 || (int)vars->sniperFOV - 13 == 17 || (int)vars->sniperFOV - 13 == 70 || (int)vars->sniperFOV - 13 == 73 || (int)vars->sniperFOV - 13 == 78 || (int)vars->sniperFOV - 13 == 90)
                        vars->sniperFOV++;
                }
                ImGui::Text("Pistol FOV");
                ImGui::SameLine(125);
                if (ImGui::LineSliderFloat("##pistolFOV", &vars->pistolFOV, 0.f, 180.f, 1)) {
                    if ((int)vars->pistolFOV - 13 == 60 || (int)vars->pistolFOV - 13 == 30 || (int)vars->pistolFOV - 13 == 17 || (int)vars->pistolFOV - 13 == 70 || (int)vars->pistolFOV - 13 == 73 || (int)vars->pistolFOV - 13 == 78 || (int)vars->pistolFOV - 13 == 90)
                        vars->pistolFOV++;
                }
                ImGui::Text("Blunder FOV");
                ImGui::SameLine(125);
                if (ImGui::LineSliderFloat("##blunderFOV", &vars->blunderFOV, 0.f, 180.f, 1)) {
                    if ((int)vars->blunderFOV - 13 == 60 || (int)vars->blunderFOV - 13 == 30 || (int)vars->blunderFOV - 13 == 17 || (int)vars->blunderFOV - 13 == 70 || (int)vars->blunderFOV - 13 == 73 || (int)vars->blunderFOV - 13 == 78 || (int)vars->blunderFOV - 13 == 90)
                        vars->blunderFOV++;
                }
                ImGui::Text("Spyglass FOV");
                ImGui::SameLine(125);
                if (ImGui::LineSliderFloat("##spyGlassFOV", &vars->spyGlassFOV, 0.f, 180.f, 1)) {
                    if ((int)vars->spyGlassFOV - 13 == 60 || (int)vars->spyGlassFOV - 13 == 30 || (int)vars->spyGlassFOV - 13 == 17 || (int)vars->spyGlassFOV - 13 == 70 || (int)vars->spyGlassFOV - 13 == 73 || (int)vars->spyGlassFOV - 13 == 78 || (int)vars->spyGlassFOV - 13 == 90)
                        vars->spyGlassFOV++;
                }
                ImGui::Text("Cannon FOV (ADS)");
                ImGui::SameLine(125);
                if (ImGui::LineSliderFloat("##cannonFOVads", &vars->cannonFOVads, 0.f, 180.f, 1)) {
                    if ((int)vars->cannonFOVads - 13 == 60 || (int)vars->cannonFOVads - 13 == 30 || (int)vars->cannonFOVads - 13 == 17 || (int)vars->cannonFOVads - 13 == 70 || (int)vars->cannonFOVads - 13 == 73 || (int)vars->cannonFOVads - 13 == 78 || (int)vars->cannonFOVads - 13 == 90)
                        vars->cannonFOVads++;
                }
                ImGui::Text("Cannon FOV");
                ImGui::SameLine(125);
                if (ImGui::LineSliderFloat("##cannonFOV", &vars->cannonFOV, 0.f, 180.f, 1)) {
                    if ((int)vars->cannonFOV - 13 == 60 || (int)vars->cannonFOV - 13 == 30 || (int)vars->cannonFOV - 13 == 17 || (int)vars->cannonFOV - 13 == 70 || (int)vars->cannonFOV - 13 == 73 || (int)vars->cannonFOV - 13 == 78 || (int)vars->cannonFOV - 13 == 90)
                        vars->cannonFOV++;
                    if (vars->isOnCannon)
                        vars->AACharacter->SetTargetFOV(vars->AACharacter, vars->cannonFOV);
                }
            }
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 255));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
            if (ImGui::Button("Reset")) {
                vars->FOV = vars->default_FOV;
                vars->sprintingFOV = vars->default_sprintingFOV;
                vars->sniperFOV = vars->default_sniperFOV;
                vars->pistolFOV = vars->default_pistolFOV;
                vars->spyGlassFOV = vars->default_spyGlassFOV;
                vars->blunderFOV = vars->default_blunderFOV;
                vars->cannonFOVads = vars->default_cannonFOVads;
                vars->cannonFOV = vars->default_cannonFOV;
                vars->linked = false;
                vars->mapFOV = vars->default_MapFOV;

                vars->AACharacter->SetTargetFOV(vars->AACharacter, vars->FOV);
            }

            ImGui::SameLine();
            if (ImGui::Button(vars->SettingHotkey ? "<Press Any Key>" : "Hot Key"))
                vars->SettingHotkey = true;
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            
        }
        ImGui::End();

        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
        ImGui::PopStyleVar();
        
    }
}