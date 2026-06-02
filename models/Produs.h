#pragma once
#include <string>
#include <stdexcept>
#include "EntitateDepozit.h"

class Produs : public EntitateDepozit {
private:
    int cantitate;
    double pret;
    int pragAlerta;
    int zonaId;
    int categorieId;
    int furnizorId;

    // ===== MEMBRU STATIC =====
    static int nrTotalInstante;

public:
    Produs() : EntitateDepozit(0, ""), cantitate(0), pret(0.0),
        pragAlerta(0), zonaId(0), categorieId(0), furnizorId(0) {
        nrTotalInstante++;
    }

    Produs(int id, std::string nume, int cantitate, double pret,
           int pragAlerta, int zonaId = 0, int categorieId = 0, int furnizorId = 0)
        : EntitateDepozit(id, nume), cantitate(cantitate), pret(pret),
          pragAlerta(pragAlerta), zonaId(zonaId),
          categorieId(categorieId), furnizorId(furnizorId) {
        nrTotalInstante++;
    }

    // Constructor de copiere
    Produs(const Produs& other)
        : EntitateDepozit(other.id, other.nume),
          cantitate(other.cantitate), pret(other.pret),
          pragAlerta(other.pragAlerta), zonaId(other.zonaId),
          categorieId(other.categorieId), furnizorId(other.furnizorId) {
        nrTotalInstante++;
    }

    // Operator de atribuire
    Produs& operator=(const Produs& other) {
        if (this != &other) {
            id = other.id;
            nume = other.nume;
            cantitate = other.cantitate;
            pret = other.pret;
            pragAlerta = other.pragAlerta;
            zonaId = other.zonaId;
            categorieId = other.categorieId;
            furnizorId = other.furnizorId;
        }
        return *this;
    }

    ~Produs() override {
        nrTotalInstante--;
    }

    // ===== STATIC GETTER =====
    static int getNrTotalInstante() { return nrTotalInstante; }

    // Getteri
    int getCantitate() const { return cantitate; }
    double getPret() const { return pret; }
    int getPragAlerta() const { return pragAlerta; }
    int getZonaId() const { return zonaId; }
    int getCategorieId() const { return categorieId; }
    int getFurnizorId() const { return furnizorId; }

    // Setteri
    void setCantitate(int c) { cantitate = c; }
    void setPret(double p) { pret = p; }
    void setPragAlerta(int p) { pragAlerta = p; }
    void setZonaId(int z) { zonaId = z; }
    void setCategorieId(int c) { categorieId = c; }
    void setFurnizorId(int f) { furnizorId = f; }

    bool eSubPrag() const { return cantitate < pragAlerta; }
    double getValoareTotala() const { return cantitate * pret; }

    // Implementare functii virtuale pure
    void afisare() const override {
        std::cout << getTip() << " [" << id << "]: " << nume
                  << " | Stoc: " << cantitate
                  << " | Pret: " << pret
                  << " | Prag: " << pragAlerta << std::endl;
    }

    std::string getTip() const override { return "Produs"; }

    std::string getInfo() const override {
        return nume + " (stoc: " + std::to_string(cantitate) +
               ", pret: " + std::to_string((int)pret) + " RON)";
    }

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

// Definire membru static
inline int Produs::nrTotalInstante = 0;