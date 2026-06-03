# Sistem de Monitorizare a Stocurilor unui Depozit

## Informatii Generale

- **Autor:** Floristeanu Vlad
- **Grupa:** 3122B
- **Materie:** Programare Orientata pe Obiecte
- **An universitar:** 2025-2026
- **Branch Git:** develop

---

## 1. Descrierea Proiectului

Proiectul reprezinta un sistem informatic complet pentru monitorizarea stocurilor unui depozit de tip retail, inspirat din modelul Carrefour/Lidl. Sistemul permite urmarirea produselor pe zone fizice, gestionarea cantitatilor la intrare/iesire, generarea de alerte automate si administrarea utilizatorilor cu roluri diferite.

### Functionalitati principale

- Harta vizuala a depozitului cu 16 zone (A-P) colorate dupa gradul de ocupare
- Gestionare produse cu alerte automate de stoc scazut
- Sistem de autentificare cu parole criptate SHA-256
- Roluri Admin si Angajat cu permisiuni diferite
- Istoric complet al tranzactiilor (INTRARE/IESIRE/MUTARE)
- Sugestie automata de zona alternativa cand o zona este plina
- Paginare pentru seturi mari de date (10/25/50/100 produse/pagina)
- Sortare si cautare case-insensitive in toate tabelele
- Capacitate maxima 1000 unitati per zona (16.000 total depozit)

---

## 2. Arhitectura Proiectului

    SistemMonitorizareDepozit/
    ├── models/                  # Entitati de date
    │   ├── EntitateDepozit.h    # Clasa de baza abstracta
    │   ├── Produs.h             # Mosteneste EntitateDepozit
    │   ├── Categorie.h          # Mosteneste EntitateDepozit
    │   ├── Furnizor.h           # Mosteneste EntitateDepozit
    │   ├── Zona.h               # Zona fizica din depozit
    │   ├── User.h               # Utilizator cu rol
    │   └── Tranzactie.h         # Template Intrare/Iesire
    ├── core/                    # Logica de business
    │   ├── Depozit.h            # Manager principal stocuri
    │   ├── AlertaManager.h      # Sistem notificari automate
    │   └── Raport.h             # Generare rapoarte
    ├── data/                    # Persistenta date
    │   ├── Database.h           # SQLite3 CRUD + indexuri
    │   └── sha256.h             # Criptare parole single-header
    ├── ui/                      # Interfata grafica ImGui
    │   ├── LoginUI.h            # Pagina autentificare
    │   └── MainUI.h             # Interfata principala cu tabs
    ├── tests/                   # Teste unitare
    │   └── TestRunner.h         # 84 teste automate
    ├── CMakeLists.txt           # Build system cross-platform
    ├── Documentatie.md          # Documentatie tehnica
    ├── README.md                # Pagina GitHub
    └── main.cpp                 # Entry point

---

## 3. Concepte POO Implementate

### 3.1 Incapsulare

Toti membrii privati sunt accesibili exclusiv prin getteri/setteri publici:

    class Produs : public EntitateDepozit {
    private:
        int cantitate;
        double pret;
        int pragAlerta;
    public:
        int getCantitate() const { return cantitate; }
        void setCantitate(int c) { cantitate = c; }
    };

### 3.2 Mostenire

Ierarhie cu clasa de baza abstracta EntitateDepozit:

    class EntitateDepozit {
    public:
        virtual void afisare() const = 0;
        virtual std::string getTip() const = 0;
        virtual std::string getInfo() const = 0;
        virtual ~EntitateDepozit() = default;
    };

    class Produs    : public EntitateDepozit { ... };
    class Furnizor  : public EntitateDepozit { ... };
    class Categorie : public EntitateDepozit { ... };

### 3.3 Polimorfism si Functii Virtuale

    std::vector<EntitateDepozit*> entitati = { &produs, &furnizor, &categorie };
    for (auto* e : entitati)
        e->afisare(); // Apeleaza implementarea corecta pentru fiecare tip

### 3.4 Supraincarcarea Operatorilor

    // Operator += pentru adaugare stoc
    Produs& operator+=(int cantitateAdaugata) {
        if (cantitateAdaugata < 0)
            throw std::invalid_argument("Nu poti adauga o cantitate negativa!");
        cantitate += cantitateAdaugata;
        return *this;
    }

    // Operator -= pentru scadere stoc
    Produs& operator-=(int cantitateScazuta) {
        if (cantitateScazuta < 0)
            throw std::invalid_argument("Nu poti scadea o cantitate negativa!");
        if (cantitate - cantitateScazuta < 0)
            throw std::runtime_error("Stoc insuficient!");
        cantitate -= cantitateScazuta;
        return *this;
    }

    // Operator << pentru afisare
    friend std::ostream& operator<<(std::ostream& os, const EntitateDepozit& e);

    // Operator == pentru comparatie
    virtual bool operator==(const EntitateDepozit& other) const;

### 3.5 Clase Template

    template<typename T>
    class Tranzactie {
        std::string getTip() const {
            if constexpr (std::is_same_v<T, Intrare>) return "INTRARE";
            else return "IESIRE";
        }
    };

    using TranzactieIntrare = Tranzactie<Intrare>;
    using TranzactieIesire  = Tranzactie<Iesire>;

### 3.6 Membri Statici

    class Produs {
        static int nrTotalInstante;
    public:
        static int getNrTotalInstante() { return nrTotalInstante; }
    };
    inline int Produs::nrTotalInstante = 0;

### 3.7 Rule of Three

    // Constructor de copiere
    Produs(const Produs& other) : EntitateDepozit(other.id, other.nume),
        cantitate(other.cantitate), pret(other.pret) {
        nrTotalInstante++;
    }

    // Operator de atribuire
    Produs& operator=(const Produs& other) {
        if (this != &other) { /* copiere membri */ }
        return *this;
    }

    // Destructor virtual
    ~Produs() override { nrTotalInstante--; }

### 3.8 Gestionarea Exceptiilor

    void adaugaStoc(int id, int cantitate) {
        if (produse.find(id) == produse.end())
            throw std::runtime_error("Produsul nu exista!");
        if (!areSpatiuZona(zonaId, cantitate))
            throw std::runtime_error("Zona plina!");
        produse[id] += cantitate;
    }

### 3.9 STL Containers

    std::unordered_map<int, Produs> produse;   // O(1) cautare dupa ID
    std::unordered_map<int, Zona>   zone;      // O(1) cautare dupa ID
    std::vector<Produs>             rezultat;  // Lista dinamica
    std::sort(v.begin(), v.end(), comparator); // Sortare STL

### 3.10 Enum Class

    enum class Rol {
        ADMIN,
        ANGAJAT
    };

---

## 4. Descrierea Claselor

### EntitateDepozit - Clasa de baza abstracta

Contine id si nume comune tuturor entitatilor. Declara functii virtuale pure afisare(), getTip(), getInfo(). Destructor virtual pentru polimorfism corect. Supraincarca operator<< si operator==.

### Produs - Derivata din EntitateDepozit

Campuri: cantitate, pret, pragAlerta, zonaId, categorieId, furnizorId. Supraincarca operator+= si operator-= pentru gestiunea stocului. Membru static nrTotalInstante pentru contorizare globala. Implementeaza Rule of Three (copy constructor, operator=, destructor).

### Furnizor - Derivata din EntitateDepozit

Campuri: telefon, email, produseIds. Asociere cu produsele prin vector de ID-uri.

### Categorie - Derivata din EntitateDepozit

Campuri: descriere. Categorizeaza produsele (Electronice, Alimente, Cosmetice, Mobilier, Componente PC).

### Zona

Reprezinta o zona fizica din depozit (A-P). Calculeaza procentul de ocupare capped la 100%. Returneaza culoarea ImGui corespunzatoare (5 culori). Capacitate maxima 1000 unitati.

### Tranzactie - Template

Parametrizat cu Intrare sau Iesire. Inregistreaza produsId, cantitate, data, observatii. Data setata automat la creare.

### Depozit

Gestioneaza colectia de produse si zone cu unordered_map. Valideaza capacitatea zonelor la fiecare operatie. Sugereaza automat zona alternativa cand o zona e plina. Suporta mutarea produselor intre zone.

### AlertaManager

Monitorizeaza produsele sub pragul de alerta. Tipuri: CRITIC (stoc 0), ATENTIE (sub prag). Culori: rosu pentru CRITIC, portocaliu pentru ATENTIE.

### Raport

Clasa statica cu metode de raportare. Top 5 produse critice sortate dupa cantitate. Valoare totala stoc, numar produse sub prag.

### Database

Wrapper complet peste SQLite3 cu CRUD pentru toate entitatile. Genereaza ~1500 produse individuale procedural la prima pornire. Indexuri SQLite pentru performanta ridicata.

### SHA256

Implementare single-header fara dependente externe. Folosita pentru criptarea parolelor utilizatorilor.

---

## 5. Baza de Date SQLite3

### Tabele

| Tabel | Descriere |
|---|---|
| produse | Stocul produselor cu zona, categorie, furnizor |
| zone | Cele 16 zone fizice A-P cu capacitate maxima 1000 |
| categorii | Categorii produse |
| furnizori | Furnizori cu informatii de contact |
| tranzactii | Istoric complet intrari/iesiri/mutari |
| users | Conturi cu roluri si hash parola SHA-256 |

### Indexuri pentru performanta

    CREATE INDEX idx_produse_zona      ON produse(zona_id);
    CREATE INDEX idx_produse_nume      ON produse(nume);
    CREATE INDEX idx_produse_prag      ON produse(cantitate, prag_alerta);
    CREATE INDEX idx_tranzactii_produs ON tranzactii(produs_id);
    CREATE INDEX idx_tranzactii_data   ON tranzactii(data);
    CREATE INDEX idx_users_username    ON users(username);

---

## 6. Interfata Grafica

Implementata cu ImGui 1.91.0 + GLFW 3.4 + OpenGL3, tema Dark Blue custom.

### Pagina Login

Autentificare cu username si parola. Parola criptata SHA-256. Mesaj de eroare la date incorecte. Conectare prin Enter sau buton.

### Tab Dashboard

Stanga (42%): Harta depozit 4x4 cu 16 zone colorate. Click pe zona afiseaza produsele din ea cu scrollbar si cautare. Legenda culori.

Dreapta (58%): Tabel produse cu sortare pe coloane, cautare, paginare (10/25/50/100 per pagina). Formular adaugare produs doar Admin. Gestiune stoc cu observatii si afisare spatiu disponibil. Popup sugestie zona alternativa cand zona e plina. Alerte active cu scrollbar. Rapoarte colapsabile.

### Tab Tranzactii

Istoric complet sortabil pe toate coloanele. INTRARE (verde), IESIRE (rosu), MUTARE (albastru).

### Tab Admin (doar Admin)

Creare si dezactivare conturi angajati. Vizualizare hash parole. Gestionare categorii si furnizori cu cautare si sortare.

---

## 7. Securitate

Parolele sunt stocate exclusiv ca hash SHA-256 de 64 caractere hex, niciodata in text clar. Roluri separate Admin/Angajat cu restrictii UI. Conturile pot fi dezactivate fara stergere permanenta.

---

## 8. Cum se Ruleaza

### Cerinte sistem

- CMake 3.20+
- GCC/MinGW 16.x pe Windows sau GCC pe Linux
- Git in PATH pentru FetchContent
- Conexiune internet la primul build

### Windows - Pas cu Pas

#### Pasul 1 - Instaleaza MinGW

Descarca de la https://winlibs.com versiunea GCC 16.x Win64 ZIP.
Extrage arhiva si muta folderul la C:\mingw64.

Adauga MinGW in PATH (PowerShell ca Administrator):

    [System.Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\mingw64\bin", "Machine")

Verifica instalarea (inchide si redeschide PowerShell):

    gcc --version

Rezultat asteptat: gcc.exe (MinGW-W64) 16.1.0

#### Pasul 2 - Instaleaza Git

Descarca de la https://git-scm.com si instaleaza cu optiunile default.

Adauga Git in PATH (PowerShell ca Administrator):

    [System.Environment]::SetEnvironmentVariable("Path", $env:Path + ";C:\Program Files\Git\cmd", "Machine")

Verifica instalarea (inchide si redeschide PowerShell):

    git --version

Rezultat asteptat: git version 2.45.x.windows.x

#### Pasul 3 - Cloneaza Repository-ul

In PowerShell:

    git clone https://github.com/FloristeanuVlad/SistemMonitorizareDepozit
    cd SistemMonitorizareDepozit

#### Pasul 4 - Deschide in CLion

1. Deschide CLion
2. File → Open → navigheaza la folderul clonat → selecteaza CMakeLists.txt → OK
3. La intrebarea Load CMake Project click Yes

#### Pasul 5 - Configureaza Toolchain

1. File → Settings → Build, Execution, Deployment → Toolchains
2. Click + → MinGW
3. Toolset: C:\mingw64
4. Asteapta sa detecteze gcc.exe si g++.exe (bifa verde)
5. Muta MinGW primul in lista cu sageata sus
6. Apply → OK

#### Pasul 6 - Ruleaza

Apasa butonul Run (sageata verde) din bara de sus.
Prima rulare descarca automat ImGui, GLFW si SQLite3 (~5 minute).
La urmatoarele rulari porneste instant.

### Linux - Ubuntu/Debian

Instaleaza dependentele sistem:

    sudo apt install cmake gcc g++ libgl1-mesa-dev xorg-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev -y

Cloneaza si ruleaza:

    git clone https://github.com/FloristeanuVlad/SistemMonitorizareDepozit
    cd SistemMonitorizareDepozit
    mkdir cmake-build-debug && cd cmake-build-debug
    cmake ..
    cmake --build .
    ./ProiectPOO

Prima rulare descarca automat ImGui, GLFW si SQLite3 prin FetchContent.

### Date implicite la prima pornire

- Username: admin
- Parola: admin123
- ~1500 produse individuale generate procedural din combinatii brand x model x varianta
- 16 zone (A-P) cu capacitate 1000 fiecare
- Zona A la 100% plina pentru demo sugestie zona alternativa

### Dependente descarcate automat prin FetchContent

- ImGui v1.91.0
- GLFW 3.4
- SQLite3 3.46.1

---

## 9. Teste Unitare

84 teste unitare organizate in 8 categorii, rulate automat la pornirea aplicatiei:

| Categorie | Teste | Ce verifica |
|---|---|---|
| Produs | 18 | Constructor, setteri, getteri, valoare totala |
| Operatori | 5 | +=, -=, lant de operatii, valori limita |
| Exceptii | 4 | Validari, stoc insuficient, cantitate negativa |
| Zona | 12 | Capacitate, culori, procente, limita 100% |
| SHA256 | 5 | Hash consistent, lungime, case sensitive |
| Tranzactie | 7 | Template, tip INTRARE/IESIRE, data, observatii |
| User | 8 | Roluri Admin/Angajat, activ/inactiv |
| Mostenire | 25 | Polimorfism, virtual, static, Rule of Three |

Rezultat final: 84/84 PASS

---

## 10. Structura Git

Branch: develop

| Commit | Descriere |
|---|---|
| feat: initial project setup | Setup CMake, ImGui, SQLite3, GLFW via FetchContent |
| feat: core classes | Produs, Depozit, Zona, Furnizor, Tranzactie, Database, UI |
| feat: zone coloring, test data | Culori zone, date demo realiste, alerte |
| feat: UI styling | Tema Dark Blue, tab-uri, roluri Admin/Angajat, logout |
| feat: unit tests | TestRunner cu 84 teste, toate trecute |
| feat: inheritance + polymorphism | EntitateDepozit abstracta, mostenire, polimorfism |
| feat: pagination, indexes, zone capacity | Paginare, indexuri SQLite, capacitate zona 1000 |
| docs: Documentatie.md si README.md complete | Documentatie tehnica si instructiuni rulare |