#pragma once
#include "imgui.h"
#include "../core/Depozit.h"
#include "../core/AlertaManager.h"
#include "../core/Raport.h"
#include "../models/User.h"
#include "../data/Database.h"
#include <string>
#include <vector>

class MainUI {
private:
    Database& db;
    Depozit& depozit;
    AlertaManager alertaManager;
    User userCurent;

    int produsSelectat = -1;
    int zonaCliclata = -1;
    char cautare[128] = "";

    char numeProdus[128] = "";
    int cantitate = 0;
    float pret = 0.0f;
    int pragAlerta = 10;
    int zonaSelectata = 1;
    int cantitateModificare = 1;
    std::string mesajStatus = "";

    void renderZoneGrid() {
        auto& zone = depozit.getZone();
        ImGui::Text("HARTA DEPOZIT");
        ImGui::Separator();
        ImGui::Spacing();

        float btnSize = 80.0f;
        float spacing = 8.0f;
        int col = 0;

        std::vector<Zona*> zoneVec;
        for (auto& [id, z] : zone)
            zoneVec.push_back(&z);
        std::sort(zoneVec.begin(), zoneVec.end(),
            [](Zona* a, Zona* b) { return a->getLitera() < b->getLitera(); });

        for (auto* z : zoneVec) {
            if (col > 0 && col % 4 != 0)
                ImGui::SameLine(0, spacing);

            ImVec4 culoare = z->getCuloare();
            bool selectata = (zonaCliclata == z->getId());

            if (selectata)
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);

            ImGui::PushStyleColor(ImGuiCol_Button, culoare);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                ImVec4(culoare.x+0.15f, culoare.y+0.15f, culoare.z+0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, culoare);

            std::string label = std::string("Zona ") + z->getLitera() +
                "\n" + std::to_string((int)z->getProcentOcupare()) + "%" +
                "##z" + z->getLitera();

            if (ImGui::Button(label.c_str(), ImVec2(btnSize, btnSize)))
                zonaCliclata = z->getId();

            ImGui::PopStyleColor(3);
            if (selectata)
                ImGui::PopStyleVar();

            col++;
        }

        // Produse din zona selectata
        if (zonaCliclata >= 0) {
            Zona* z = depozit.getZona(zonaCliclata);
            if (z) {
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.2f,0.7f,1.0f,1.0f),
                    "Zona %c — %d/%d (%.0f%%)",
                    z->getLitera(),
                    z->getCapacitateCurenta(),
                    z->getCapacitateMax(),
                    z->getProcentOcupare());
                ImGui::Spacing();

                bool areProduse = false;
                for (auto& [id, p] : depozit.getProduse()) {
                    if (p.getZonaId() == zonaCliclata) {
                        ImGui::BulletText("%s — stoc: %d buc | %.2f RON",
                            p.getNume().c_str(), p.getCantitate(), p.getPret());
                        areProduse = true;
                    }
                }
                if (!areProduse)
                    ImGui::TextDisabled("Niciun produs in aceasta zona.");
            }
        }

        // Legenda
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Legenda:");
        ImGui::SameLine(); ImGui::TextColored(ImVec4(0.4f,0.4f,0.4f,1), "[Goala]");
        ImGui::SameLine(); ImGui::TextColored(ImVec4(0.2f,0.8f,0.2f,1), "[1-25%%]");
        ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f,0.9f,0.1f,1), "[26-50%%]");
        ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f,0.6f,0.1f,1), "[51-75%%]");
        ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f,0.2f,0.2f,1), "[76-100%%]");
    }

    void renderTabelProduse() {
        ImGui::Text("PRODUSE");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::SetNextItemWidth(200);
        ImGui::InputText("Cauta##search", cautare, sizeof(cautare));
        ImGui::Spacing();

        if (ImGui::BeginTable("produse", 6,
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_ScrollY,
            ImVec2(0, 250)))
        {
            ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 40);
            ImGui::TableSetupColumn("Nume", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Cantitate", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableSetupColumn("Pret", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableSetupColumn("Prag", ImGuiTableColumnFlags_WidthFixed, 60);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableHeadersRow();

            std::string filtru(cautare);
            for (auto& [id, p] : depozit.getProduse()) {
                if (!filtru.empty() &&
                    p.getNume().find(filtru) == std::string::npos)
                    continue;

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", p.getId());

                ImGui::TableSetColumnIndex(1);
                bool selectat = (produsSelectat == p.getId());
                if (ImGui::Selectable(p.getNume().c_str(), selectat,
                    ImGuiSelectableFlags_SpanAllColumns))
                    produsSelectat = p.getId();

                ImGui::TableSetColumnIndex(2);
                if (p.eSubPrag())
                    ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "%d", p.getCantitate());
                else
                    ImGui::Text("%d", p.getCantitate());

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.2f", p.getPret());

                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%d", p.getPragAlerta());

                ImGui::TableSetColumnIndex(5);
                if (p.getCantitate() == 0)
                    ImGui::TextColored(ImVec4(1,0,0,1), "EPUIZAT");
                else if (p.eSubPrag())
                    ImGui::TextColored(ImVec4(1,0.6f,0,1), "SCAZUT");
                else
                    ImGui::TextColored(ImVec4(0,1,0,1), "OK");
            }
            ImGui::EndTable();
        }
    }

    void renderFormularAdauga() {
        if (ImGui::CollapsingHeader("Adauga Produs Nou")) {
            ImGui::InputText("Nume##add", numeProdus, sizeof(numeProdus));
            ImGui::InputInt("Cantitate##add", &cantitate);
            ImGui::InputFloat("Pret##add", &pret, 0.1f, 1.0f, "%.2f");
            ImGui::InputInt("Prag Alerta##add", &pragAlerta);
            ImGui::InputInt("Zona ID##add", &zonaSelectata);
            ImGui::Spacing();

            if (ImGui::Button("Adauga##btn", ImVec2(120, 30))) {
                if (strlen(numeProdus) > 0 && cantitate >= 0 && pret >= 0) {
                    try {
                        Produs p(0, numeProdus, cantitate, pret, pragAlerta, zonaSelectata);
                        depozit.adaugaProdus(p);
                        mesajStatus = "Produs adaugat cu succes!";
                        memset(numeProdus, 0, sizeof(numeProdus));
                        cantitate = 0; pret = 0; pragAlerta = 10;
                        alertaManager.refresh(getProduseVec());
                    } catch (const std::exception& e) {
                        mesajStatus = std::string("Eroare: ") + e.what();
                    }
                }
            }
        }
    }

    void renderGestiuneStoc() {
        if (produsSelectat < 0) return;
        Produs* p = depozit.getProdus(produsSelectat);
        if (!p) return;

        if (ImGui::CollapsingHeader("Gestioneaza Stoc", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Produs selectat: %s (stoc: %d)",
                p->getNume().c_str(), p->getCantitate());
            ImGui::InputInt("Cantitate##stoc", &cantitateModificare);
            if (cantitateModificare < 1) cantitateModificare = 1;

            ImGui::Spacing();
            if (ImGui::Button("+ Adauga Stoc", ImVec2(130, 30))) {
                try {
                    depozit.adaugaStoc(produsSelectat, cantitateModificare);
                    mesajStatus = "Stoc actualizat!";
                    alertaManager.refresh(getProduseVec());
                } catch (const std::exception& e) {
                    mesajStatus = e.what();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("- Scade Stoc", ImVec2(130, 30))) {
                try {
                    depozit.scadeStoc(produsSelectat, cantitateModificare);
                    mesajStatus = "Stoc actualizat!";
                    alertaManager.refresh(getProduseVec());
                } catch (const std::exception& e) {
                    mesajStatus = e.what();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Sterge Produs", ImVec2(130, 30))) {
                try {
                    depozit.eliminaProdus(produsSelectat);
                    produsSelectat = -1;
                    mesajStatus = "Produs sters!";
                    alertaManager.refresh(getProduseVec());
                } catch (const std::exception& e) {
                    mesajStatus = e.what();
                }
            }
        }
    }

    void renderAlerte() {
        if (!alertaManager.areAlerte()) return;
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1,0.3f,0.3f,1),
            "ALERTE ACTIVE: %d", alertaManager.getNrAlerte());
        ImGui::Separator();
        for (const auto& a : alertaManager.getAlerte()) {
            ImGui::TextColored(a.getCuloare(), "%s %s",
                a.getTipString().c_str(), a.mesaj.c_str());
        }
    }

    void renderRapoarte() {
        if (ImGui::CollapsingHeader("Rapoarte")) {
            ImGui::Text("Valoare totala stoc: %.2f RON",
                Raport::getValoareTotala(depozit.getProduse()));
            ImGui::Text("Produse sub prag: %d",
                Raport::getNrSubPrag(depozit.getProduse()));
            ImGui::Text("Total produse: %d", depozit.getNrProduse());
            ImGui::Spacing();
            ImGui::Text("Top produse critice:");
            auto top = Raport::getTopCritice(depozit.getProduse());
            for (const auto& r : top) {
                ImGui::TextColored(
                    r.subPrag ? ImVec4(1,0.3f,0.3f,1) : ImVec4(1,1,1,1),
                    "  %s: %d buc (%.2f RON)",
                    r.nume.c_str(), r.cantitate, r.valoare);
            }
        }
    }

    std::vector<Produs> getProduseVec() {
        std::vector<Produs> v;
        for (auto& [id, p] : depozit.getProduse())
            v.push_back(p);
        return v;
    }

public:
    MainUI(Database& db, Depozit& depozit, User user)
        : db(db), depozit(depozit), userCurent(user) {
        alertaManager.refresh(getProduseVec());
    }

    void render() {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);

        ImGui::Begin("##Main", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus);

        ImGui::TextColored(ImVec4(0.2f,0.7f,1.0f,1.0f),
            "SISTEM MONITORIZARE DEPOZIT");
        ImGui::SameLine(ImGui::GetWindowWidth() - 200);
        ImGui::Text("User: %s (%s)",
            userCurent.getUsername().c_str(),
            userCurent.getRolString().c_str());
        ImGui::Separator();
        ImGui::Spacing();

        float leftWidth = io.DisplaySize.x * 0.42f;

        ImGui::BeginChild("##zone", ImVec2(leftWidth, -1), true);
        renderZoneGrid();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("##produse", ImVec2(-1, -1), true);
        renderTabelProduse();
        ImGui::Spacing();
        renderFormularAdauga();
        ImGui::Spacing();
        renderGestiuneStoc();
        ImGui::Spacing();
        renderAlerte();
        ImGui::Spacing();
        renderRapoarte();

        if (!mesajStatus.empty()) {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(0.2f,1.0f,0.2f,1.0f),
                "%s", mesajStatus.c_str());
        }

        ImGui::EndChild();
        ImGui::End();
    }
};