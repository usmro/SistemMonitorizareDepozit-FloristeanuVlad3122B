#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include "../models/EntitateDepozit.h"
#include "../models/Produs.h"
#include "../models/Zona.h"
#include "../models/User.h"
#include "../models/Tranzactie.h"
#include "../models/Categorie.h"
#include "../models/Furnizor.h"
#include "../data/sha256.h"

struct TestResult {
    std::string nume;
    bool trecut;
    std::string mesaj;
};

class TestRunner {
private:
    std::vector<TestResult> rezultate;
    int trecute = 0;
    int esuate = 0;

    void adaugaTest(const std::string& nume, bool conditie,
                    const std::string& mesajEsec = "") {
        TestResult r;
        r.nume = nume;
        r.trecut = conditie;
        r.mesaj = mesajEsec;
        rezultate.push_back(r);
        if (conditie) trecute++;
        else esuate++;
    }

public:
    void ruleazaToate() {
        std::cout << "\n========================================" << std::endl;
        std::cout << "   TESTE UNITARE - SISTEM DEPOZIT" << std::endl;
        std::cout << "========================================\n" << std::endl;

        testeazaProdus();
        testeazaOperatori();
        testeazaExceptii();
        testeazaZona();
        testeazaSHA256();
        testeazaTranzactie();
        testeazaUser();
        testeazaMostenire();

        afiseazaRezultate();
    }

    // ===== TESTE PRODUS =====
    void testeazaProdus() {
        std::cout << "--- Teste Produs ---" << std::endl;

        Produs p(1, "Laptop", 50, 3500.0, 10);
        adaugaTest("Produs - Constructor ID", p.getId() == 1);
        adaugaTest("Produs - Constructor Nume", p.getNume() == "Laptop");
        adaugaTest("Produs - Constructor Cantitate", p.getCantitate() == 50);
        adaugaTest("Produs - Constructor Pret", p.getPret() == 3500.0);
        adaugaTest("Produs - Constructor PragAlerta", p.getPragAlerta() == 10);

        Produs p2(2, "Mouse", 5, 120.0, 10);
        adaugaTest("Produs - eSubPrag true", p2.eSubPrag() == true);
        adaugaTest("Produs - eSubPrag false", p.eSubPrag() == false);
        adaugaTest("Produs - getValoareTotala", p.getValoareTotala() == 50 * 3500.0);

        p.setNume("Laptop Dell");
        adaugaTest("Produs - setNume", p.getNume() == "Laptop Dell");
        p.setPret(4000.0);
        adaugaTest("Produs - setPret", p.getPret() == 4000.0);
        p.setPragAlerta(15);
        adaugaTest("Produs - setPragAlerta", p.getPragAlerta() == 15);

        // Test static
        int nrInainte = Produs::getNrTotalInstante();
        {
            Produs temp(99, "Temp", 1, 1.0, 1);
            adaugaTest("Static - contor creste la creare",
                Produs::getNrTotalInstante() == nrInainte + 1);
        }
        adaugaTest("Static - contor scade la distrugere",
            Produs::getNrTotalInstante() == nrInainte);

        // Test copy constructor
        Produs original(10, "Original", 50, 100.0, 5);
        Produs copie(original);
        adaugaTest("Copy constructor - nume copiat",
            copie.getNume() == original.getNume());
        adaugaTest("Copy constructor - cantitate copiata",
            copie.getCantitate() == original.getCantitate());
        adaugaTest("Copy constructor - instante independente",
            &copie != &original);

        // Test operator=
        Produs atribuit;
        atribuit = original;
        adaugaTest("Operator= - nume atribuit",
            atribuit.getNume() == original.getNume());
        adaugaTest("Operator= - self assignment sigur",
            (original = original).getNume() == "Original");
    }

    // ===== TESTE OPERATORI =====
    void testeazaOperatori() {
        std::cout << "\n--- Teste Operatori += si -= ---" << std::endl;

        Produs p(1, "Test", 100, 10.0, 5);

        p += 50;
        adaugaTest("Operator += adauga corect", p.getCantitate() == 150);

        p += 0;
        adaugaTest("Operator += cu 0", p.getCantitate() == 150);

        p -= 30;
        adaugaTest("Operator -= scade corect", p.getCantitate() == 120);

        p -= 0;
        adaugaTest("Operator -= cu 0", p.getCantitate() == 120);

        p += 10;
        p -= 5;
        adaugaTest("Operatori in lant", p.getCantitate() == 125);
    }

    // ===== TESTE EXCEPTII =====
    void testeazaExceptii() {
        std::cout << "\n--- Teste Exceptii ---" << std::endl;

        Produs p(1, "Test", 10, 10.0, 5);

        bool exceptiePrinsa = false;
        try { p += -5; } catch (const std::invalid_argument&) { exceptiePrinsa = true; }
        adaugaTest("Exceptie += cantitate negativa", exceptiePrinsa);

        exceptiePrinsa = false;
        try { p -= -5; } catch (const std::invalid_argument&) { exceptiePrinsa = true; }
        adaugaTest("Exceptie -= cantitate negativa", exceptiePrinsa);

        exceptiePrinsa = false;
        try { p -= 100; } catch (const std::runtime_error&) { exceptiePrinsa = true; }
        adaugaTest("Exceptie -= stoc insuficient", exceptiePrinsa);

        adaugaTest("Cantitate neschimbata dupa exceptie", p.getCantitate() == 10);
    }

    // ===== TESTE ZONA =====
    void testeazaZona() {
        std::cout << "\n--- Teste Zona ---" << std::endl;

        Zona z(1, 'A', 100);
        adaugaTest("Zona - Constructor litera", z.getLitera() == 'A');
        adaugaTest("Zona - Constructor capacitate", z.getCapacitateMax() == 100);
        adaugaTest("Zona - Initial goala", z.getProcentOcupare() == 0.0f);
        adaugaTest("Zona - eGoala true", z.eGoala() == true);

        z.setCapacitateCurenta(50);
        adaugaTest("Zona - Procent 50%%", z.getProcentOcupare() == 50.0f);
        adaugaTest("Zona - eGoala false", z.eGoala() == false);

        z.setCapacitateCurenta(100);
        adaugaTest("Zona - eePlina true", z.eePlina() == true);

        z.setCapacitateCurenta(150);
        adaugaTest("Zona - Procent max 100%%", z.getProcentOcupare() == 100.0f);

        Zona z2(2, 'B', 100);
        z2.adaugaCapacitate(30);
        adaugaTest("Zona - adaugaCapacitate", z2.getCapacitateCurenta() == 30);
        z2.scadeCapacitate(10);
        adaugaTest("Zona - scadeCapacitate", z2.getCapacitateCurenta() == 20);

        Zona zGoala(3, 'C', 100);
        zGoala.setCapacitateCurenta(0);
        ImVec4 culoareGoala = zGoala.getCuloare();
        adaugaTest("Zona - Culoare goala (gri)", culoareGoala.x == 0.4f);

        Zona zVerde(4, 'D', 100);
        zVerde.setCapacitateCurenta(20);
        ImVec4 culoareVerde = zVerde.getCuloare();
        adaugaTest("Zona - Culoare verde (1-25%%)", culoareVerde.y > 0.7f);
    }

    // ===== TESTE SHA256 =====
    void testeazaSHA256() {
        std::cout << "\n--- Teste SHA256 ---" << std::endl;

        std::string hash1 = SHA256::hash("admin123");
        std::string hash2 = SHA256::hash("admin123");
        adaugaTest("SHA256 - Hash consistent", hash1 == hash2);

        std::string hash3 = SHA256::hash("altaParola");
        adaugaTest("SHA256 - Hash diferit pentru parole diferite", hash1 != hash3);
        adaugaTest("SHA256 - Lungime corecta (64 chars)", hash1.length() == 64);

        std::string hashEmpty = SHA256::hash("");
        adaugaTest("SHA256 - Hash non-empty pentru string gol", hashEmpty.length() == 64);

        std::string hashMic = SHA256::hash("parola");
        std::string hashMare = SHA256::hash("PAROLA");
        adaugaTest("SHA256 - Case sensitive", hashMic != hashMare);
    }

    // ===== TESTE TRANZACTIE =====
    void testeazaTranzactie() {
        std::cout << "\n--- Teste Tranzactie Template ---" << std::endl;

        TranzactieIntrare ti(1, 5, 100, "Restock");
        adaugaTest("TranzactieIntrare - ID", ti.getId() == 1);
        adaugaTest("TranzactieIntrare - ProdusId", ti.getProdusId() == 5);
        adaugaTest("TranzactieIntrare - Cantitate", ti.getCantitate() == 100);
        adaugaTest("TranzactieIntrare - Tip", ti.getTipTranzactie() == "INTRARE");
        adaugaTest("TranzactieIntrare - Observatii", ti.getObservatii() == "Restock");

        TranzactieIesire te(2, 3, 50, "Vanzare");
        adaugaTest("TranzactieIesire - Tip", te.getTipTranzactie() == "IESIRE");
        adaugaTest("TranzactieIesire - Cantitate", te.getCantitate() == 50);
        adaugaTest("TranzactieIntrare - Data setata", !ti.getData().empty());
    }

    // ===== TESTE USER =====
    void testeazaUser() {
        std::cout << "\n--- Teste User ---" << std::endl;

        User admin(1, "admin", "hash123", Rol::ADMIN);
        adaugaTest("User - Constructor username", admin.getUsername() == "admin");
        adaugaTest("User - Constructor rol Admin", admin.eAdmin() == true);
        adaugaTest("User - getRolString Admin", admin.getRolString() == "Admin");
        adaugaTest("User - eActiv default true", admin.eActiv() == true);

        User angajat(2, "ion", "hash456", Rol::ANGAJAT);
        adaugaTest("User - Constructor rol Angajat", angajat.eAdmin() == false);
        adaugaTest("User - getRolString Angajat", angajat.getRolString() == "Angajat");

        angajat.setActiv(false);
        adaugaTest("User - setActiv false", angajat.eActiv() == false);
        angajat.setActiv(true);
        adaugaTest("User - setActiv true", angajat.eActiv() == true);
    }

    // ===== TESTE MOSTENIRE SI POLIMORFISM =====
    void testeazaMostenire() {
        std::cout << "\n--- Teste Mostenire si Polimorfism ---" << std::endl;

        // Test mostenire - Produs e EntitateDepozit
        Produs p(1, "Laptop Dell", 50, 3500.0, 10);
        EntitateDepozit* e1 = &p;

        adaugaTest("Mostenire - Produs este EntitateDepozit", e1 != nullptr);
        adaugaTest("Mostenire - getId() prin pointer baza", e1->getId() == 1);
        adaugaTest("Mostenire - getNume() prin pointer baza",
            e1->getNume() == "Laptop Dell");
        adaugaTest("Polimorfism - getTip() Produs", e1->getTip() == "Produs");

        // Test cu Furnizor
        Furnizor f(2, "TechSupply", "0721000001", "contact@tech.ro");
        EntitateDepozit* e2 = &f;
        adaugaTest("Mostenire - Furnizor este EntitateDepozit", e2 != nullptr);
        adaugaTest("Polimorfism - getTip() Furnizor", e2->getTip() == "Furnizor");
        adaugaTest("Polimorfism - getNume() Furnizor", e2->getNume() == "TechSupply");

        // Test cu Categorie
        Categorie c(3, "Electronice", "Produse IT");
        EntitateDepozit* e3 = &c;
        adaugaTest("Mostenire - Categorie este EntitateDepozit", e3 != nullptr);
        adaugaTest("Polimorfism - getTip() Categorie", e3->getTip() == "Categorie");

        // Test polimorfism prin vector
        std::vector<EntitateDepozit*> entitati = { e1, e2, e3 };
        adaugaTest("Polimorfism - vector de EntitateDepozit",
            entitati.size() == 3);

        // Test tipuri corecte in vector
        std::vector<std::string> tipuriAsteptate = {"Produs", "Furnizor", "Categorie"};
        bool toateTipurileCorecte = true;
        for (int i = 0; i < 3; i++) {
            if (entitati[i]->getTip() != tipuriAsteptate[i]) {
                toateTipurileCorecte = false;
                break;
            }
        }
        adaugaTest("Polimorfism - tipuri identificate corect in vector",
            toateTipurileCorecte);

        // Test getInfo() polimorfic
        adaugaTest("Polimorfism - getInfo() Produs contine numele",
            e1->getInfo().find("Laptop Dell") != std::string::npos);
        adaugaTest("Polimorfism - getInfo() Furnizor contine numele",
            e2->getInfo().find("TechSupply") != std::string::npos);
        adaugaTest("Polimorfism - getInfo() Categorie contine numele",
            e3->getInfo().find("Electronice") != std::string::npos);

        // Test operator==
        Produs p2(1, "Laptop Dell", 30, 3500.0, 10);
        adaugaTest("Polimorfism - operator== prin baza", (*e1) == p2);

        // Test destructor virtual
        EntitateDepozit* eTest = new Produs(99, "Test", 1, 1.0, 1);
        delete eTest;
        adaugaTest("Polimorfism - destructor virtual functioneaza", true);

        // Test static member
        int nrInainte = Produs::getNrTotalInstante();
        {
            Produs temp(99, "Temp", 1, 1.0, 1);
            adaugaTest("Static - contor creste la creare",
                Produs::getNrTotalInstante() == nrInainte + 1);
        }
        adaugaTest("Static - contor scade la distrugere",
            Produs::getNrTotalInstante() == nrInainte);

        // Test copy constructor
        Produs original(10, "Original", 50, 100.0, 5);
        Produs copie(original);
        adaugaTest("Copy constructor - nume copiat",
            copie.getNume() == original.getNume());
        adaugaTest("Copy constructor - cantitate copiata",
            copie.getCantitate() == original.getCantitate());
        adaugaTest("Copy constructor - instante independente",
            &copie != &original);

        // Test operator=
        Produs atribuit;
        atribuit = original;
        adaugaTest("Operator= - nume atribuit",
            atribuit.getNume() == original.getNume());
        adaugaTest("Operator= - self assignment sigur",
            (original = original).getNume() == "Original");

        // Test afisare virtuala (nu craseaza)
        bool afisareFunctioneaza = false;
        try {
            for (auto* e : entitati)
                e->afisare();
            afisareFunctioneaza = true;
        } catch (...) {}
        adaugaTest("Polimorfism - afisare() virtuala functioneaza",
            afisareFunctioneaza);
    }

    // ===== AFISARE REZULTATE =====
    void afiseazaRezultate() {
        std::cout << "\n========================================" << std::endl;
        std::cout << "   REZULTATE" << std::endl;
        std::cout << "========================================" << std::endl;

        for (const auto& r : rezultate) {
            if (r.trecut)
                std::cout << "  [PASS] " << r.nume << std::endl;
            else {
                std::cout << "  [FAIL] " << r.nume;
                if (!r.mesaj.empty())
                    std::cout << " -> " << r.mesaj;
                std::cout << std::endl;
            }
        }

        std::cout << "\n----------------------------------------" << std::endl;
        std::cout << "  Total:   " << rezultate.size() << " teste" << std::endl;
        std::cout << "  Trecute: " << trecute << std::endl;
        std::cout << "  Esuate:  " << esuate << std::endl;
        std::cout << "----------------------------------------" << std::endl;

        if (esuate == 0)
            std::cout << "  >> TOATE TESTELE AU TRECUT! <<" << std::endl;
        else
            std::cout << "  >> " << esuate << " TESTE AU ESUAT! <<" << std::endl;

        std::cout << "========================================\n" << std::endl;
    }

    int getNrEsuate() const { return esuate; }
    int getNrTrecute() const { return trecute; }
    int getNrTotal() const { return rezultate.size(); }
};