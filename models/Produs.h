#pragma once
#include <string>

class Produs {
private:
    int id;
    std::string nume;
    int cantitate;
    double pret;
    int pragAlerta;
    int zonaId;
    int categorieId;
    int furnizorId;

public:
    Produs() : id(0), cantitate(0), pret(0.0), pragAlerta(0), zonaId(0), categorieId(0), furnizorId(0) {}

    Produs(int id, std::string nume, int cantitate, double pret, int pragAlerta, int zonaId = 0, int categorieId = 0, int furnizorId = 0)
        : id(id), nume(nume), cantitate(cantitate), pret(pret), pragAlerta(pragAlerta), zonaId(zonaId), categorieId(categorieId), furnizorId(furnizorId) {}

    // Getters
    int getId() const { return id; }
    std::string getNume() const { return nume; }
    int getCantitate() const { return cantitate; }
    double getPret() const { return pret; }
    int getPragAlerta() const { return pragAlerta; }
    int getZonaId() const { return zonaId; }
    int getCategorieId() const { return categorieId; }
    int getFurnizorId() const { return furnizorId; }

    // Setters
    void setNume(std::string n) { nume = n; }
    void setCantitate(int c) { cantitate = c; }
    void setPret(double p) { pret = p; }
    void setPragAlerta(int p) { pragAlerta = p; }
    void setZonaId(int z) { zonaId = z; }
    void setCategorieId(int c) { categorieId = c; }
    void setFurnizorId(int f) { furnizorId = f; }

    bool eSubPrag() const { return cantitate < pragAlerta; }
    double getValoareTotala() const { return cantitate * pret; }

    // Operatori
    Produs& operator+=(int cantitateAdaugata) {
        if (cantitateAdaugata < 0)
            throw std::invalid_argument("Nu poti adauga o cantitate negativa!");
        cantitate += cantitateAdaugata;
        return *this;
    }

    Produs& operator-=(int cantitateScazuta) {
        if (cantitateScazuta < 0)
            throw std::invalid_argument("Nu poti scadea o cantitate negativa!");
        if (cantitate - cantitateScazuta < 0)
            throw std::runtime_error("Stoc insuficient!");
        cantitate -= cantitateScazuta;
        return *this;
    }
};