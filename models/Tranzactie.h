#pragma once
#include <string>
#include <chrono>
#include <ctime>

// Tipuri de tranzactie
struct Intrare {};
struct Iesire {};

template<typename T>
class Tranzactie {
private:
    int id;
    int produsId;
    int cantitate;
    std::string data;
    std::string observatii;

    std::string getTip() const {
        if constexpr (std::is_same_v<T, Intrare>)
            return "INTRARE";
        else
            return "IESIRE";
    }

    static std::string getDataCurenta() {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        char buf[20];
        struct tm timeinfo;
        localtime_s(&timeinfo, &t);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &timeinfo);
        return std::string(buf);
    }

public:
    Tranzactie() : id(0), produsId(0), cantitate(0) {
        data = getDataCurenta();
    }

    Tranzactie(int id, int produsId, int cantitate, std::string observatii = "")
        : id(id), produsId(produsId), cantitate(cantitate), observatii(observatii) {
        data = getDataCurenta();
    }

    int getId() const { return id; }
    int getProdusId() const { return produsId; }
    int getCantitate() const { return cantitate; }
    std::string getData() const { return data; }
    std::string getObservatii() const { return observatii; }
    std::string getTipTranzactie() const { return getTip(); }

    void setObservatii(std::string o) { observatii = o; }
};

// Alias-uri pentru folosire usoara
using TranzactieIntrare = Tranzactie<Intrare>;
using TranzactieIesire = Tranzactie<Iesire>;