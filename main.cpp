#include <iostream>
#include <string>
#include <unordered_map>
#include <stdexcept> // Avem nevoie de asta pentru excepțiile standard

using namespace std;

// ================= CLASA PRODUS =================
class Produs {
private:
    int id;
    string nume;
    int cantitate;
    double pret;
    int pragAlerta;

public:
    Produs() : id(0), cantitate(0), pret(0.0), pragAlerta(0) {}
    Produs(int id, string nume, int cantitate, double pret, int pragAlerta)
        : id(id), nume(nume), cantitate(cantitate), pret(pret), pragAlerta(pragAlerta) {}

    int getId() const { return id; }
    string getNume() const { return nume; }
    int getCantitate() const { return cantitate; }
    int getPragAlerta() const { return pragAlerta; }

    // Operator += pentru intrare stoc
    Produs& operator+=(int cantitateAdaugata) {
        if (cantitateAdaugata < 0) {
            throw invalid_argument("Nu poti adauga o cantitate negativa!");
        }
        this->cantitate += cantitateAdaugata;
        return *this;
    }

    // Operator -= pentru iesire stoc
    Produs& operator-=(int cantitateScazuta) {
        if (cantitateScazuta < 0) {
            throw invalid_argument("Nu poti scadea o cantitate negativa!");
        }
        if (this->cantitate - cantitateScazuta < 0) {
            throw runtime_error("Stoc insuficient pentru aceasta operatiune!");
        }
        this->cantitate -= cantitateScazuta;
        return *this;
    }

    void afisare() const {
        cout << "ID: " << id << " | Nume: " << nume << " | Stoc: " << cantitate
             << " | Pret: " << pret << " | Prag: " << pragAlerta << endl;
    }
};

// ================= CLASA DEPOZIT =================
class Depozit {
private:
    unordered_map<int, Produs> stocuri;

public:
    void adaugaProdus(const Produs& p) {
        if (stocuri.find(p.getId()) != stocuri.end()) {
            throw runtime_error("Eroare: Produsul cu acest ID exista deja in depozit!");
        }
        stocuri[p.getId()] = p;
    }

    void eliminaProdus(int id) {
        if (stocuri.find(id) == stocuri.end()) {
            throw runtime_error("Eroare: Nu poti sterge un produs care nu exista!");
        }
        stocuri.erase(id);
    }

    void adaugaStoc(int id, int cantitate) {
        if (stocuri.find(id) == stocuri.end()) {
            throw runtime_error("Eroare: Produsul nu exista!");
        }
        stocuri[id] += cantitate; // Aici se apeleaza automat operatorul +=
    }

    void scadeStoc(int id, int cantitate) {
        if (stocuri.find(id) == stocuri.end()) {
            throw runtime_error("Eroare: Produsul nu exista!");
        }
        stocuri[id] -= cantitate; // Aici se apeleaza automat operatorul -=
    }

    void genereazaRaportAlerte() const {
        cout << "\n--- ALERTE STOC ---" << endl;
        bool gasit = false;
        for (const auto& pereche : stocuri) {
            if (pereche.second.getCantitate() < pereche.second.getPragAlerta()) {
                pereche.second.afisare();
                gasit = true;
            }
        }
        if (!gasit) cout << "Totul este OK. Niciun produs sub pragul de alerta." << endl;
        cout << "-------------------\n" << endl;
    }
};

// ================= MAIN =================
int main() {
    Depozit depozit;

    try {
        depozit.adaugaProdus(Produs(1, "Laptop", 50, 3500.0, 10));
        depozit.adaugaProdus(Produs(2, "Mouse", 15, 120.0, 20)); // Prag 20, stoc 15 -> va da alerta

        cout << "Adaugam 5 Laptopuri..." << endl;
        depozit.adaugaStoc(1, 5);

        cout << "Vindem 10 Mouse-uri..." << endl;
        depozit.scadeStoc(2, 10);

        depozit.genereazaRaportAlerte();

        // Fortam o eroare pentru a testa exceptiile
        cout << "Incercam sa vindem 100 de laptopuri..." << endl;
        depozit.scadeStoc(1, 100);

    } catch (const exception& e) {
        // Aici ajunge orice eroare aruncata cu "throw"
        cout << "[EROARE PRINSA]: " << e.what() << endl;
    }

    return 0;
}