#pragma once
#include "imgui.h"
#include "../core/Depozit.h"
#include "../core/AlertaManager.h"
#include "../core/Raport.h"
#include "../models/User.h"
#include "../data/Database.h"
#include <string>
#include <vector>
#include <algorithm>

class MainUI {
private:
    Database& db;
    Depozit& depozit;
    AlertaManager alertaManager;
    User userCurent;

    int produsSelectat = -1;
    int zonaCliclata = -1;
    char cautare[128] = "";
    char cautareZona[128] = "";
    char cautareCategorie[128] = "";
    char cautareFurnizor[128] = "";
    bool shouldLogout = false;

    char numeProdus[128] = "";
    int cantitate = 0;
    float pret = 0.0f;
    int pragAlerta = 10;
    int zonaSelectata = 1;
    int cantitateModificare = 1;
    char observatiiStoc[256] = "";
    char observatiiAdauga[256] = "";
    std::string mesajStatus = "";

    char editNume[128] = "";
    float editPret = 0.0f;
    int editPrag = 0;
    int editZona = 0;
    bool editMode = false;

    char newUsername[64] = "";
    char newParola[64] = "";
    bool showParola = false;
    int newRol = 1;
    std::string mesajAdmin = "";

    int paginaCurenta = 0;
    int produsePerPagina = 50;

    bool showPopupZonaPlina = false;
    int zonaAlternativaId = -1;
    int produsDeMusat = -1;
    std::string mesajZonaPlina = "";

    bool showPopupZonaPlinaAdauga = false;
    int zonaAlternativaAdaugaId = -1;
    std::string numeProdusPending = "";
    int cantPending = 0;
    float pretPending = 0.0f;
    int pragPending = 0;
    std::string obsPending = "";

    int sortColProduse = 0;
    bool sortAscProduse = true;
    int sortColTranzactii = 0;
    bool sortAscTranzactii = false;
    int sortColUsers = 0;
    bool sortAscUsers = true;
    int sortColCategorii = 0;
    bool sortAscCategorii = true;
    int sortColFurnizori = 0;
    bool sortAscFurnizori = true;

    static std::string toLower(const std::string& s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    void sorteazaProduse(std::vector<Produs>& v) {
        std::sort(v.begin(), v.end(), [&](const Produs& a, const Produs& b) {
            bool cmp = false;
            switch (sortColProduse) {
                case 0: cmp = a.getId() < b.getId(); break;
                case 1: cmp = toLower(a.getNume()) < toLower(b.getNume()); break;
                case 2: cmp = a.getCantitate() < b.getCantitate(); break;
                case 3: cmp = a.getPret() < b.getPret(); break;
                case 4: cmp = a.getZonaId() < b.getZonaId(); break;
                case 5: cmp = a.getCantitate() < b.getCantitate(); break;
                default: cmp = a.getId() < b.getId(); break;
            }
            return sortAscProduse ? cmp : !cmp;
        });
    }

    void renderZoneGrid() {
        auto& zone = depozit.getZone();
        ImGui::Text("HARTA DEPOZIT");
        ImGui::Separator();
        ImGui::Spacing();

        float spacing = 8.0f;
        float availableWidth = ImGui::GetContentRegionAvail().x;
        float availableHeight = ImGui::GetContentRegionAvail().y * 0.55f;
        float btnWidth = (availableWidth - spacing * 3) / 4.0f;
        float btnHeight = (availableHeight - spacing * 3) / 4.0f;
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
            ImVec4 culoareText = z->getCuloareText();
            bool selectata = (zonaCliclata == z->getId());

            if (selectata)
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);

            ImGui::PushStyleColor(ImGuiCol_Button, culoare);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                ImVec4(culoare.x+0.15f, culoare.y+0.15f, culoare.z+0.15f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, culoare);
            ImGui::PushStyleColor(ImGuiCol_Text, culoareText);

            std::string label = std::string("Zona ") + z->getLitera() +
                "\n" + std::to_string((int)z->getProcentOcupare()) + "%" +
                "##z" + z->getLitera();

            if (ImGui::Button(label.c_str(), ImVec2(btnWidth, btnHeight)))
                zonaCliclata = z->getId();

            ImGui::PopStyleColor(4);
            if (selectata)
                ImGui::PopStyleVar();
            col++;
        }

        // Detalii zona selectata
        if (zonaCliclata >= 0) {
            Zona* z = depozit.getZona(zonaCliclata);
            if (z) {
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::TextColored(ImVec4(0.2f,0.7f,1.0f,1.0f),
                    "Zona %c: %d/%d (%.0f%%)",
                    z->getLitera(),
                    z->getCapacitateCurenta(),
                    z->getCapacitateMax(),
                    z->getProcentOcupare());
                ImGui::Spacing();

                ImGui::Text("Cauta in zona:");
                ImGui::SetNextItemWidth(-1);
                ImGui::InputText("##cz", cautareZona, sizeof(cautareZona));
                ImGui::Spacing();

                // Child scrollabil pentru produse din zona
                float altimeDisponibila = ImGui::GetContentRegionAvail().y - 65.0f;
                ImGui::BeginChild("##produse_zona",
            ImVec2(-1, altimeDisponibila > 80 ? altimeDisponibila : 80),
            true,
                ImGuiWindowFlags_HorizontalScrollbar);

                bool areProduse = false;
                std::string filtruZona(cautareZona);
                for (auto& [id, p] : depozit.getProduse()) {
                    if (p.getZonaId() == zonaCliclata) {
                        if (!filtruZona.empty() &&
                            toLower(p.getNume()).find(toLower(filtruZona)) == std::string::npos)
                            continue;
                        ImGui::BulletText("%s - stoc: %d | %.2f RON",
                            p.getNume().c_str(), p.getCantitate(), p.getPret());
                        areProduse = true;
                    }
                }
                if (!areProduse)
                    ImGui::TextDisabled("Niciun produs in aceasta zona.");

                ImGui::EndChild();
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

    void renderTabDashboard() {
        float leftWidth = ImGui::GetContentRegionAvail().x * 0.42f;

        ImGui::BeginChild("##zone", ImVec2(leftWidth, -1), true);
        renderZoneGrid();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("##produse", ImVec2(-1, -1), true);
        renderTabelProduse();
        ImGui::Spacing();
        if (userCurent.eAdmin())
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
    }

    void renderTabelProduse() {
        ImGui::Text("PRODUSE");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::SetNextItemWidth(200);
        if (ImGui::InputText("Cauta##search", cautare, sizeof(cautare)))
            paginaCurenta = 0;
        ImGui::Spacing();

        std::string filtru(cautare);
        std::vector<Produs> produseVec;
        for (auto& [id, p] : depozit.getProduse()) {
            if (!filtru.empty() &&
                toLower(p.getNume()).find(toLower(filtru)) == std::string::npos)
                continue;
            produseVec.push_back(p);
        }
        sorteazaProduse(produseVec);

        int totalProduseFiltrate = (int)produseVec.size();
        int totalPagini = std::max(1, (totalProduseFiltrate + produsePerPagina - 1) / produsePerPagina);

        if (paginaCurenta >= totalPagini) paginaCurenta = totalPagini - 1;
        if (paginaCurenta < 0) paginaCurenta = 0;

        int startIdx = paginaCurenta * produsePerPagina;
        int endIdx = std::min(startIdx + produsePerPagina, totalProduseFiltrate);
        std::vector<Produs> produsePagina(
            produseVec.begin() + startIdx,
            produseVec.begin() + endIdx);

        if (ImGui::BeginTable("produse", 6,
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_ScrollY |
            ImGuiTableFlags_ScrollX |
            ImGuiTableFlags_Sortable,
            ImVec2(0, 200)))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("ID",
                ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 40);
            ImGui::TableSetupColumn("Nume", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Cantitate", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableSetupColumn("Pret", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableSetupColumn("Zona", ImGuiTableColumnFlags_WidthFixed, 70);
            ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableHeadersRow();

            if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
                if (sortSpecs->SpecsDirty && sortSpecs->SpecsCount > 0) {
                    sortColProduse = sortSpecs->Specs[0].ColumnIndex;
                    sortAscProduse = sortSpecs->Specs[0].SortDirection == ImGuiSortDirection_Ascending;
                    sortSpecs->SpecsDirty = false;
                    paginaCurenta = 0;
                }
            }

            for (auto& p : produsePagina) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", p.getId());

                ImGui::TableSetColumnIndex(1);
                bool selectat = (produsSelectat == p.getId());
                if (ImGui::Selectable(p.getNume().c_str(), selectat,
                    ImGuiSelectableFlags_SpanAllColumns)) {
                    produsSelectat = p.getId();
                    strncpy(editNume, p.getNume().c_str(), sizeof(editNume));
                    editPret = (float)p.getPret();
                    editPrag = p.getPragAlerta();
                    editZona = p.getZonaId();
                    editMode = false;
                }

                ImGui::TableSetColumnIndex(2);
                if (p.eSubPrag())
                    ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "%d", p.getCantitate());
                else
                    ImGui::Text("%d", p.getCantitate());

                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.2f", p.getPret());

                ImGui::TableSetColumnIndex(4);
                Zona* z = depozit.getZona(p.getZonaId());
                if (z)
                    ImGui::Text("Zona %c", z->getLitera());
                else
                    ImGui::TextDisabled("N/A");

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

        // Paginare
        ImGui::Spacing();
        if (paginaCurenta <= 0) ImGui::BeginDisabled();
        if (ImGui::Button("< Inapoi##pag", ImVec2(80, 25))) paginaCurenta--;
        if (paginaCurenta <= 0) ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.2f,0.7f,1.0f,1.0f),
            "Pagina %d / %d  (%d-%d din %d)",
            paginaCurenta + 1, totalPagini,
            startIdx + 1, endIdx, totalProduseFiltrate);
        ImGui::SameLine();

        if (paginaCurenta >= totalPagini - 1) ImGui::BeginDisabled();
        if (ImGui::Button("Inainte >##pag", ImVec2(80, 25))) paginaCurenta++;
        if (paginaCurenta >= totalPagini - 1) ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::SetNextItemWidth(60);
        const char* optiuni[] = { "10", "25", "50", "100" };
        int indexCurent = 2;
        if (produsePerPagina == 10) indexCurent = 0;
        else if (produsePerPagina == 25) indexCurent = 1;
        else if (produsePerPagina == 50) indexCurent = 2;
        else if (produsePerPagina == 100) indexCurent = 3;
        if (ImGui::Combo("##perPagina", &indexCurent, optiuni, 4)) {
            switch (indexCurent) {
                case 0: produsePerPagina = 10; break;
                case 1: produsePerPagina = 25; break;
                case 2: produsePerPagina = 50; break;
                case 3: produsePerPagina = 100; break;
            }
            paginaCurenta = 0;
        }
        ImGui::SameLine();
        ImGui::Text("/ pagina");
    }

    void renderFormularAdauga() {
        if (ImGui::CollapsingHeader("Adauga Produs Nou")) {
            ImGui::InputText("Nume##add", numeProdus, sizeof(numeProdus));
            ImGui::InputInt("Cantitate##add", &cantitate);
            ImGui::InputFloat("Pret##add", &pret, 0.1f, 1.0f, "%.2f");
            ImGui::InputInt("Prag Alerta##add", &pragAlerta);

            auto& zone = depozit.getZone();
            std::vector<Zona*> zoneVec;
            for (auto& [id, z] : zone) zoneVec.push_back(&z);
            std::sort(zoneVec.begin(), zoneVec.end(),
                [](Zona* a, Zona* b) { return a->getLitera() < b->getLitera(); });

            std::string zonaPreview = "Selecteaza zona";
            for (auto* z : zoneVec)
                if (z->getId() == zonaSelectata)
                    zonaPreview = z->getNume() + " (" +
                        std::to_string(depozit.getSpatiu(z->getId())) + " spatiu liber)";

            if (ImGui::BeginCombo("Zona##add", zonaPreview.c_str())) {
                for (auto* z : zoneVec) {
                    int spatiu = depozit.getSpatiu(z->getId());
                    bool sel = (zonaSelectata == z->getId());
                    std::string optiune = z->getNume() + " - " +
                        std::to_string(spatiu) + " spatiu liber";
                    if (spatiu == 0)
                        ImGui::TextDisabled("%s", optiune.c_str());
                    else if (ImGui::Selectable(optiune.c_str(), sel))
                        zonaSelectata = z->getId();
                }
                ImGui::EndCombo();
            }

            ImGui::InputText("Observatii##addObs", observatiiAdauga, sizeof(observatiiAdauga));
            ImGui::Spacing();

            if (ImGui::Button("Adauga##btn", ImVec2(120, 30))) {
                if (strlen(numeProdus) > 0 && cantitate >= 0 && pret >= 0) {
                    try {
                        Produs p(0, numeProdus, cantitate, pret, pragAlerta, zonaSelectata);
                        depozit.adaugaProdus(p, std::string(observatiiAdauga));
                        mesajStatus = "Produs adaugat cu succes!";
                        memset(numeProdus, 0, sizeof(numeProdus));
                        memset(observatiiAdauga, 0, sizeof(observatiiAdauga));
                        cantitate = 0; pret = 0; pragAlerta = 10;
                        alertaManager.refresh(getProduseVec());
                    } catch (const std::exception& e) {
                        mesajStatus = e.what();
                        Zona* zA = depozit.gasestZonaDisponibila(cantitate);
                        if (zA) {
                            zonaAlternativaAdaugaId = zA->getId();
                            numeProdusPending = numeProdus;
                            cantPending = cantitate;
                            pretPending = pret;
                            pragPending = pragAlerta;
                            obsPending = observatiiAdauga;
                            mesajZonaPlina = e.what();
                            showPopupZonaPlinaAdauga = true;
                        }
                    }
                }
            }
        }

        if (showPopupZonaPlinaAdauga) {
            ImGui::OpenPopup("Zona Plina - Adaugare##popup2");
            showPopupZonaPlinaAdauga = false;
        }

        if (ImGui::BeginPopupModal("Zona Plina - Adaugare##popup2", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "Zona selectata este plina!");
            ImGui::Text("%s", mesajZonaPlina.c_str());
            ImGui::Spacing();

            if (zonaAlternativaAdaugaId >= 0) {
                Zona* zA = depozit.getZona(zonaAlternativaAdaugaId);
                if (zA) {
                    ImGui::TextColored(ImVec4(0.2f,0.8f,0.2f,1),
                        "Zona disponibila: Zona %c (%.0f%% ocupata, %d spatiu liber)",
                        zA->getLitera(),
                        zA->getProcentOcupare(),
                        depozit.getSpatiu(zonaAlternativaAdaugaId));
                    ImGui::Spacing();

                    if (ImGui::Button("Adauga in aceasta zona", ImVec2(220, 30))) {
                        try {
                            Produs p(0, numeProdusPending, cantPending,
                                pretPending, pragPending, zonaAlternativaAdaugaId);
                            depozit.adaugaProdus(p, obsPending);
                            mesajStatus = "Produs adaugat in zona alternativa!";
                            memset(numeProdus, 0, sizeof(numeProdus));
                            memset(observatiiAdauga, 0, sizeof(observatiiAdauga));
                            cantitate = 0; pret = 0; pragAlerta = 10;
                            alertaManager.refresh(getProduseVec());
                        } catch (const std::exception& e) {
                            mesajStatus = e.what();
                        }
                        ImGui::CloseCurrentPopup();
                        zonaAlternativaAdaugaId = -1;
                    }
                    ImGui::SameLine();
                }
            }
            if (ImGui::Button("Inchide##p2", ImVec2(100, 30)))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
    }

    void renderGestiuneStoc() {
        if (produsSelectat < 0) return;
        Produs* p = depozit.getProdus(produsSelectat);
        if (!p) return;

        if (ImGui::CollapsingHeader("Gestioneaza Stoc", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Produs selectat: %s (stoc: %d)",
                p->getNume().c_str(), p->getCantitate());

            Zona* zonaCurenta = depozit.getZona(p->getZonaId());
            if (zonaCurenta) {
                int spatiu = depozit.getSpatiu(p->getZonaId());
                ImVec4 culoareSpatiu = spatiu > 100 ?
                    ImVec4(0.2f,0.8f,0.2f,1) :
                    spatiu > 0 ?
                    ImVec4(0.9f,0.6f,0.1f,1) :
                    ImVec4(1,0.3f,0.3f,1);
                ImGui::TextColored(culoareSpatiu,
                    "Zona %c: %d spatiu liber din %d",
                    zonaCurenta->getLitera(), spatiu,
                    zonaCurenta->getCapacitateMax());
            }

            ImGui::InputInt("Cantitate##stoc", &cantitateModificare);
            if (cantitateModificare < 1) cantitateModificare = 1;
            ImGui::InputText("Observatii##stocObs", observatiiStoc, sizeof(observatiiStoc));
            ImGui::Spacing();

            if (ImGui::Button("+ Adauga Stoc", ImVec2(130, 30))) {
                try {
                    depozit.adaugaStoc(produsSelectat, cantitateModificare,
                        std::string(observatiiStoc));
                    mesajStatus = "Stoc actualizat!";
                    memset(observatiiStoc, 0, sizeof(observatiiStoc));
                    alertaManager.refresh(getProduseVec());
                } catch (const std::exception& e) {
                    mesajStatus = e.what();
                    Zona* zA = depozit.gasestZonaDisponibila(cantitateModificare);
                    if (zA) {
                        zonaAlternativaId = zA->getId();
                        produsDeMusat = produsSelectat;
                        mesajZonaPlina = e.what();
                        showPopupZonaPlina = true;
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("- Scade Stoc", ImVec2(130, 30))) {
                try {
                    depozit.scadeStoc(produsSelectat, cantitateModificare,
                        std::string(observatiiStoc));
                    mesajStatus = "Stoc actualizat!";
                    memset(observatiiStoc, 0, sizeof(observatiiStoc));
                    alertaManager.refresh(getProduseVec());
                } catch (const std::exception& e) {
                    mesajStatus = e.what();
                }
            }

            if (userCurent.eAdmin()) {
                ImGui::SameLine();
                if (ImGui::Button("Editeaza", ImVec2(100, 30)))
                    editMode = !editMode;
                ImGui::SameLine();
                if (ImGui::Button("Sterge", ImVec2(100, 30))) {
                    try {
                        depozit.eliminaProdus(produsSelectat);
                        produsSelectat = -1;
                        editMode = false;
                        mesajStatus = "Produs sters!";
                        alertaManager.refresh(getProduseVec());
                    } catch (const std::exception& e) {
                        mesajStatus = e.what();
                    }
                }

                if (editMode && produsSelectat >= 0) {
                    ImGui::Spacing();
                    ImGui::Separator();
                    ImGui::Text("Editeaza produs:");
                    ImGui::InputText("Nume##edit", editNume, sizeof(editNume));
                    ImGui::InputFloat("Pret##edit", &editPret, 0.1f, 1.0f, "%.2f");
                    ImGui::InputInt("Prag Alerta##edit", &editPrag);

                    auto& zone = depozit.getZone();
                    std::vector<Zona*> zoneVec;
                    for (auto& [id, z] : zone) zoneVec.push_back(&z);
                    std::sort(zoneVec.begin(), zoneVec.end(),
                        [](Zona* a, Zona* b) { return a->getLitera() < b->getLitera(); });

                    std::string zonaPreview = "Selecteaza zona";
                    for (auto* z : zoneVec)
                        if (z->getId() == editZona)
                            zonaPreview = z->getNume();

                    if (ImGui::BeginCombo("Zona##edit", zonaPreview.c_str())) {
                        for (auto* z : zoneVec) {
                            int spatiu = depozit.getSpatiu(z->getId());
                            bool sel = (editZona == z->getId());
                            std::string optiune = z->getNume() + " - " +
                                std::to_string(spatiu) + " spatiu liber";
                            if (ImGui::Selectable(optiune.c_str(), sel))
                                editZona = z->getId();
                        }
                        ImGui::EndCombo();
                    }

                    ImGui::Spacing();
                    if (ImGui::Button("Salveaza##edit", ImVec2(120, 30))) {
                        try {
                            db.updateProdus(produsSelectat, editNume, editPret, editPrag, editZona);
                            depozit.incarcaDate();
                            mesajStatus = "Produs actualizat!";
                            editMode = false;
                            alertaManager.refresh(getProduseVec());
                        } catch (const std::exception& e) {
                            mesajStatus = e.what();
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Anuleaza##edit", ImVec2(120, 30)))
                        editMode = false;
                }
            }
        }

        if (showPopupZonaPlina) {
            ImGui::OpenPopup("Zona Plina##popup");
            showPopupZonaPlina = false;
        }

        if (ImGui::BeginPopupModal("Zona Plina##popup", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "Zona plina!");
            ImGui::Text("%s", mesajZonaPlina.c_str());
            ImGui::Spacing();

            if (zonaAlternativaId >= 0) {
                Zona* zA = depozit.getZona(zonaAlternativaId);
                if (zA) {
                    ImGui::TextColored(ImVec4(0.2f,0.8f,0.2f,1),
                        "Zona disponibila: Zona %c (%.0f%% ocupata, %d spatiu liber)",
                        zA->getLitera(),
                        zA->getProcentOcupare(),
                        depozit.getSpatiu(zonaAlternativaId));
                    ImGui::Spacing();

                    if (ImGui::Button("Muta produsul in aceasta zona", ImVec2(250, 30))) {
                        try {
                            depozit.mutaProdusInZona(produsDeMusat, zonaAlternativaId);
                            mesajStatus = "Produs mutat cu succes!";
                            alertaManager.refresh(getProduseVec());
                        } catch (const std::exception& e) {
                            mesajStatus = e.what();
                        }
                        ImGui::CloseCurrentPopup();
                        zonaAlternativaId = -1;
                        produsDeMusat = -1;
                    }
                    ImGui::SameLine();
                }
            }
            if (ImGui::Button("Inchide##p1", ImVec2(100, 30)))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
    }

    void renderAlerte() {
        if (!alertaManager.areAlerte()) return;
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1,0.3f,0.3f,1),
            "ALERTE ACTIVE: %d", alertaManager.getNrAlerte());
        ImGui::Separator();

        // Child scrollabil pentru alerte
        ImGui::BeginChild("##alerte_list", ImVec2(-1, 90), false);
        for (const auto& a : alertaManager.getAlerte()) {
            ImGui::TextColored(a.getCuloare(), "%s %s",
                a.getTipString().c_str(), a.mesaj.c_str());
        }
        ImGui::EndChild();
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

    void renderTabTranzactii() {
        ImGui::Text("ISTORIC TRANZACTII");
        ImGui::Separator();
        ImGui::Spacing();

        auto tranzactii = db.getTranzactii(100);

        std::sort(tranzactii.begin(), tranzactii.end(),
            [&](const TranzactieRecord& a, const TranzactieRecord& b) {
                bool cmp = false;
                switch (sortColTranzactii) {
                    case 0: cmp = a.id < b.id; break;
                    case 1: cmp = toLower(a.numeProdus) < toLower(b.numeProdus); break;
                    case 2: cmp = a.tip < b.tip; break;
                    case 3: cmp = a.cantitate < b.cantitate; break;
                    case 4: cmp = a.data < b.data; break;
                    case 5: cmp = toLower(a.observatii) < toLower(b.observatii); break;
                    default: cmp = a.id < b.id; break;
                }
                return sortAscTranzactii ? cmp : !cmp;
            });

        if (ImGui::BeginTable("tranzactii", 6,
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_ScrollY |
            ImGuiTableFlags_ScrollX |
            ImGuiTableFlags_Sortable))
        {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("ID",
                ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 40);
            ImGui::TableSetupColumn("Produs", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Tip", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableSetupColumn("Cantitate", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableSetupColumn("Data", ImGuiTableColumnFlags_WidthFixed, 140);
            ImGui::TableSetupColumn("Observatii", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
                if (sortSpecs->SpecsDirty && sortSpecs->SpecsCount > 0) {
                    sortColTranzactii = sortSpecs->Specs[0].ColumnIndex;
                    sortAscTranzactii = sortSpecs->Specs[0].SortDirection == ImGuiSortDirection_Ascending;
                    sortSpecs->SpecsDirty = false;
                }
            }

            for (const auto& t : tranzactii) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%d", t.id);
                ImGui::TableSetColumnIndex(1);
                ImGui::Text("%s", t.numeProdus.c_str());
                ImGui::TableSetColumnIndex(2);
                if (t.tip == "INTRARE")
                    ImGui::TextColored(ImVec4(0,1,0,1), "INTRARE");
                else if (t.tip == "MUTARE")
                    ImGui::TextColored(ImVec4(0.2f,0.7f,1.0f,1), "MUTARE");
                else
                    ImGui::TextColored(ImVec4(1,0.3f,0.3f,1), "IESIRE");
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%d", t.cantitate);
                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%s", t.data.c_str());
                ImGui::TableSetColumnIndex(5);
                ImGui::Text("%s", t.observatii.c_str());
            }
            ImGui::EndTable();
        }
    }

    void renderTabAdmin() {
        if (!userCurent.eAdmin()) {
            ImGui::TextColored(ImVec4(1,0.3f,0.3f,1),
                "Acces restrictionat - doar Admin");
            return;
        }

        if (ImGui::CollapsingHeader("Adauga Cont", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::InputText("Username##new", newUsername, sizeof(newUsername));
            ImGuiInputTextFlags parolaFlags = showParola ?
                ImGuiInputTextFlags_None : ImGuiInputTextFlags_Password;
            ImGui::InputText("Parola##new", newParola, sizeof(newParola), parolaFlags);
            ImGui::SameLine();
            if (ImGui::SmallButton(showParola ? "Ascunde##sp" : "Arata##sp"))
                showParola = !showParola;

            const char* roluri[] = { "Admin", "Angajat" };
            ImGui::Combo("Rol##new", &newRol, roluri, 2);
            ImGui::Spacing();

            if (ImGui::Button("Creeaza Cont", ImVec2(150, 30))) {
                if (strlen(newUsername) > 0 && strlen(newParola) > 0) {
                    try {
                        Rol r = (newRol == 0) ? Rol::ADMIN : Rol::ANGAJAT;
                        db.adaugaUser(newUsername, newParola, r);
                        mesajAdmin = std::string("Cont creat: ") + newUsername;
                        memset(newUsername, 0, sizeof(newUsername));
                        memset(newParola, 0, sizeof(newParola));
                        newRol = 1;
                    } catch (const std::exception& e) {
                        mesajAdmin = std::string("Eroare: ") + e.what();
                    }
                } else {
                    mesajAdmin = "Username si parola sunt obligatorii!";
                }
            }

            if (!mesajAdmin.empty()) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.2f,1.0f,0.2f,1.0f),
                    "%s", mesajAdmin.c_str());
            }
        }

        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Conturi Existente")) {
            auto users = db.getUsers();
            std::sort(users.begin(), users.end(),
                [&](const User& a, const User& b) {
                    bool cmp = false;
                    switch (sortColUsers) {
                        case 0: cmp = a.getId() < b.getId(); break;
                        case 1: cmp = toLower(a.getUsername()) < toLower(b.getUsername()); break;
                        case 2: cmp = a.getPasswordHash() < b.getPasswordHash(); break;
                        case 3: cmp = a.getRolString() < b.getRolString(); break;
                        default: cmp = a.getId() < b.getId(); break;
                    }
                    return sortAscUsers ? cmp : !cmp;
                });

            if (ImGui::BeginTable("users", 5,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_ScrollX |
                ImGuiTableFlags_Sortable, ImVec2(0, 200)))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("ID",
                    ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed, 40);
                ImGui::TableSetupColumn("Username", ImGuiTableColumnFlags_WidthFixed, 120);
                ImGui::TableSetupColumn("Parola (hash)", ImGuiTableColumnFlags_WidthFixed, 300);
                ImGui::TableSetupColumn("Rol", ImGuiTableColumnFlags_WidthFixed, 80);
                ImGui::TableSetupColumn("Actiuni",
                    ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 180);
                ImGui::TableHeadersRow();

                if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
                    if (sortSpecs->SpecsDirty && sortSpecs->SpecsCount > 0) {
                        sortColUsers = sortSpecs->Specs[0].ColumnIndex;
                        sortAscUsers = sortSpecs->Specs[0].SortDirection == ImGuiSortDirection_Ascending;
                        sortSpecs->SpecsDirty = false;
                    }
                }

                for (auto& u : users) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%d", u.getId());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", u.getUsername().c_str());
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%s", u.getPasswordHash().c_str());
                    ImGui::TableSetColumnIndex(3);
                    if (u.eAdmin())
                        ImGui::TextColored(ImVec4(0.2f,0.7f,1.0f,1), "Admin");
                    else
                        ImGui::Text("Angajat");
                    ImGui::TableSetColumnIndex(4);
                    if (u.getUsername() != "admin") {
                        if (u.eActiv()) {
                            std::string btn = "Dezactiveaza##" + std::to_string(u.getId());
                            if (ImGui::SmallButton(btn.c_str())) {
                                db.dezactiveazaUser(u.getId());
                                mesajAdmin = "Cont dezactivat: " + u.getUsername();
                            }
                        } else {
                            ImGui::TextColored(ImVec4(0.5f,0.5f,0.5f,1), "[Inactiv]");
                            ImGui::SameLine();
                            std::string btn = "Activeaza##" + std::to_string(u.getId());
                            if (ImGui::SmallButton(btn.c_str())) {
                                db.activeazaUser(u.getId());
                                mesajAdmin = "Cont activat: " + u.getUsername();
                            }
                        }
                    } else {
                        ImGui::TextDisabled("Cont sistem");
                    }
                }
                ImGui::EndTable();
            }
        }

        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Categorii")) {
            ImGui::SetNextItemWidth(200);
            ImGui::InputText("Cauta##catSearch", cautareCategorie, sizeof(cautareCategorie));
            ImGui::Spacing();

            auto categorii = db.getCategorii();
            std::sort(categorii.begin(), categorii.end(),
                [&](const Categorie& a, const Categorie& b) {
                    bool cmp = false;
                    switch (sortColCategorii) {
                        case 0: cmp = toLower(a.getNume()) < toLower(b.getNume()); break;
                        case 1: cmp = toLower(a.getDescriere()) < toLower(b.getDescriere()); break;
                        default: cmp = toLower(a.getNume()) < toLower(b.getNume()); break;
                    }
                    return sortAscCategorii ? cmp : !cmp;
                });

            if (ImGui::BeginTable("categorii", 2,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable,
                ImVec2(0, 150)))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Nume",
                    ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Descriere", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
                    if (sortSpecs->SpecsDirty && sortSpecs->SpecsCount > 0) {
                        sortColCategorii = sortSpecs->Specs[0].ColumnIndex;
                        sortAscCategorii = sortSpecs->Specs[0].SortDirection == ImGuiSortDirection_Ascending;
                        sortSpecs->SpecsDirty = false;
                    }
                }

                std::string filtruCat(cautareCategorie);
                for (const auto& c : categorii) {
                    if (!filtruCat.empty() &&
                        toLower(c.getNume()).find(toLower(filtruCat)) == std::string::npos)
                        continue;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", c.getNume().c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", c.getDescriere().c_str());
                }
                ImGui::EndTable();
            }
        }

        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Furnizori")) {
            ImGui::SetNextItemWidth(200);
            ImGui::InputText("Cauta##furnSearch", cautareFurnizor, sizeof(cautareFurnizor));
            ImGui::Spacing();

            auto furnizori = db.getFurnizori();
            std::sort(furnizori.begin(), furnizori.end(),
                [&](const Furnizor& a, const Furnizor& b) {
                    bool cmp = false;
                    switch (sortColFurnizori) {
                        case 0: cmp = toLower(a.getNume()) < toLower(b.getNume()); break;
                        case 1: cmp = a.getTelefon() < b.getTelefon(); break;
                        case 2: cmp = toLower(a.getEmail()) < toLower(b.getEmail()); break;
                        default: cmp = toLower(a.getNume()) < toLower(b.getNume()); break;
                    }
                    return sortAscFurnizori ? cmp : !cmp;
                });

            if (ImGui::BeginTable("furnizori", 3,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_Sortable,
                ImVec2(0, 150)))
            {
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableSetupColumn("Nume",
                    ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Telefon", ImGuiTableColumnFlags_WidthFixed, 120);
                ImGui::TableSetupColumn("Email", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                if (ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs()) {
                    if (sortSpecs->SpecsDirty && sortSpecs->SpecsCount > 0) {
                        sortColFurnizori = sortSpecs->Specs[0].ColumnIndex;
                        sortAscFurnizori = sortSpecs->Specs[0].SortDirection == ImGuiSortDirection_Ascending;
                        sortSpecs->SpecsDirty = false;
                    }
                }

                std::string filtruFurn(cautareFurnizor);
                for (const auto& f : furnizori) {
                    if (!filtruFurn.empty() &&
                        toLower(f.getNume()).find(toLower(filtruFurn)) == std::string::npos)
                        continue;
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%s", f.getNume().c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%s", f.getTelefon().c_str());
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%s", f.getEmail().c_str());
                }
                ImGui::EndTable();
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

    bool trebuieLogout() const { return shouldLogout; }

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
        ImGui::SameLine(ImGui::GetWindowWidth() - 350);
        ImGui::Text("User: %s (%s)",
            userCurent.getUsername().c_str(),
            userCurent.getRolString().c_str());
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f,0.1f,0.1f,1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f,0.2f,0.2f,1.0f));
        if (ImGui::Button("Logout", ImVec2(80, 0)))
            shouldLogout = true;
        ImGui::PopStyleColor(2);
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTabBar("##tabs")) {
            if (ImGui::BeginTabItem("Dashboard")) {
                renderTabDashboard();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Tranzactii")) {
                renderTabTranzactii();
                ImGui::EndTabItem();
            }
            if (userCurent.eAdmin()) {
                if (ImGui::BeginTabItem("Admin")) {
                    renderTabAdmin();
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    }
};