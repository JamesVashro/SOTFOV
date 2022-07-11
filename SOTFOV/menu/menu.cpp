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
            }
        }

        vars->cfgFile.close();

        printf("%s\n", vars->cfgFileDir.c_str());

        vars->fileBrowser.updateDefaultPath(vars->cfgFileDir);
        menu.initialized = true;
    }

    void Menu::Loop() {
        
        if (ImGui::Begin("FOV        Made by - Valasco#6113", &menu.opened, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar)) {
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
                    }

                        
                    vars->playerCharacter->SetTargetFOV(vars->AACharacter, vars->FOV);
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
                }

                vars->cfgFile.close();

                vars->saveDialogOpen = false;
            } else 
                vars->saveDialogOpen = false;


            ImGui::Text("FOV");
            ImGui::SameLine(125);
            if (ImGui::SliderFloat("##FOV", &vars->FOV, 0.f, 180.f, 1)) {
                   
                vars->playerCharacter->SetTargetFOV(vars->AACharacter, vars->FOV);

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

            ImGui::Checkbox("Performance mode", &vars->performance);
            ImGui::Checkbox("Link Values", &vars->linked);
            ImGui::SameLine(240);
            if (ImGui::Checkbox("90 FOV Map", &vars->mapFOV)) {
                if (vars->isOnMap && !vars->mapFOV)
                    vars->AACharacter->SetTargetFOV(vars->AACharacter, vars->FOV);
            }
            if (ImGui::CollapsingHeader("Extras")) {
                ImGui::Text("Sprinting FOV");
                ImGui::SameLine(125);
                ImGui::SliderFloat("##sprintingFOV", &vars->sprintingFOV, 0.f, 180.f, 1);
                ImGui::Text("Sniper FOV");
                ImGui::SameLine(125);
                ImGui::SliderFloat("##sniperFOV", &vars->sniperFOV, 0.f, 180.f, 1);
                ImGui::Text("Pistol FOV");
                ImGui::SameLine(125);
                ImGui::SliderFloat("##pistolFOV", &vars->pistolFOV, 0.f, 180.f, 1);
                ImGui::Text("Blunder FOV");
                ImGui::SameLine(125);
                ImGui::SliderFloat("##blunderFOV", &vars->blunderFOV, 0.f, 180.f, 1);
                ImGui::Text("Spyglass FOV");
                ImGui::SameLine(125);
                ImGui::SliderFloat("##spyGlassFOV", &vars->spyGlassFOV, 0.f, 180.f, 1);
                ImGui::Text("Cannon FOV (ADS)");
                ImGui::SameLine(125);
                ImGui::SliderFloat("##cannonFOVads", &vars->cannonFOVads, 0.f, 180.f, 1);
                ImGui::Text("Cannon FOV");
                ImGui::SameLine(125);
                if (ImGui::SliderFloat("##cannonFOV", &vars->cannonFOV, 0.f, 180.f, 1)) {
                    if (vars->isOnCannon)
                        vars->playerCharacter->SetTargetFOV(vars->AACharacter, vars->cannonFOV);
                }
                /*ImGui::Separator();
                if (ImGui::Button("HotKey") ){
                    vars->SettingHotkey = true;
                    ImGui::Text("<Press any Key>");
                }*/
            }
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

                vars->playerCharacter->SetTargetFOV(vars->AACharacter, vars->FOV);
            }
            
        }
        ImGui::End();
        
    }
}