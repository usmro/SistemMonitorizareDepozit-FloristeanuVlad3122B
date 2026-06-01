#pragma once
#include <string>
#include "imgui.h"

class Zona {
private:
    int id;
    char litera;
    std::string nume;
    int capacitateMax;
    int capacitateCurenta;

public:
    Zona() : id(0), litera('A'), capacitateMax(100), capacitateCurenta(0) {}
    Zona(int id, char litera, int capacitateMax = 100)
        : id(id), litera(litera), nume(std::string("Zona ") + litera),
          capacitateMax(capacitateMax), capacitateCurenta(0) {}

    int getId() const { return id; }
    char getLitera() const { return litera; }
    std::string getNume() const { return nume; }
    int getCapacitateMax() const { return capacitateMax; }
    int getCapacitateCurenta() const { return capacitateCurenta; }

    void setCapacitateCurenta(int c) { capacitateCurenta = c; }
    void adaugaCapacitate(int c) { capacitateCurenta += c; }
    void scadeCapacitate(int c) { capacitateCurenta -= c; }

    float getProcentOcupare() const {
        if (capacitateMax == 0) return 0.0f;
        float p = (float)capacitateCurenta / (float)capacitateMax * 100.0f;
        return p > 100.0f ? 100.0f : p;
    }

    ImVec4 getCuloare() const {
        float procent = getProcentOcupare();
        if (procent == 0.0f)
            return ImVec4(0.4f, 0.4f, 0.4f, 1.0f);   // Gri - Goala
        else if (procent <= 25.0f)
            return ImVec4(0.2f, 0.8f, 0.2f, 1.0f);   // Verde
        else if (procent <= 50.0f)
            return ImVec4(0.9f, 0.9f, 0.1f, 1.0f);   // Galben
        else if (procent <= 75.0f)
            return ImVec4(0.9f, 0.6f, 0.1f, 1.0f);   // Portocaliu
        else
            return ImVec4(0.9f, 0.2f, 0.2f, 1.0f);   // Rosu
    }

    ImVec4 getCuloareText() const {
        float procent = getProcentOcupare();
        if (procent == 0.0f)
            return ImVec4(0.8f, 0.8f, 0.8f, 1.0f);   // Gri deschis
        else if (procent <= 25.0f)
            return ImVec4(0.0f, 0.15f, 0.0f, 1.0f);  // Verde inchis
        else if (procent <= 50.0f)
            return ImVec4(0.15f, 0.10f, 0.0f, 1.0f); // Maro inchis pe galben
        else if (procent <= 75.0f)
            return ImVec4(0.15f, 0.08f, 0.0f, 1.0f); // Maro pe portocaliu
        else
            return ImVec4(1.0f, 0.9f, 0.9f, 1.0f);   // Alb pe rosu
    }

    bool eGoala() const { return capacitateCurenta == 0; }
    bool eePlina() const { return capacitateCurenta >= capacitateMax; }
};