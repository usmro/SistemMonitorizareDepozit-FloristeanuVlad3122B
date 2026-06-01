#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include "../models/Produs.h"
#include "../models/Zona.h"

struct RaportProdus {
    std::string nume;
    int cantitate;
    double valoare;
    bool subPrag;
};

struct RaportZona {
    char litera;
    float procentOcupare;
    int capacitateCurenta;
    int capacitateMax;
};

class Raport {
public:
    // Top 5 produse cu stoc critic (sortate dupa cantitate)
    static std::vector<RaportProdus> getTopCritice(
        const std::unordered_map<int, Produs>& produse, int n = 5)
    {
        std::vector<RaportProdus> rezultat;
        for (const auto& [id, p] : produse) {
            rezultat.push_back({
                p.getNume(),
                p.getCantitate(),
                p.getValoareTotala(),
                p.eSubPrag()
            });
        }
        std::sort(rezultat.begin(), rezultat.end(),
            [](const RaportProdus& a, const RaportProdus& b) {
                return a.cantitate < b.cantitate;
            });
        if (rezultat.size() > n)
            rezultat.resize(n);
        return rezultat;
    }

    // Valoare totala stoc
    static double getValoareTotala(
        const std::unordered_map<int, Produs>& produse)
    {
        double total = 0;
        for (const auto& [id, p] : produse)
            total += p.getValoareTotala();
        return total;
    }

    // Raport zone
    static std::vector<RaportZona> getRaportZone(
        const std::unordered_map<int, Zona>& zone)
    {
        std::vector<RaportZona> rezultat;
        for (const auto& [id, z] : zone) {
            rezultat.push_back({
                z.getLitera(),
                z.getProcentOcupare(),
                z.getCapacitateCurenta(),
                z.getCapacitateMax()
            });
        }
        std::sort(rezultat.begin(), rezultat.end(),
            [](const RaportZona& a, const RaportZona& b) {
                return a.litera < b.litera;
            });
        return rezultat;
    }

    // Numar produse sub prag
    static int getNrSubPrag(
        const std::unordered_map<int, Produs>& produse)
    {
        int n = 0;
        for (const auto& [id, p] : produse)
            if (p.eSubPrag()) n++;
        return n;
    }
};