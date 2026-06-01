#pragma once
#include <string>
#include <vector>
#include <functional>
#include "sqlite3.h"
#include "../models/Produs.h"
#include "../models/Categorie.h"
#include "../models/Furnizor.h"
#include "../models/Zona.h"
#include "../models/User.h"
#include "../data/sha256.h"

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
        // Creeaza admin implicit daca nu exista
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

        // Creeaza cele 16 zone daca nu exista
        sqlite3_prepare_v2(db,
            "SELECT COUNT(*) FROM zone",
            -1, &stmt, nullptr);
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
};