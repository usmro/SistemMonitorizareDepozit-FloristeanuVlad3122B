# Sistem de Monitorizare a Stocurilor unui Depozit

Proiect POO — Floristeanu Vlad, Grupa 3122B

Sistem informatic complet pentru monitorizarea stocurilor unui depozit de tip retail, dezvoltat in C++ cu interfata grafica ImGui, baza de date SQLite3 si autentificare cu parole criptate SHA-256.

---

## Functionalitati

- Harta depozit cu 16 zone (A-P) colorate dupa gradul de ocupare
- Gestiune stoc cu adaugare, scadere, editare si istoric tranzactii
- Alerte automate pentru produse sub prag si stoc epuizat
- Autentificare cu login si parole criptate SHA-256
- Roluri Admin si Angajat cu permisiuni diferite
- Sugestie automata de zona alternativa cand o zona este plina
- Rapoarte cu valoare totala stoc si top produse critice
- Paginare pentru seturi mari de date
- Sortare si cautare in toate tabelele

---

## Tehnologii

| Tehnologie | Versiune | Rol |
|---|---|---|
| C++ | 20 | Limbaj principal |
| ImGui | 1.91.0 | Interfata grafica |
| GLFW | 3.4 | Fereastra si input |
| SQLite3 | 3.46.1 | Baza de date |
| CMake | 3.20+ | Build system cross-platform |
| SHA-256 | - | Criptare parole |

---

## Concepte POO

- Mostenire: EntitateDepozit (baza abstracta) derivata in Produs, Furnizor, Categorie
- Polimorfism: functii virtuale pure si dispatch dinamic
- Template: Tranzactie parametrizat cu Intrare sau Iesire
- Operatori supraincarcati: +=, -=, ==,
- Exceptii: validare stoc, capacitate zona, ID invalid
- STL: unordered_map, vector, sort, algorithm
- Membri statici: contor global instante Produs
- Rule of Three: copy constructor, operator=, destructor

---

## Cum rulezi proiectul

### Cerinte

- GCC/MinGW 16.x
- CMake 3.20+
- Git in PATH

### Windows

    git clone https://github.com/FloristeanuVlad/SistemMonitorizareDepozit
    cd SistemMonitorizareDepozit
    mkdir cmake-build-debug
    cd cmake-build-debug
    cmake -G "MinGW Makefiles" ..
    cmake --build .
    ProiectPOO.exe

### Linux

    git clone https://github.com/FloristeanuVlad/SistemMonitorizareDepozit
    cd SistemMonitorizareDepozit
    mkdir cmake-build-debug
    cd cmake-build-debug
    cmake ..
    cmake --build .
    ./ProiectPOO

Prima rulare descarca automat ImGui, GLFW si SQLite3 prin FetchContent.

---

## Date implicite

| Camp | Valoare |
|---|--|
| Username | admin |
| Parola | admin123 |
| Produse pre-generate | 1500 (generate procedural) |
| Zone | 16 (A-P), capacitate 1000 fiecare |
| Zona A | 100% plina pentru demo sugestie |

---

## Teste unitare

84 teste organizate in 8 categorii rulate automat la pornire.

Categorii: Produs, Operatori, Exceptii, Zona, SHA256, Tranzactie, User, Mostenire si Polimorfism.

Rezultat: 84/84 PASS

---

## Structura proiect

    models/    Entitati: Produs, Furnizor, Categorie, Zona, User, Tranzactie
    core/      Logica: Depozit, AlertaManager, Raport
    data/      Persistenta: Database SQLite3, SHA256
    ui/        Interfata: LoginUI, MainUI
    tests/     84 teste unitare automate
    main.cpp   Entry point

---

## Autor

Floristeanu Vlad, Grupa 3122B
Programare Orientata pe Obiecte, 2025-2026