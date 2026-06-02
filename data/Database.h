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

            // ===== ZONA A - 100% PLINA =====
            // Total: 1000 unitati - demonstreaza sugestia de zona alternativa
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Laptop Dell Inspiron 15', 200, 3800.00, 20, 1, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Laptop Lenovo ThinkPad E15', 150, 4200.00, 15, 1, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Laptop HP EliteBook 840', 150, 5500.00, 10, 1, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Laptop Asus VivoBook 15', 200, 3200.00, 25, 1, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Laptop MacBook Air M2', 100, 7500.00, 10, 1, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Laptop Acer Aspire 5', 200, 2900.00, 20, 1, 1, 1)");

            // ===== ZONA B - ~80% (Rosu) =====
            // Total: 800 unitati
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Monitor Samsung 27 inch 4K', 120, 1800.00, 10, 2, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Monitor LG UltraWide 34', 80, 2400.00, 8, 2, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Monitor Asus ProArt 32', 100, 3200.00, 8, 2, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Tastatura Mecanica Corsair K70', 150, 650.00, 20, 2, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Mouse Logitech MX Master 3', 200, 420.00, 30, 2, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Webcam Logitech C920 HD', 150, 380.00, 25, 2, 1, 1)");

            // ===== ZONA C - ~60% (Portocaliu) =====
            // Total: 600 unitati
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Telefon Samsung Galaxy S24', 100, 4500.00, 10, 3, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Telefon iPhone 15 Pro', 80, 7200.00, 8, 3, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Telefon Xiaomi Redmi Note 13', 150, 1200.00, 20, 3, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Tableta Samsung Galaxy Tab S9', 70, 3200.00, 8, 3, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Smartwatch Samsung Galaxy Watch 6', 100, 1400.00, 10, 3, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Casti Sony WH-1000XM5', 100, 1600.00, 15, 3, 1, 1)");

            // ===== ZONA D - ~50% (Portocaliu) =====
            // Total: 500 unitati
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Ulei Floarea Soarelui Bunica 1L', 200, 12.50, 50, 4, 2, 2)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Zahar Alb Cristal 1kg', 150, 7.00, 60, 4, 2, 2)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Faina Alba Dobrogea 1kg', 150, 6.50, 60, 4, 2, 2)");

            // ===== ZONA E - ~40% (Galben) =====
            // Total: 400 unitati
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Cafea Jacobs Kronung 500g', 5, 42.00, 20, 5, 2, 2)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Cafea Lavazza Qualita Oro 1kg', 50, 85.00, 15, 5, 2, 2)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Orez Basmati Uncle Bens 1kg', 100, 18.00, 30, 5, 2, 2)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Paste Fainoase Barilla 500g', 100, 8.50, 40, 5, 2, 2)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Ulei Masline Bertolli Extra Virgin 750ml', 100, 48.00, 20, 5, 2, 2)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Miere Albine Naturala 500g', 45, 35.00, 10, 5, 2, 2)");

            // ===== ZONA F - ~35% (Galben) =====
            // Total: 350 unitati
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Sampon Pantene Pro-V 400ml', 0, 28.00, 15, 6, 3, 3)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Balsam Pantene Repair 200ml', 8, 24.00, 15, 6, 3, 3)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Crema Hidratanta Nivea Soft 200ml', 3, 22.00, 10, 6, 3, 3)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Gel Dus Dove Deep Moisture 250ml', 150, 18.00, 25, 6, 3, 3)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Deodorant Rexona Men 150ml', 100, 19.50, 20, 6, 3, 3)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Pasta Dinti Colgate Total 75ml', 89, 14.00, 20, 6, 3, 3)");

            // ===== ZONA G - ~25% (Verde) =====
            // Total: 250 unitati
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Parfum Hugo Boss Bottled 100ml', 50, 380.00, 5, 7, 3, 3)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Parfum Chanel Chance 50ml', 30, 520.00, 5, 7, 3, 3)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Ruj L Oreal Color Riche', 80, 42.00, 15, 7, 3, 3)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Fond de Ten Maybelline Fit Me', 90, 68.00, 20, 7, 3, 3)");

            // ===== ZONA H - ~20% (Verde) =====
            // Total: 200 unitati
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Scaun Gaming DXRacer Formula', 40, 1450.00, 5, 8, 4, 4)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Birou Reglabil Standing Desk', 30, 2800.00, 3, 8, 4, 4)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Lampa Birou LED Philips Hue', 60, 250.00, 8, 8, 4, 4)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Raft Depozitare IKEA Kallax', 70, 380.00, 8, 8, 4, 4)");

            // ===== ZONA I - ~15% (Verde) =====
            // Total: 150 unitati
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Canapea 3 Locuri Ektorp', 20, 4200.00, 2, 9, 4, 4)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Masa Dining Extensa 120cm', 30, 2100.00, 3, 9, 4, 4)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Set 4 Scaune Dining Stefan', 50, 1200.00, 5, 9, 4, 4)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Pat Matrimonial 160x200', 30, 1800.00, 3, 9, 4, 4)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Saltea Memory Foam 160x200', 20, 1400.00, 2, 9, 4, 4)");

            // ===== ZONA J - ~10% (Verde) =====
            // Total: 100 unitati
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Router WiFi 6 Asus RT-AX88U', 30, 850.00, 5, 10, 5, 5)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Switch Managed 8 Porturi TP-Link', 40, 420.00, 8, 10, 5, 5)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('UPS APC 650VA', 30, 380.00, 5, 10, 5, 5)");

            // ===== ZONA K - ~8% (Verde) =====
            // Total: 80 unitati
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('SSD Samsung 970 EVO 1TB', 8, 520.00, 5, 11, 5, 5)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('SSD Kingston NV2 2TB', 32, 380.00, 8, 11, 5, 5)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('HDD Seagate Barracuda 4TB', 40, 320.00, 8, 11, 5, 5)");

            // ===== ZONA L - ~5% (Verde) =====
            // Total: 50 unitati
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('RAM Corsair Vengeance DDR5 32GB', 20, 420.00, 5, 12, 5, 5)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('RAM Kingston Fury Beast 16GB', 30, 220.00, 5, 12, 5, 5)");

            // ===== ZONA M - ~3% (Verde) =====
            // Total: 30 unitati
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Placa Video RTX 4070 Super', 10, 3800.00, 2, 13, 5, 5)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Placa Video RX 7800 XT', 20, 2800.00, 3, 13, 5, 5)");

            // ===== ZONA N, O, P - GOALE =====
            // Rezervate - demonstreaza zonele goale (gri)
        }
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
        sqlite3_prepare_v2(db,
            "UPDATE users SET activ = 0 WHERE id = ?",
            -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    void activeazaUser(int id) {
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db,
            "UPDATE users SET activ = 1 WHERE id = ?",
            -1, &stmt, nullptr);
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
        sqlite3_prepare_v2(db,
            "UPDATE produse SET cantitate = ? WHERE id = ?",
            -1, &stmt, nullptr);
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
        sqlite3_prepare_v2(db,
            "UPDATE zone SET capacitate_curenta = ? WHERE id = ?",
            -1, &stmt, nullptr);
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
                FROM produse
                WHERE produse.zona_id = zone.id
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
        sqlite3_prepare_v2(db,
            "INSERT INTO categorii (nume, descriere) VALUES (?,?)",
            -1, &stmt, nullptr);
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
        sqlite3_prepare_v2(db,
            "INSERT INTO furnizori (nume, telefon, email) VALUES (?,?,?)",
            -1, &stmt, nullptr);
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