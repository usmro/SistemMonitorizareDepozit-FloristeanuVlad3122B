#pragma once
#include <string>
#include <vector>

class Furnizor {
private:
    int id;
    std::string nume;
    std::string telefon;
    std::string email;
    std::vector<int> produseIds;

public:
    Furnizor() : id(0) {}
    Furnizor(int id, std::string nume, std::string telefon = "", std::string email = "")
        : id(id), nume(nume), telefon(telefon), email(email) {}

    int getId() const { return id; }
    std::string getNume() const { return nume; }
    std::string getTelefon() const { return telefon; }
    std::string getEmail() const { return email; }
    std::vector<int> getProduseIds() const { return produseIds; }

    void setNume(std::string n) { nume = n; }
    void setTelefon(std::string t) { telefon = t; }
    void setEmail(std::string e) { email = e; }

    void adaugaProdus(int produsId) { produseIds.push_back(produsId); }
    void eliminaProdus(int produsId) {
        produseIds.erase(
            std::remove(produseIds.begin(), produseIds.end(), produsId),
            produseIds.end()
        );
    }
};