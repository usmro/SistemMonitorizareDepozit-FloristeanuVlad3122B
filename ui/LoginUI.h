#pragma once
#include "imgui.h"
#include "../data/Database.h"
#include "../models/User.h"
#include <string>
#include <cstring>

class LoginUI {
private:
    char username[64] = "";
    char parola[64] = "";
    std::string mesajEroare = "";
    bool autentificat = false;
    User userCurent;
    Database& db;

public:
    LoginUI(Database& db) : db(db) {}

    bool eAutentificat() const { return autentificat; }
    User getUserCurent() const { return userCurent; }

    void reset() {
        memset(username, 0, sizeof(username));
        memset(parola, 0, sizeof(parola));
        mesajEroare = "";
        autentificat = false;
        userCurent = User();
    }

    void render() {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(420, 270), ImGuiCond_Always);

        ImGui::Begin("##Login", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove);

        float windowWidth = ImGui::GetWindowWidth();
        std::string titlu = "SISTEM MONITORIZARE DEPOZIT";
        ImGui::SetCursorPosX((windowWidth - ImGui::CalcTextSize(titlu.c_str()).x) * 0.5f);
        ImGui::TextColored(ImVec4(0.2f, 0.7f, 1.0f, 1.0f), "%s", titlu.c_str());
        ImGui::Separator();
        ImGui::Spacing();

        std::string sub = "Autentificare";
        ImGui::SetCursorPosX((windowWidth - ImGui::CalcTextSize(sub.c_str()).x) * 0.5f);
        ImGui::Text("%s", sub.c_str());
        ImGui::Spacing();

        ImGui::Text("Utilizator:");
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##user", username, sizeof(username));

        ImGui::Text("Parola:");
        ImGui::SetNextItemWidth(-1);
        bool enterApasat = ImGui::InputText("##pass", parola, sizeof(parola),
            ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue);

        ImGui::Spacing();

        ImGui::SetCursorPosX((windowWidth - 120) * 0.5f);
        if (ImGui::Button("Conectare", ImVec2(120, 35)) || enterApasat) {
            if (db.autentifica(username, parola, userCurent)) {
                autentificat = true;
                mesajEroare = "";
            } else {
                mesajEroare = "Username sau parola incorecte!";
                memset(parola, 0, sizeof(parola));
            }
        }

        if (!mesajEroare.empty()) {
            ImGui::Spacing();
            ImGui::SetCursorPosX((windowWidth - ImGui::CalcTextSize(mesajEroare.c_str()).x) * 0.5f);
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", mesajEroare.c_str());
        }

        ImGui::End();
    }
};