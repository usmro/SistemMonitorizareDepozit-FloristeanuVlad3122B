#pragma once
#include <vector>
#include <string>
#include "../models/Produs.h"
#include "imgui.h"

struct Alerta {
    enum class Tip { CRITIC, AVERTISMENT, INFO };

    std::string mesaj;
    std::string numeProdus;
    int produsId;
    Tip tip;

    ImVec4 getCuloare() const {
        switch(tip) {
            case Tip::CRITIC:      return ImVec4(0.9f, 0.2f, 0.2f, 1.0f); // Rosu
            case Tip::AVERTISMENT: return ImVec4(0.9f, 0.6f, 0.1f, 1.0f); // Portocaliu
            default:               return ImVec4(0.2f, 0.8f, 0.2f, 1.0f); // Verde
        }
    }

    std::string getTipString() const {
        switch(tip) {
            case Tip::CRITIC:      return "[CRITIC]";
            case Tip::AVERTISMENT: return "[ATENTIE]";
            default:               return "[INFO]";
        }
    }
};

class AlertaManager {
private:
    std::vector<Alerta> alerte;

public:
    void refresh(const std::vector<Produs>& produse) {
        alerte.clear();
        for (const auto& p : produse) {
            if (p.getCantitate() == 0) {
                alerte.push_back({
                    "Stoc EPUIZAT: " + p.getNume(),
                    p.getNume(),
                    p.getId(),
                    Alerta::Tip::CRITIC
                });
            } else if (p.eSubPrag()) {
                alerte.push_back({
                    "Stoc scazut: " + p.getNume() +
                    " (" + std::to_string(p.getCantitate()) +
                    "/" + std::to_string(p.getPragAlerta()) + ")",
                    p.getNume(),
                    p.getId(),
                    Alerta::Tip::AVERTISMENT
                });
            }
        }
    }

    const std::vector<Alerta>& getAlerte() const { return alerte; }
    int getNrAlerte() const { return alerte.size(); }
    int getNrCritice() const {
        int n = 0;
        for (const auto& a : alerte)
            if (a.tip == Alerta::Tip::CRITIC) n++;
        return n;
    }
    bool areAlerte() const { return !alerte.empty(); }
};