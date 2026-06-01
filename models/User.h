#pragma once
#include <string>

enum class Rol {
    ADMIN,
    ANGAJAT
};

class User {
private:
    int id;
    std::string username;
    std::string passwordHash;
    Rol rol;
    bool activ;

public:
    User() : id(0), rol(Rol::ANGAJAT), activ(true) {}
    User(int id, std::string username, std::string passwordHash, Rol rol = Rol::ANGAJAT)
        : id(id), username(username), passwordHash(passwordHash), rol(rol), activ(true) {}

    int getId() const { return id; }
    std::string getUsername() const { return username; }
    std::string getPasswordHash() const { return passwordHash; }
    Rol getRol() const { return rol; }
    bool eActiv() const { return activ; }

    void setUsername(std::string u) { username = u; }
    void setPasswordHash(std::string h) { passwordHash = h; }
    void setRol(Rol r) { rol = r; }
    void setActiv(bool a) { activ = a; }

    bool eAdmin() const { return rol == Rol::ADMIN; }

    std::string getRolString() const {
        return rol == Rol::ADMIN ? "Admin" : "Angajat";
    }
};