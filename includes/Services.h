//
// Created by pravl on 30.04.2025.
//

#ifndef SERVICES_H
#define SERVICES_H

#include "Models.h"
#include <vector>
#include <string>
#include <ctime>

class FleetService {
public:
    // 2.1 Рейсы траулера за период
    std::vector<std::pair<Voyage, double>> getVoyagesByTrawler(int trawlerId,
        const std::tm& from, const std::tm& to);

    // 2.2 Улов по банке
    std::vector<std::pair<std::string, double>> getCatchByBank(int bankId);

    // 2.3 Банка с макс низкого качества
    struct LowQualityVoyage { Voyage v; std::string trawlerName; };
    std::vector<LowQualityVoyage> getMaxLowQualityBankVoyages();

    // 2.4 Траулер с наибольшим уловом
    struct CaptainBank { std::string captainLastName; Bank bank; Voyage v; };
    std::vector<CaptainBank> getTopTrawlerInfo();

    // 2.5 Пенсионеры на дату
    std::vector<CrewMember> getPensioners(const std::tm& date);

    // 3. CRUD-операции (примеры)
    bool addTrawler(const Trawler&);
    bool updateCrewRole(int crewId, const std::string& newRole);
    bool deleteZeroCatch();

    // 5. Начисление премий за внеплановый улов
    bool awardBonuses(const std::tm& start, const std::tm& end,
                      double plannedKg, double avgPrice);

    // 6. Начисление премии конкретному члену
    bool awardBonusToMember(int crewId, const std::tm& start,
                            const std::tm& end, double plannedKg, double avgPrice);

private:
    // Вспомогательные методы для подготовки и выполнения запросов
    static bool parseDate(const std::string& s, std::tm& dt);
    static std::string toSqlDate(const std::tm& dt);

};

#endif //SERVICES_H
