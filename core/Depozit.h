#pragma once
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "../models/Produs.h"
#include "../models/Zona.h"
#include "../data/Database.h"

class Depozit {
private:
    std::unordered_map<int, Produs> produse;
    std::unordered_map<int, Zona> zone;
    Database& db;

public:
    Depozit(Database& db) : db(db) {
        db.recalculeazaZone();
        incarcaDate();
    }

    void incarcaDate() {
        produse.clear();
        zone.clear();
        for (auto& p : db.getProduse())
            produse[p.getId()] = p;
        for (auto& z : db.getZone())
            zone[z.getId()] = z;
    }

    void adaugaProdus(Produs& p, const std::string& observatii = "") {
        db.adaugaProdus(p);
        db.recalculeazaZone();
        incarcaDate();
        for (auto& [id, prod] : produse) {
            if (prod.getNume() == p.getNume() &&
                prod.getCantitate() == p.getCantitate()) {
                if (p.getCantitate() > 0)
                    db.adaugaTranzactie(id, "INTRARE", p.getCantitate(),
                        observatii.empty() ? "Produs nou adaugat" : observatii);
                break;
            }
        }
    }

    void eliminaProdus(int id) {
        if (produse.find(id) == produse.end())
            throw std::runtime_error("Produsul nu exista!");
        db.eliminaProdus(id);
        db.recalculeazaZone();
        produse.erase(id);
        incarcaDate();
    }

    void adaugaStoc(int id, int cantitate, const std::string& observatii = "") {
        if (produse.find(id) == produse.end())
            throw std::runtime_error("Produsul nu exista!");
        produse[id] += cantitate;
        db.updateCantitate(id, produse[id].getCantitate());
        db.adaugaTranzactie(id, "INTRARE", cantitate, observatii);
        db.recalculeazaZone();
        incarcaDate();
    }

    void scadeStoc(int id, int cantitate, const std::string& observatii = "") {
        if (produse.find(id) == produse.end())
            throw std::runtime_error("Produsul nu exista!");
        produse[id] -= cantitate;
        db.updateCantitate(id, produse[id].getCantitate());
        db.adaugaTranzactie(id, "IESIRE", cantitate, observatii);
        db.recalculeazaZone();
        incarcaDate();
    }

    std::unordered_map<int, Produs>& getProduse() { return produse; }
    std::unordered_map<int, Zona>& getZone() { return zone; }

    Produs* getProdus(int id) {
        auto it = produse.find(id);
        return it != produse.end() ? &it->second : nullptr;
    }

    Zona* getZona(int id) {
        auto it = zone.find(id);
        return it != zone.end() ? &it->second : nullptr;
    }

    std::vector<Produs> getProduseSubPrag() const {
        std::vector<Produs> rezultat;
        for (const auto& [id, p] : produse)
            if (p.eSubPrag()) rezultat.push_back(p);
        return rezultat;
    }

    std::vector<Produs> getProduseSortateDupaCantitate() const {
        std::vector<Produs> rezultat;
        for (const auto& [id, p] : produse)
            rezultat.push_back(p);
        std::sort(rezultat.begin(), rezultat.end(),
            [](const Produs& a, const Produs& b) {
                return a.getCantitate() < b.getCantitate();
            });
        return rezultat;
    }

    double getValoareTotalaStoc() const {
        double total = 0;
        for (const auto& [id, p] : produse)
            total += p.getValoareTotala();
        return total;
    }

    int getNrProduse() const { return produse.size(); }
    int getNrAlerte() const { return getProduseSubPrag().size(); }
};