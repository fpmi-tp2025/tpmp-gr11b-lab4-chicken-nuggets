//
// Created by pravl on 30.04.2025.
//

#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>
#include <ctime>

struct Trawler {
    int      id;
    std::string name;
    int      displacement;
    std::tm  builtDate;
};

struct CrewMember {
    int      id;
    std::string lastName;
    std::string role;
    std::tm  hireDate;
    int      birthYear;
    bool     isManager;      // true = управление, false = член экипажа
    std::string password;    // для аутентификации
};


struct Bank {
    int      id;
    std::string name;
};

struct Voyage {
    int      id;
    int      trawlerId;
    std::tm  departureDate;
    std::tm  returnDate;
    int      bankId;
};

struct Catch {
    int      id;
    int      voyageId;
    std::string fishName;
    std::string quality; // "high", "medium", "low"
    double   quantityKg;
};

#endif //MODELS_H
