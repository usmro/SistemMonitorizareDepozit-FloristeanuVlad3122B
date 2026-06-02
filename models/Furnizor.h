#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include "EntitateDepozit.h"

class Furnizor : public EntitateDepozit {
private:
    std::string telefon;
    std::string email;
    std::vector<int> produseIds;

public:
    Furnizor() : EntitateDepozit(0, "") {}
    Furnizor(int id, std::string nume,
             std::string telefon = "", std::string email = "")
        : EntitateDepozit(id, nume), telefon(telefon), email(email) {}

    std::string getTelefon() const { return telefon; }
    std::string getEmail() const { return email; }
    std::vector<int> getProduseIds() const { return produseIds; }

    void setTelefon(std::string t) { telefon = t; }
    void setEmail(std::string e) { email = e; }

    void adaugaProdus(int produsId) {
        produseIds.push_back(produsId);
    }

    void eliminaProdus(int produsId) {
        auto it = std::find(produseIds.begin(), produseIds.end(), produsId);
        if (it != produseIds.end())
            produseIds.erase(it);
    }

    // Implementare functii virtuale pure
    void afisare() const override {
        std::cout << getTip() << " [" << id << "]: " << nume
                  << " | Tel: " << telefon
                  << " | Email: " << email << std::endl;
    }

    std::string getTip() const override {
        return "Furnizor";
    }

    std::string getInfo() const override {
        return nume + " (tel: " + telefon + ")";
    }
};