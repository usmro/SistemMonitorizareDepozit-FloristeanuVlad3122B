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

// Structura pentru istoricul tranzactiilor
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
        executeSQL(R"(
            CREATE TABLE IF NOT EXISTS categorii (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                nume TEXT NOT NULL,
                descriere TEXT
            );
        )");
        executeSQL(R"(
            CREATE TABLE IF NOT EXISTS furnizori (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                nume TEXT NOT NULL,
                telefon TEXT,
                email TEXT
            );
        )");
        executeSQL(R"(
            CREATE TABLE IF NOT EXISTS zone (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                litera TEXT NOT NULL,
                capacitate_max INTEGER DEFAULT 100,
                capacitate_curenta INTEGER DEFAULT 0
            );
        )");
        executeSQL(R"(
            CREATE TABLE IF NOT EXISTS produse (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                nume TEXT NOT NULL,
                cantitate INTEGER DEFAULT 0,
                pret REAL DEFAULT 0.0,
                prag_alerta INTEGER DEFAULT 10,
                zona_id INTEGER DEFAULT 0,
                categorie_id INTEGER DEFAULT 0,
                furnizor_id INTEGER DEFAULT 0
            );
        )");
        executeSQL(R"(
            CREATE TABLE IF NOT EXISTS tranzactii (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                produs_id INTEGER,
                tip TEXT,
                cantitate INTEGER,
                data TEXT,
                observatii TEXT
            );
        )");
        executeSQL(R"(
            CREATE TABLE IF NOT EXISTS users (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                username TEXT UNIQUE NOT NULL,
                password_hash TEXT NOT NULL,
                rol TEXT DEFAULT 'ANGAJAT',
                activ INTEGER DEFAULT 1
            );
        )");
    }

    void initDateImplicite() {
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db,
            "SELECT COUNT(*) FROM users WHERE username = 'admin'",
            -1, &stmt, nullptr);
        sqlite3_step(stmt);
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);

        if (count == 0) {
            std::string hash = SHA256::hash("admin123");
            executeSQL("INSERT INTO users (username, password_hash, rol) VALUES ('admin', '" + hash + "', 'ADMIN')");
        }

        sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM zone", -1, &stmt, nullptr);
        sqlite3_step(stmt);
        count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);

        if (count == 0) {
            for (int i = 0; i < 16; i++) {
                char litera = 'A' + i;
                std::string sql = "INSERT INTO zone (litera, capacitate_max, capacitate_curenta) VALUES ('" +
                    std::string(1, litera) + "', 100, 0)";
                executeSQL(sql);
            }
        }

        sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM categorii", -1, &stmt, nullptr);
        sqlite3_step(stmt);
        count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);

        if (count == 0) {
            executeSQL("INSERT INTO categorii (nume, descriere) VALUES ('Electronice', 'Produse electronice si IT')");
            executeSQL("INSERT INTO categorii (nume, descriere) VALUES ('Alimente', 'Produse alimentare')");
            executeSQL("INSERT INTO categorii (nume, descriere) VALUES ('Cosmetice', 'Produse cosmetice si ingrijire')");
            executeSQL("INSERT INTO categorii (nume, descriere) VALUES ('Mobilier', 'Mobila si decoratiuni')");
        }

        sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM furnizori", -1, &stmt, nullptr);
        sqlite3_step(stmt);
        count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);

        if (count == 0) {
            executeSQL("INSERT INTO furnizori (nume, telefon, email) VALUES ('TechSupply SRL', '0721000001', 'contact@techsupply.ro')");
            executeSQL("INSERT INTO furnizori (nume, telefon, email) VALUES ('FoodDist SA', '0721000002', 'office@fooddist.ro')");
            executeSQL("INSERT INTO furnizori (nume, telefon, email) VALUES ('CosmeticPro', '0721000003', 'info@cosmeticpro.ro')");
        }

        sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM produse", -1, &stmt, nullptr);
        sqlite3_step(stmt);
        count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);

        if (count == 0) {
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Laptop Dell', 45, 3500.00, 10, 1, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Mouse Wireless', 8, 120.00, 20, 1, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Cafea Jacobs 500g', 5, 35.00, 15, 3, 2, 2)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Sampon Pantene', 0, 22.50, 10, 5, 3, 3)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Tastatura Mecanica', 30, 450.00, 5, 2, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Ulei Floarea Soarelui', 120, 12.00, 30, 4, 2, 2)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Monitor 27inch', 12, 1200.00, 5, 2, 1, 1)");
            executeSQL("INSERT INTO produse (nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id) VALUES ('Crema Nivea', 3, 18.00, 10, 6, 3, 3)");
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
    std::vector<Produs> getProduse() {
        std::vector<Produs> produse;
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db,
            "SELECT id, nume, cantitate, pret, prag_alerta, zona_id, categorie_id, furnizor_id FROM produse",
            -1, &stmt, nullptr);
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
    void adaugaTranzactie(int produsId, const std::string& tip, int cantitate, const std::string& observatii = "") {
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
            r.data = sqlite3_column_text(stmt, 5) ? (const char*)sqlite3_column_text(stmt, 5) : "";
            r.observatii = sqlite3_column_text(stmt, 6) ? (const char*)sqlite3_column_text(stmt, 6) : "";
            rezultat.push_back(r);
        }
        sqlite3_finalize(stmt);
        return rezultat;
    }
};