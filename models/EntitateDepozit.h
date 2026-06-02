#pragma once
#include <string>
#include <iostream>

class EntitateDepozit {
protected:
    int id;
    std::string nume;

public:
    EntitateDepozit() : id(0), nume("") {}
    EntitateDepozit(int id, std::string nume) : id(id), nume(nume) {}

    // Getteri de baza
    int getId() const { return id; }
    std::string getNume() const { return nume; }
    void setNume(const std::string& n) { nume = n; }

    // Functii virtuale pure - obliga clasele derivate sa le implementeze
    virtual void afisare() const = 0;
    virtual std::string getTip() const = 0;
    virtual std::string getInfo() const = 0;

    // Destructor virtual - obligatoriu pentru polimorfism corect
    virtual ~EntitateDepozit() = default;

    // Operator de comparatie virtual
    virtual bool operator==(const EntitateDepozit& other) const {
        return id == other.id && nume == other.nume;
    }

    // Operator de afisare
    friend std::ostream& operator<<(std::ostream& os, const EntitateDepozit& e) {
        os << e.getTip() << " [" << e.id << "]: " << e.nume;
        return os;
    }
};