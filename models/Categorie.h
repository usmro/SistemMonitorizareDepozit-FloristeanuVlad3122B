#pragma once
#include <string>
#include "EntitateDepozit.h"

class Categorie : public EntitateDepozit {
private:
    std::string descriere;

public:
    Categorie() : EntitateDepozit(0, "") {}
    Categorie(int id, std::string nume, std::string descriere = "")
        : EntitateDepozit(id, nume), descriere(descriere) {}

    std::string getDescriere() const { return descriere; }
    void setDescriere(std::string d) { descriere = d; }

    // Implementare functii virtuale pure
    void afisare() const override {
        std::cout << getTip() << " [" << id << "]: " << nume
                  << " | " << descriere << std::endl;
    }

    std::string getTip() const override {
        return "Categorie";
    }

    std::string getInfo() const override {
        return nume + " - " + descriere;
    }
};