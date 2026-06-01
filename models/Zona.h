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
        return (float)capacitateCurenta / (float)capacitateMax * 100.0f;
    }

    // Culoare bazata pe procent - pentru ImGui
    ImVec4 getCuloare() const {
        float procent = getProcentOcupare();
        if (procent >= 61.0f)
            return ImVec4(0.9f, 0.2f, 0.2f, 1.0f); // Rosu
        else if (procent >= 21.0f)
            return ImVec4(0.9f, 0.6f, 0.1f, 1.0f); // Portocaliu
        else
            return ImVec4(0.2f, 0.8f, 0.2f, 1.0f); // Verde
    }

    bool eGoala() const { return capacitateCurenta == 0; }
    bool eePlina() const { return capacitateCurenta >= capacitateMax; }
};