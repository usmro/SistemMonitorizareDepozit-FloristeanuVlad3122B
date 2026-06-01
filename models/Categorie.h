#pragma once
#include <string>

class Categorie {
private:
    int id;
    std::string nume;
    std::string descriere;

public:
    Categorie() : id(0) {}
    Categorie(int id, std::string nume, std::string descriere = "")
        : id(id), nume(nume), descriere(descriere) {}

    int getId() const { return id; }
    std::string getNume() const { return nume; }
    std::string getDescriere() const { return descriere; }

    void setNume(std::string n) { nume = n; }
    void setDescriere(std::string d) { descriere = d; }
};