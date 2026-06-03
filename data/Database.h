#pragma once
#include <string>
#include <vector>
#include "sqlite3.h"
#include "../models/Produs.h"
#include "../models/Categorie.h"
#include "../models/Furnizor.h"
#include "../models/Zona.h"
#include "../models/User.h"
#include "../data/sha256.h"

struct TranzactieRecord {
    int id;
    int produsId;
    std::string numeProdus;
    std::string tip;
    int cantitate;
    std::string data;
    std::string observatii;
};

class Database {
private:
    sqlite3* db;
    std::string dbPath;

    void executeSQL(const std::string& sql) {
        char* errMsg = nullptr;
        int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::string err = errMsg ? errMsg : "Unknown error";
            sqlite3_free(errMsg);
            throw std::runtime_error("SQL Error: " + err);
        }
    }

    void inserareProdus(const std::string& nume, int cantitate, double pret,
                        int pragAlerta, int zonaId, int catId, int furnId) {
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db,
            "INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES (?,?,?,?,?,?,?)",
            -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, nume.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, cantitate);
        sqlite3_bind_double(stmt, 3, pret);
        sqlite3_bind_int(stmt, 4, pragAlerta);
        sqlite3_bind_int(stmt, 5, zonaId);
        sqlite3_bind_int(stmt, 6, catId);
        sqlite3_bind_int(stmt, 7, furnId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

public:
    Database(const std::string& path = "depozit.db") : db(nullptr), dbPath(path) {}

    ~Database() {
        if (db) sqlite3_close(db);
    }

    bool conecteaza() {
        int rc = sqlite3_open(dbPath.c_str(), &db);
        if (rc != SQLITE_OK) return false;
        initTabele();
        initDateImplicite();
        return true;
    }

    void initTabele() {
        executeSQL(R"(CREATE TABLE IF NOT EXISTS categorii (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            nume TEXT NOT NULL, descriere TEXT);)");
        executeSQL(R"(CREATE TABLE IF NOT EXISTS furnizori (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            nume TEXT NOT NULL, telefon TEXT, email TEXT);)");
        executeSQL(R"(CREATE TABLE IF NOT EXISTS zone (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            litera TEXT NOT NULL,
            capacitate_max INTEGER DEFAULT 1000,
            capacitate_curenta INTEGER DEFAULT 0);)");
        executeSQL(R"(CREATE TABLE IF NOT EXISTS produse (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            nume TEXT NOT NULL,
            cantitate INTEGER DEFAULT 0,
            pret REAL DEFAULT 0.0,
            prag_alerta INTEGER DEFAULT 10,
            zona_id INTEGER DEFAULT 0,
            categorie_id INTEGER DEFAULT 0,
            furnizor_id INTEGER DEFAULT 0);)");
        executeSQL(R"(CREATE TABLE IF NOT EXISTS tranzactii (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            produs_id INTEGER, tip TEXT,
            cantitate INTEGER, data TEXT, observatii TEXT);)");
        executeSQL(R"(CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT UNIQUE NOT NULL,
            password_hash TEXT NOT NULL,
            rol TEXT DEFAULT 'ANGAJAT',
            activ INTEGER DEFAULT 1);)");

        executeSQL("CREATE INDEX IF NOT EXISTS idx_produse_zona ON produse(zona_id);");
        executeSQL("CREATE INDEX IF NOT EXISTS idx_produse_nume ON produse(nume);");
        executeSQL("CREATE INDEX IF NOT EXISTS idx_produse_prag ON produse(cantitate, prag_alerta);");
        executeSQL("CREATE INDEX IF NOT EXISTS idx_tranzactii_produs ON tranzactii(produs_id);");
        executeSQL("CREATE INDEX IF NOT EXISTS idx_tranzactii_data ON tranzactii(data);");
        executeSQL("CREATE INDEX IF NOT EXISTS idx_users_username ON users(username);");
    }

    void initDateImplicite() {
        sqlite3_stmt* stmt;

        // ===== ADMIN =====
        sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM users WHERE username = 'admin'", -1, &stmt, nullptr);
        sqlite3_step(stmt);
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        if (count == 0) {
            std::string hash = SHA256::hash("admin123");
            executeSQL("INSERT INTO users (username, password_hash, rol) VALUES ('admin', '" + hash + "', 'ADMIN')");
        }

        // ===== ZONE =====
        sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM zone", -1, &stmt, nullptr);
        sqlite3_step(stmt);
        count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        if (count == 0) {
            for (int i = 0; i < 16; i++) {
                char litera = 'A' + i;
                executeSQL("INSERT INTO zone (litera, capacitate_max, capacitate_curenta) VALUES ('" +
                    std::string(1, litera) + "', 1000, 0)");
            }
        }

        // ===== CATEGORII =====
        sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM categorii", -1, &stmt, nullptr);
        sqlite3_step(stmt);
        count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        if (count == 0) {
            executeSQL("INSERT INTO categorii (nume, descriere) VALUES ('Electronice', 'Laptopuri, telefoane, accesorii IT')");
            executeSQL("INSERT INTO categorii (nume, descriere) VALUES ('Alimente', 'Produse alimentare si bauturi')");
            executeSQL("INSERT INTO categorii (nume, descriere) VALUES ('Cosmetice', 'Produse cosmetice si ingrijire personala')");
            executeSQL("INSERT INTO categorii (nume, descriere) VALUES ('Mobilier', 'Mobila, decoratiuni si articole casa')");
            executeSQL("INSERT INTO categorii (nume, descriere) VALUES ('Componente PC', 'Componente si periferice calculator')");
        }

        // ===== FURNIZORI =====
        sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM furnizori", -1, &stmt, nullptr);
        sqlite3_step(stmt);
        count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        if (count == 0) {
            executeSQL("INSERT INTO furnizori (nume, telefon, email) VALUES ('TechSupply SRL', '0721-100-001', 'comenzi@techsupply.ro')");
            executeSQL("INSERT INTO furnizori (nume, telefon, email) VALUES ('FoodDist SA', '0721-100-002', 'office@fooddist.ro')");
            executeSQL("INSERT INTO furnizori (nume, telefon, email) VALUES ('BeautyPro Distribution', '0721-100-003', 'info@beautypro.ro')");
            executeSQL("INSERT INTO furnizori (nume, telefon, email) VALUES ('Casa & Design SRL', '0721-100-004', 'contact@casadesign.ro')");
            executeSQL("INSERT INTO furnizori (nume, telefon, email) VALUES ('PCComponents Europe', '0721-100-005', 'sales@pccomponents.eu')");
        }

        // ===== PRODUSE =====
        sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM produse", -1, &stmt, nullptr);
        sqlite3_step(stmt);
        count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        if (count == 0) {
            generateazaProduse();
        }
    }

    void generateazaProduse() {
        executeSQL("BEGIN TRANSACTION;");

        // ===== DEFINITII PRODUSE =====

        // Laptopuri - brand x model x ram
        std::vector<std::string> brandLaptop = {"Dell", "Lenovo", "HP", "Asus", "Acer", "MSI", "Apple"};
        std::vector<std::string> modelLaptop = {"Inspiron", "ThinkPad", "EliteBook", "VivoBook", "Aspire", "Stealth", "MacBook Air"};
        std::vector<std::string> ramLaptop = {"8GB", "16GB", "32GB"};
        std::vector<double> pretLaptop = {2800, 3500, 4200, 5500, 7200};
        int zonaLaptop = 1; // Zona A - 100%
        for (int b = 0; b < (int)brandLaptop.size(); b++) {
            for (int m = 0; m < (int)modelLaptop.size(); m++) {
                for (int r = 0; r < (int)ramLaptop.size(); r++) {
                    std::string nume = brandLaptop[b] + " " + modelLaptop[m] + " " + ramLaptop[r];
                    double pret = pretLaptop[(b + m) % pretLaptop.size()];
                    int cant = 2 + (b + m + r) % 4;
                    inserareProdus(nume, cant, pret, 3, zonaLaptop, 1, 1);
                }
            }
        }

        // Monitoare - brand x diagonala x rezolutie
        std::vector<std::string> brandMonitor = {"Samsung", "LG", "Asus", "Dell", "Philips", "AOC"};
        std::vector<std::string> diagMonitor = {"24 inch", "27 inch", "32 inch", "34 inch"};
        std::vector<std::string> rezMonitor = {"Full HD", "4K", "QHD"};
        std::vector<double> pretMonitor = {800, 1200, 1800, 2400, 3200};
        int zonaMonitor = 2; // Zona B - 80%
        for (int b = 0; b < (int)brandMonitor.size(); b++) {
            for (int d = 0; d < (int)diagMonitor.size(); d++) {
                for (int r = 0; r < (int)rezMonitor.size(); r++) {
                    std::string nume = "Monitor " + brandMonitor[b] + " " + diagMonitor[d] + " " + rezMonitor[r];
                    double pret = pretMonitor[(b + d + r) % pretMonitor.size()];
                    int cant = 3 + (b + d) % 5;
                    inserareProdus(nume, cant, pret, 3, zonaMonitor, 1, 1);
                }
            }
        }

        // Telefoane - brand x model x stocare
        std::vector<std::string> brandTelefon = {"Samsung", "Apple", "Xiaomi", "OnePlus", "Huawei", "Google"};
        std::vector<std::string> modelTelefon = {"Galaxy S24", "iPhone 15", "Redmi Note 13", "12 Pro", "P60 Pro", "Pixel 8"};
        std::vector<std::string> stocareTelefon = {"128GB", "256GB", "512GB"};
        std::vector<double> pretTelefon = {1200, 2500, 4500, 6500, 8000};
        int zonaTelefon = 3; // Zona C - 60%
        for (int b = 0; b < (int)brandTelefon.size(); b++) {
            for (int s = 0; s < (int)stocareTelefon.size(); s++) {
                std::string nume = brandTelefon[b] + " " + modelTelefon[b] + " " + stocareTelefon[s];
                double pret = pretTelefon[(b + s) % pretTelefon.size()];
                int cant = 5 + (b + s) % 8;
                inserareProdus(nume, cant, pret, 5, zonaTelefon, 1, 1);
            }
        }

        // Tablete - brand x model x stocare
        std::vector<std::string> brandTablete = {"Samsung", "Apple", "Lenovo", "Huawei", "Microsoft"};
        std::vector<std::string> modelTablete = {"Galaxy Tab S9", "iPad Pro", "Tab P12", "MatePad Pro", "Surface Pro"};
        std::vector<std::string> stocareTablete = {"64GB", "128GB", "256GB"};
        std::vector<double> pretTablete = {1500, 2500, 3500, 4500};
        for (int b = 0; b < (int)brandTablete.size(); b++) {
            for (int s = 0; s < (int)stocareTablete.size(); s++) {
                std::string nume = brandTablete[b] + " " + modelTablete[b] + " " + stocareTablete[s];
                double pret = pretTablete[(b + s) % pretTablete.size()];
                int cant = 4 + (b + s) % 6;
                inserareProdus(nume, cant, pret, 3, zonaTelefon, 1, 1);
            }
        }

        // Cafea - brand x gramaj x tip
        std::vector<std::string> brandCafea = {"Jacobs", "Lavazza", "Illy", "Nescafe", "Doncafe", "Julius Meinl"};
        std::vector<std::string> gramajCafea = {"100g", "250g", "500g", "1kg"};
        std::vector<std::string> tipCafea = {"Boabe", "Macinata", "Instant"};
        std::vector<double> pretCafea = {15, 28, 45, 72, 95};
        int zonaCafea = 4; // Zona D - 50%
        for (int b = 0; b < (int)brandCafea.size(); b++) {
            for (int g = 0; g < (int)gramajCafea.size(); g++) {
                for (int t = 0; t < (int)tipCafea.size(); t++) {
                    std::string nume = "Cafea " + brandCafea[b] + " " + gramajCafea[g] + " " + tipCafea[t];
                    double pret = pretCafea[(b + g) % pretCafea.size()];
                    int cant = 3 + (b + g + t) % 6;
                    inserareProdus(nume, cant, pret, 5, zonaCafea, 2, 2);
                }
            }
        }

        // Uleiuri - brand x tip x volum
        std::vector<std::string> brandUlei = {"Bunica", "Floriol", "Unisol", "Argus", "Mester"};
        std::vector<std::string> tipUlei = {"Floarea Soarelui", "Masline Extra Virgin", "Rapita", "Cocos"};
        std::vector<std::string> volumUlei = {"500ml", "1L", "2L", "5L"};
        std::vector<double> pretUlei = {8, 12, 22, 48, 65};
        int zonaUlei = 4; // Zona D
        for (int b = 0; b < (int)brandUlei.size(); b++) {
            for (int t = 0; t < (int)tipUlei.size(); t++) {
                for (int v = 0; v < (int)volumUlei.size(); v++) {
                    std::string nume = "Ulei " + brandUlei[b] + " " + tipUlei[t] + " " + volumUlei[v];
                    double pret = pretUlei[(b + t + v) % pretUlei.size()];
                    int cant = 5 + (b + t + v) % 10;
                    inserareProdus(nume, cant, pret, 8, zonaUlei, 2, 2);
                }
            }
        }

        // Paste fainoase - brand x tip x gramaj
        std::vector<std::string> brandPaste = {"Barilla", "La Molisana", "Divella", "Panzani", "Baneasa"};
        std::vector<std::string> tipPaste = {"Spaghetti", "Penne", "Fusilli", "Farfalle", "Tagliatelle", "Rigatoni"};
        std::vector<std::string> gramajPaste = {"400g", "500g", "1kg"};
        std::vector<double> pretPaste = {5, 8, 12, 18};
        int zonaPaste = 5; // Zona E - 40%
        for (int b = 0; b < (int)brandPaste.size(); b++) {
            for (int t = 0; t < (int)tipPaste.size(); t++) {
                for (int g = 0; g < (int)gramajPaste.size(); g++) {
                    std::string nume = brandPaste[b] + " " + tipPaste[t] + " " + gramajPaste[g];
                    double pret = pretPaste[(b + t + g) % pretPaste.size()];
                    int cant = 4 + (b + t + g) % 8;
                    inserareProdus(nume, cant, pret, 10, zonaPaste, 2, 2);
                }
            }
        }

        // Orez - brand x tip x gramaj
        std::vector<std::string> brandOrez = {"Uncle Bens", "Baneasa", "SeleRiso", "Risella", "Carmencita"};
        std::vector<std::string> tipOrez = {"Basmati", "Jasmine", "Arborio", "Negru", "Brun"};
        std::vector<std::string> gramajOrez = {"500g", "1kg", "2kg", "5kg"};
        std::vector<double> pretOrez = {6, 10, 18, 28};
        for (int b = 0; b < (int)brandOrez.size(); b++) {
            for (int t = 0; t < (int)tipOrez.size(); t++) {
                for (int g = 0; g < (int)gramajOrez.size(); g++) {
                    std::string nume = "Orez " + brandOrez[b] + " " + tipOrez[t] + " " + gramajOrez[g];
                    double pret = pretOrez[(b + t + g) % pretOrez.size()];
                    int cant = 3 + (b + t + g) % 7;
                    inserareProdus(nume, cant, pret, 8, zonaPaste, 2, 2);
                }
            }
        }

        // Sampon - brand x tip x volum
        std::vector<std::string> brandSampon = {"Pantene", "Head & Shoulders", "Dove", "Garnier", "L Oreal", "Elvive"};
        std::vector<std::string> tipSampon = {"Par Normal", "Par Gras", "Par Uscat", "Anti-Matreata", "Reparator"};
        std::vector<std::string> volumSampon = {"200ml", "400ml", "700ml"};
        std::vector<double> pretSampon = {14, 22, 35, 48};
        int zonaSampon = 6; // Zona F - 35%
        for (int b = 0; b < (int)brandSampon.size(); b++) {
            for (int t = 0; t < (int)tipSampon.size(); t++) {
                for (int v = 0; v < (int)volumSampon.size(); v++) {
                    std::string nume = "Sampon " + brandSampon[b] + " " + tipSampon[t] + " " + volumSampon[v];
                    double pret = pretSampon[(b + t + v) % pretSampon.size()];
                    int cant = 2 + (b + t + v) % 5;
                    inserareProdus(nume, cant, pret, 5, zonaSampon, 3, 3);
                }
            }
        }

        // Creme - brand x tip x gramaj
        std::vector<std::string> brandCrema = {"Nivea", "Garnier", "Dove", "L Oreal", "Neutrogena"};
        std::vector<std::string> tipCrema = {"Hidratanta", "Anti-Rid", "Nutritiva", "SPF 50", "Noapte"};
        std::vector<std::string> gramajCrema = {"50ml", "100ml", "200ml"};
        std::vector<double> pretCrema = {18, 28, 45, 65, 85};
        for (int b = 0; b < (int)brandCrema.size(); b++) {
            for (int t = 0; t < (int)tipCrema.size(); t++) {
                for (int g = 0; g < (int)gramajCrema.size(); g++) {
                    std::string nume = "Crema " + brandCrema[b] + " " + tipCrema[t] + " " + gramajCrema[g];
                    double pret = pretCrema[(b + t) % pretCrema.size()];
                    int cant = 2 + (b + t + g) % 6;
                    inserareProdus(nume, cant, pret, 5, zonaSampon, 3, 3);
                }
            }
        }

        // Parfumuri - brand x model x ml
        std::vector<std::string> brandParfum = {"Chanel", "Dior", "Hugo Boss", "Armani", "Paco Rabanne", "Calvin Klein"};
        std::vector<std::string> modelParfum = {"No 5", "Sauvage", "Bottled", "Acqua di Gio", "Invictus", "Eternity"};
        std::vector<std::string> mlParfum = {"30ml", "50ml", "100ml"};
        std::vector<double> pretParfum = {180, 280, 420, 580, 750};
        int zonaParfum = 7; // Zona G - 25%
        for (int b = 0; b < (int)brandParfum.size(); b++) {
            for (int v = 0; v < (int)mlParfum.size(); v++) {
                std::string nume = "Parfum " + brandParfum[b] + " " + modelParfum[b] + " " + mlParfum[v];
                double pret = pretParfum[(b + v) % pretParfum.size()];
                int cant = 2 + (b + v) % 4;
                inserareProdus(nume, cant, pret, 3, zonaParfum, 3, 3);
            }
        }

        // Deodorante - brand x tip x volum
        std::vector<std::string> brandDeo = {"Rexona", "Nivea", "Dove", "Axe", "Old Spice", "Gillette"};
        std::vector<std::string> tipDeo = {"Men", "Women", "Sensitive", "Sport", "Fresh"};
        std::vector<std::string> volumDeo = {"150ml", "250ml"};
        std::vector<double> pretDeo = {12, 18, 24, 32};
        for (int b = 0; b < (int)brandDeo.size(); b++) {
            for (int t = 0; t < (int)tipDeo.size(); t++) {
                for (int v = 0; v < (int)volumDeo.size(); v++) {
                    std::string nume = "Deodorant " + brandDeo[b] + " " + tipDeo[t] + " " + volumDeo[v];
                    double pret = pretDeo[(b + t + v) % pretDeo.size()];
                    int cant = 3 + (b + t + v) % 6;
                    inserareProdus(nume, cant, pret, 5, zonaParfum, 3, 3);
                }
            }
        }

        // Scaune - brand x tip x culoare
        std::vector<std::string> brandScaun = {"DXRacer", "Noblechairs", "Secretlab", "IKEA", "Hm"};
        std::vector<std::string> tipScaun = {"Gaming", "Office", "Executive", "Ergonomic"};
        std::vector<std::string> culoareScaun = {"Negru", "Alb", "Rosu", "Albastru"};
        std::vector<double> pretScaun = {450, 800, 1200, 1800, 2500};
        int zonaScaun = 8; // Zona H - 20%
        for (int b = 0; b < (int)brandScaun.size(); b++) {
            for (int t = 0; t < (int)tipScaun.size(); t++) {
                for (int c = 0; c < (int)culoareScaun.size(); c++) {
                    std::string nume = "Scaun " + brandScaun[b] + " " + tipScaun[t] + " " + culoareScaun[c];
                    double pret = pretScaun[(b + t + c) % pretScaun.size()];
                    int cant = 1 + (b + t + c) % 3;
                    inserareProdus(nume, cant, pret, 2, zonaScaun, 4, 4);
                }
            }
        }

        // Birouri - brand x dimensiune x material
        std::vector<std::string> brandBirou = {"IKEA", "Samas", "Steelcase", "Herman Miller"};
        std::vector<std::string> dimBirou = {"120x60cm", "140x70cm", "160x80cm", "180x90cm"};
        std::vector<std::string> matBirou = {"Lemn", "Metal", "Sticla", "MDF"};
        std::vector<double> pretBirou = {450, 800, 1400, 2200, 3500};
        for (int b = 0; b < (int)brandBirou.size(); b++) {
            for (int d = 0; d < (int)dimBirou.size(); d++) {
                for (int m = 0; m < (int)matBirou.size(); m++) {
                    std::string nume = "Birou " + brandBirou[b] + " " + dimBirou[d] + " " + matBirou[m];
                    double pret = pretBirou[(b + d + m) % pretBirou.size()];
                    int cant = 1 + (b + d + m) % 3;
                    inserareProdus(nume, cant, pret, 2, zonaScaun, 4, 4);
                }
            }
        }

        // Routere - brand x standard x viteza
        std::vector<std::string> brandRouter = {"Asus", "TP-Link", "Netgear", "Dlink", "Ubiquiti"};
        std::vector<std::string> standardRouter = {"WiFi 5", "WiFi 6", "WiFi 6E"};
        std::vector<std::string> vitezaRouter = {"AC1200", "AC2400", "AX3000", "AX6000"};
        std::vector<double> pretRouter = {120, 250, 450, 750, 1200};
        int zonaRetea = 9; // Zona I - 15%
        for (int b = 0; b < (int)brandRouter.size(); b++) {
            for (int s = 0; s < (int)standardRouter.size(); s++) {
                for (int v = 0; v < (int)vitezaRouter.size(); v++) {
                    std::string nume = "Router " + brandRouter[b] + " " + standardRouter[s] + " " + vitezaRouter[v];
                    double pret = pretRouter[(b + s + v) % pretRouter.size()];
                    int cant = 2 + (b + s + v) % 5;
                    inserareProdus(nume, cant, pret, 3, zonaRetea, 5, 5);
                }
            }
        }

        // SSD - brand x capacitate x interfata
        std::vector<std::string> brandSSD = {"Samsung", "Kingston", "WD", "Seagate", "Crucial"};
        std::vector<std::string> capacitateSSD = {"256GB", "512GB", "1TB", "2TB", "4TB"};
        std::vector<std::string> interfataSSD = {"SATA", "NVMe M.2", "PCIe 4.0"};
        std::vector<double> pretSSD = {150, 280, 450, 750, 1200};
        int zonaSSD = 10; // Zona J - 10%
        for (int b = 0; b < (int)brandSSD.size(); b++) {
            for (int c = 0; c < (int)capacitateSSD.size(); c++) {
                for (int i = 0; i < (int)interfataSSD.size(); i++) {
                    std::string nume = "SSD " + brandSSD[b] + " " + capacitateSSD[c] + " " + interfataSSD[i];
                    double pret = pretSSD[(b + c + i) % pretSSD.size()];
                    int cant = 1 + (b + c + i) % 4;
                    inserareProdus(nume, cant, pret, 3, zonaSSD, 5, 5);
                }
            }
        }

        // RAM - brand x capacitate x frecventa
        std::vector<std::string> brandRAM = {"Corsair", "Kingston", "G.Skill", "Crucial"};
        std::vector<std::string> capacitateRAM = {"8GB", "16GB", "32GB", "64GB"};
        std::vector<std::string> frecventaRAM = {"DDR4 3200", "DDR4 3600", "DDR5 5200", "DDR5 6000"};
        std::vector<double> pretRAM = {120, 220, 380, 650};
        int zonaRAM = 11; // Zona K - 8%
        for (int b = 0; b < (int)brandRAM.size(); b++) {
            for (int c = 0; c < (int)capacitateRAM.size(); c++) {
                for (int f = 0; f < (int)frecventaRAM.size(); f++) {
                    std::string nume = "RAM " + brandRAM[b] + " " + capacitateRAM[c] + " " + frecventaRAM[f];
                    double pret = pretRAM[(b + c + f) % pretRAM.size()];
                    int cant = 2 + (b + c + f) % 4;
                    inserareProdus(nume, cant, pret, 3, zonaRAM, 5, 5);
                }
            }
        }

        // Placi video - brand x model x VRAM
        std::vector<std::string> brandGPU = {"Nvidia", "AMD", "Asus", "MSI", "Gigabyte"};
        std::vector<std::string> modelGPU = {"RTX 4060", "RTX 4070", "RX 7600", "RX 7800", "RTX 4080"};
        std::vector<std::string> vramGPU = {"8GB", "12GB", "16GB"};
        std::vector<double> pretGPU = {1200, 2200, 3500, 4800};
        int zonaGPU = 12; // Zona L - 5%
        for (int b = 0; b < (int)brandGPU.size(); b++) {
            for (int v = 0; v < (int)vramGPU.size(); v++) {
                std::string nume = "Placa Video " + brandGPU[b] + " " + modelGPU[b % modelGPU.size()] + " " + vramGPU[v];
                double pret = pretGPU[(b + v) % pretGPU.size()];
                int cant = 1 + (b + v) % 3;
                inserareProdus(nume, cant, pret, 2, zonaGPU, 5, 5);
            }
        }

        // Procesoare - brand x model x frecventa
        std::vector<std::string> brandCPU = {"Intel", "AMD"};
        std::vector<std::string> modelIntel = {"Core i5-13400", "Core i5-13600K", "Core i7-13700K", "Core i9-13900K", "Core i5-14400", "Core i7-14700K"};
        std::vector<std::string> modelAMD = {"Ryzen 5 7600", "Ryzen 5 7600X", "Ryzen 7 7700X", "Ryzen 9 7900X", "Ryzen 5 8600X", "Ryzen 7 9700X"};
        std::vector<double> pretCPU = {850, 1200, 1800, 2800, 3500};
        int zonaCPU = 13; // Zona M - 3%
        for (int m = 0; m < (int)modelIntel.size(); m++) {
            int cant = 1 + m % 2;
            inserareProdus("Procesor " + modelIntel[m], cant,
                pretCPU[m % pretCPU.size()], 2, zonaCPU, 5, 5);
        }
        for (int m = 0; m < (int)modelAMD.size(); m++) {
            int cant = 1 + m % 2;
            inserareProdus("Procesor " + modelAMD[m], cant,
                pretCPU[m % pretCPU.size()], 2, zonaCPU, 5, 5);
        }

        // Zone N, O, P - goale (rezervate)

        executeSQL("COMMIT;");
    }

    // ===== AUTH =====
    bool autentifica(const std::string& username, const std::string& parola, User& userOut) {
        std::string hash = SHA256::hash(parola);
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db,
            "SELECT id, username, password_hash, rol, activ FROM users WHERE username = ? AND password_hash = ? AND activ = 1",
            -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hash.c_str(), -1, SQLITE_STATIC);

        bool succes = false;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            std::string user = (const char*)sqlite3_column_text(stmt, 1);
            std::string rol = (const char*)sqlite3_column_text(stmt, 3);
            Rol r = (rol == "ADMIN") ? Rol::ADMIN : Rol::ANGAJAT;
            userOut = User(id, user, hash, r);
            succes = true;
        }
        sqlite3_finalize(stmt);
        return succes;
    }

    // ===== USERS =====
    std::vector<User> getUsers() {
        std::vector<User> users;
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db,
            "SELECT id, username, password_hash, rol, activ FROM users",
            -1, &stmt, nullptr);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            std::string username = (const char*)sqlite3_column_text(stmt, 1);
            std::string hash = (const char*)sqlite3_column_text(stmt, 2);
            std::string rol = (const char*)sqlite3_column_text(stmt, 3);
            bool activ = sqlite3_column_int(stmt, 4) == 1;
            Rol r = (rol == "ADMIN") ? Rol::ADMIN : Rol::ANGAJAT;
            User u(id, username, hash, r);
            u.setActiv(activ);
            users.push_back(u);
        }
        sqlite3_finalize(stmt);
        return users;
    }

    void adaugaUser(const std::string& username, const std::string& parola, Rol rol) {
        std::string hash = SHA256::hash(parola);
        std::string rolStr = (rol == Rol::ADMIN) ? "ADMIN" : "ANGAJAT";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db,
            "INSERT INTO users (username, password_hash, rol) VALUES (?,?,?)",
            -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hash.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, rolStr.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void dezactiveazaUser(int id) {
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "UPDATE users SET activ = 0 WHERE id = ?", -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void activeazaUser(int id) {
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "UPDATE users SET activ = 1 WHERE id = ?", -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    // ===== PRODUSE =====
    std::vector<Produs> getProduse(int limit = -1, int offset = 0) {
        std::vector<Produs> produse;
        sqlite3_stmt* stmt;
        std::string sql = "SELECT id, nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id FROM produse";
        if (limit > 0)
            sql += " LIMIT " + std::to_string(limit) + " OFFSET " + std::to_string(offset);
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            produse.emplace_back(
                sqlite3_column_int(stmt, 0),
                (const char*)sqlite3_column_text(stmt, 1),
                sqlite3_column_int(stmt, 2),
                sqlite3_column_double(stmt, 3),
                sqlite3_column_int(stmt, 4),
                sqlite3_column_int(stmt, 5),
                sqlite3_column_int(stmt, 6),
                sqlite3_column_int(stmt, 7)
            );
        }
        sqlite3_finalize(stmt);
        return produse;
    }

    int getNrTotalProduse() {
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM produse", -1, &stmt, nullptr);
        sqlite3_step(stmt);
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        return count;
    }

    void adaugaProdus(const Produs& p) {
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db,
            "INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES (?,?,?,?,?,?,?)",
            -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, p.getNume().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, p.getCantitate());
        sqlite3_bind_double(stmt, 3, p.getPret());
        sqlite3_bind_int(stmt, 4, p.getPragAlerta());
        sqlite3_bind_int(stmt, 5, p.getZonaId());
        sqlite3_bind_int(stmt, 6, p.getCategorieId());
        sqlite3_bind_int(stmt, 7, p.getFurnizorId());
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void updateCantitate(int produsId, int cantitateNoua) {
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "UPDATE produse SET cantitate = ? WHERE id = ?", -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, cantitateNoua);
        sqlite3_bind_int(stmt, 2, produsId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void updateProdus(int id, const std::string& nume, double pret, int pragAlerta, int zonaId) {
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db,
            "UPDATE produse SET nume=?, pret=?, prag_alerta=?, zona_id=? WHERE id=?",
            -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, nume.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 2, pret);
        sqlite3_bind_int(stmt, 3, pragAlerta);
        sqlite3_bind_int(stmt, 4, zonaId);
        sqlite3_bind_int(stmt, 5, id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void eliminaProdus(int id) {
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "DELETE FROM produse WHERE id = ?", -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    // ===== ZONE =====
    std::vector<Zona> getZone() {
        std::vector<Zona> zone;
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db,
            "SELECT id, litera, capacitate_max, capacitate_curenta FROM zone ORDER BY litera",
            -1, &stmt, nullptr);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            char litera = ((const char*)sqlite3_column_text(stmt, 1))[0];
            int capMax = sqlite3_column_int(stmt, 2);
            int capCur = sqlite3_column_int(stmt, 3);
            Zona z(id, litera, capMax);
            z.setCapacitateCurenta(capCur);
            zone.push_back(z);
        }
        sqlite3_finalize(stmt);
        return zone;
    }

    void updateZona(int zonaId, int capacitateCurenta) {
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "UPDATE zone SET capacitate_curenta = ? WHERE id = ?", -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, capacitateCurenta);
        sqlite3_bind_int(stmt, 2, zonaId);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void recalculeazaZone() {
        executeSQL("UPDATE zone SET capacitate_curenta = 0");
        executeSQL(R"(
            UPDATE zone SET capacitate_curenta = (
                SELECT COALESCE(SUM(cantitate), 0)
                FROM produse WHERE produse.zona_id = zone.id
            )
        )");
    }

    // ===== CATEGORII =====
    std::vector<Categorie> getCategorii() {
        std::vector<Categorie> categorii;
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "SELECT id, nume, descriere FROM categorii", -1, &stmt, nullptr);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            categorii.emplace_back(
                sqlite3_column_int(stmt, 0),
                (const char*)sqlite3_column_text(stmt, 1),
                sqlite3_column_text(stmt, 2) ? (const char*)sqlite3_column_text(stmt, 2) : ""
            );
        }
        sqlite3_finalize(stmt);
        return categorii;
    }

    void adaugaCategorie(const Categorie& c) {
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "INSERT INTO categorii (nume, descriere) VALUES (?,?)", -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, c.getNume().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, c.getDescriere().c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    // ===== FURNIZORI =====
    std::vector<Furnizor> getFurnizori() {
        std::vector<Furnizor> furnizori;
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "SELECT id, nume, telefon, email FROM furnizori", -1, &stmt, nullptr);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            furnizori.emplace_back(
                sqlite3_column_int(stmt, 0),
                (const char*)sqlite3_column_text(stmt, 1),
                sqlite3_column_text(stmt, 2) ? (const char*)sqlite3_column_text(stmt, 2) : "",
                sqlite3_column_text(stmt, 3) ? (const char*)sqlite3_column_text(stmt, 3) : ""
            );
        }
        sqlite3_finalize(stmt);
        return furnizori;
    }

    void adaugaFurnizor(const Furnizor& f) {
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, "INSERT INTO furnizori (nume, telefon, email) VALUES (?,?,?)", -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, f.getNume().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, f.getTelefon().c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, f.getEmail().c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    // ===== TRANZACTII =====
    void adaugaTranzactie(int produsId, const std::string& tip, int cantitate,
                          const std::string& observatii = "") {
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db,
            "INSERT INTO tranzactii (produs_id, tip, cantitate, data, observatii) VALUES (?,?,?,datetime('now'),?)",
            -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, produsId);
        sqlite3_bind_text(stmt, 2, tip.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, cantitate);
        sqlite3_bind_text(stmt, 4, observatii.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    std::vector<TranzactieRecord> getTranzactii(int limit = 50) {
        std::vector<TranzactieRecord> rezultat;
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db,
            R"(SELECT t.id, t.produs_id, COALESCE(p.nume, 'Sters'),
               t.tip, t.cantitate, t.data, t.observatii
               FROM tranzactii t
               LEFT JOIN produse p ON t.produs_id = p.id
               ORDER BY t.id DESC LIMIT ?)",
            -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, limit);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            TranzactieRecord r;
            r.id = sqlite3_column_int(stmt, 0);
            r.produsId = sqlite3_column_int(stmt, 1);
            r.numeProdus = (const char*)sqlite3_column_text(stmt, 2);
            r.tip = (const char*)sqlite3_column_text(stmt, 3);
            r.cantitate = sqlite3_column_int(stmt, 4);
            r.data = sqlite3_column_text(stmt, 5) ?
                (const char*)sqlite3_column_text(stmt, 5) : "";
            r.observatii = sqlite3_column_text(stmt, 6) ?
                (const char*)sqlite3_column_text(stmt, 6) : "";
            rezultat.push_back(r);
        }
        sqlite3_finalize(stmt);
        return rezultat;
    }
};