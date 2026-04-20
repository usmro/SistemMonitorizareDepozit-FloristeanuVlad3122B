# Sistem de Monitorizare a Stocurilor unui Depozit

## 1. Analiza Temei
Proiectul are ca scop dezvoltarea unui sistem informatic pentru gestionarea stocurilor dintr-un depozit.
Sistemul permite urmărirea precisă a produselor, actualizarea cantităților la intrarea/ieșirea din gestiune și generarea de alerte atunci când stocul scade sub un anumit prag critic.

**Arhitectura de bază:**
* **Modele de date:** `Produs` (reprezintă entitatea de bază).
* **Servicii/Gestiune:** `Depozit` (gestionează colecția de produse folosind un `std::unordered_map` pentru căutări rapide O(1) bazate pe ID).
* **Tratarea Erorilor:** Excepții customizate pentru validări (ex: `ProdusExistentException`, `ProdusInexistentException`, `StocInsuficientException`).

## 2. Diagrama UML Simplificată

```mermaid
classDiagram
    class Produs {
        - int ID
        - string nume
        - int cantitate
        - double pret
        - int pragAlerta
        + Produs(ID, nume, cantitate, pret, pragAlerta)
        + operator+=(cantitate) Produs&
        + operator-=(cantitate) Produs&
        + getID() int
        + getCantitate() int
        + getPragAlerta() int
        + afisare() void
    }

    class Depozit {
        - unordered_map~int, Produs~ stocuri
        + adaugaProdus(Produs p) void
        + eliminaProdus(int id) void
        + actualizeazaStoc(int id, int cantitate, bool esteIntrare) void
        + genereazaRaportAlerte() void
    }

    Depozit "1" *-- "0..*" Produs : contine >