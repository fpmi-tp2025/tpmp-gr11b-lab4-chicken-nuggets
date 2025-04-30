//
// Created by pravl on 30.04.2025.
//

#include "Services.h"
#include "Database.h"
#include <sqlite3.h>
#include <sstream>
#include <iomanip>
#include <iostream>

// static std::string toSqlDate(const std::tm& dt) {
//     char buf[11];
//     std::strftime(buf, sizeof(buf), "%Y-%m-%d", &dt);
//     return buf;
// }

// static bool parseDate(const std::string& s, std::tm& dt) {
//     std::istringstream ss(s);
//     ss >> std::get_time(&dt, "%Y-%m-%d");
//     return !ss.fail();
// }

bool FleetService::parseDate(const std::string& s, std::tm& dt) {
    std::istringstream ss(s);
    ss >> std::get_time(&dt, "%Y-%m-%d");
    return !ss.fail();
}

std::string FleetService::toSqlDate(const std::tm& dt) {
    char buf[11];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", &dt);
    return buf;
}

// 2.1 Рейсы траулера за период
std::vector<std::pair<Voyage, double>> FleetService::getVoyagesByTrawler(
    int trawlerId, const std::tm& from, const std::tm& to)
{
    auto db = Database::instance().handle();
    const char* sql =
        "SELECT v.voyage_id, departure_date, return_date, COALESCE(SUM(c.quantity_kg),0) "
        "FROM VOYAGES v "
        "LEFT JOIN CATCHES c ON v.voyage_id = c.voyage_id "
        "WHERE v.trawler_id = ? AND v.departure_date BETWEEN ? AND ? "
        "GROUP BY v.voyage_id;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, trawlerId);
    sqlite3_bind_text(stmt, 2, toSqlDate(from).c_str(), -1, nullptr);
    sqlite3_bind_text(stmt, 3, toSqlDate(to).c_str(), -1, nullptr);

    std::vector<std::pair<Voyage, double>> res;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Voyage v;
        v.id = sqlite3_column_int(stmt, 0);
        std::string dep = reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
        std::string ret = reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
        parseDate(dep, v.departureDate);
        parseDate(ret, v.returnDate);
        double total = sqlite3_column_double(stmt, 3);
        res.emplace_back(v, total);
    }
    sqlite3_finalize(stmt);
    return res;
}

// 2.2 Улов по банке
std::vector<std::pair<std::string, double>> FleetService::getCatchByBank(int bankId) {
    auto db = Database::instance().handle();
    const char* sql =
        "SELECT fish_name, SUM(quantity_kg) "
        "FROM CATCHES c JOIN VOYAGES v ON c.voyage_id=v.voyage_id "
        "WHERE v.bank_id = ? GROUP BY fish_name;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, bankId);

    std::vector<std::pair<std::string, double>> res;
    while (sqlite3_step(stmt)==SQLITE_ROW) {
        res.emplace_back(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt,0)),
            sqlite3_column_double(stmt,1)
        );
    }
    sqlite3_finalize(stmt);
    return res;
}

// 2.3 Банка с макс уловом низкого качества
std::vector<FleetService::LowQualityVoyage> FleetService::getMaxLowQualityBankVoyages() {
    auto db = Database::instance().handle();
    const char* sql =
        "WITH low_catches AS ("
        "  SELECT v.bank_id, SUM(c.quantity_kg) AS sum_low "
        "  FROM CATCHES c "
        "  JOIN FISH_QUALITY q ON c.quality_id=q.quality_id "
        "  JOIN VOYAGES v ON c.voyage_id=v.voyage_id "
        "  WHERE q.quality_level='low' "
        "  GROUP BY v.bank_id"
        ") "
        "SELECT v.voyage_id, departure_date, return_date, t.name "
        "FROM low_catches lc "
        "JOIN VOYAGES v ON lc.bank_id=v.bank_id "
        "JOIN TRAWLERS t ON v.trawler_id=t.trawler_id "
        "WHERE lc.sum_low = (SELECT MAX(sum_low) FROM low_catches);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    std::vector<LowQualityVoyage> res;
    while (sqlite3_step(stmt)==SQLITE_ROW) {
        Voyage v; LowQualityVoyage lq;
        v.id = sqlite3_column_int(stmt,0);
        parseDate(reinterpret_cast<const char*>(sqlite3_column_text(stmt,1)), v.departureDate);
        parseDate(reinterpret_cast<const char*>(sqlite3_column_text(stmt,2)), v.returnDate);
        lq.v = v;
        lq.trawlerName = reinterpret_cast<const char*>(sqlite3_column_text(stmt,3));
        res.push_back(lq);
    }
    sqlite3_finalize(stmt);
    return res;
}

// 2.4 Траулер с наибольшим уловом
std::vector<FleetService::CaptainBank> FleetService::getTopTrawlerInfo() {
    auto db = Database::instance().handle();
    const char* sql =
        "WITH trawler_totals AS ("
        "  SELECT v.trawler_id, SUM(c.quantity_kg) AS total_sum "
        "  FROM VOYAGES v JOIN CATCHES c ON v.voyage_id=c.voyage_id "
        "  GROUP BY v.trawler_id"
        ") "
        "SELECT cm.last_name, b.bank_id, b.name, v.voyage_id, v.departure_date, v.return_date "
        "FROM trawler_totals tt "
        "JOIN VOYAGES v ON tt.trawler_id=v.trawler_id "
        "JOIN BANKS b ON v.bank_id=b.bank_id "
        "JOIN CREW_MEMBERS cm ON cm.role='Captain' AND cm.crew_id IN ("
        "  SELECT crew_id FROM CREW_MEMBERS WHERE role='Captain' LIMIT 1"
        ") "
        "WHERE tt.total_sum = (SELECT MAX(total_sum) FROM trawler_totals);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

    std::vector<CaptainBank> res;
    while (sqlite3_step(stmt)==SQLITE_ROW) {
        CaptainBank cb;
        cb.captainLastName = reinterpret_cast<const char*>(sqlite3_column_text(stmt,0));
        cb.bank.id   = sqlite3_column_int(stmt,1);
        cb.bank.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
        cb.v.id      = sqlite3_column_int(stmt,3);
        parseDate(reinterpret_cast<const char*>(sqlite3_column_text(stmt,4)), cb.v.departureDate);
        parseDate(reinterpret_cast<const char*>(sqlite3_column_text(stmt,5)), cb.v.returnDate);
        res.push_back(cb);
    }
    sqlite3_finalize(stmt);
    return res;
}

// 2.5 Пенсионеры
std::vector<CrewMember> FleetService::getPensioners(const std::tm& date) {
    auto db = Database::instance().handle();
    const char* sql =
        "SELECT crew_id, last_name, role, hire_date, birth_year, isManager, password "
        "FROM CREW_MEMBERS "
        "WHERE date(?) >= date(birth_year || '-01-01', '+60 years');";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, toSqlDate(date).c_str(), -1, nullptr);

    std::vector<CrewMember> res;
    while (sqlite3_step(stmt)==SQLITE_ROW) {
        CrewMember cm;
        cm.id         = sqlite3_column_int(stmt,0);
        cm.lastName   = reinterpret_cast<const char*>(sqlite3_column_text(stmt,1));
        cm.role       = reinterpret_cast<const char*>(sqlite3_column_text(stmt,2));
        parseDate(reinterpret_cast<const char*>(sqlite3_column_text(stmt,3)), cm.hireDate);
        cm.birthYear  = sqlite3_column_int(stmt,4);
        cm.isManager  = sqlite3_column_int(stmt,5)!=0;
        cm.password   = reinterpret_cast<const char*>(sqlite3_column_text(stmt,6));
        res.push_back(cm);
    }
    sqlite3_finalize(stmt);
    return res;
}

// 3. CRUD
bool FleetService::addTrawler(const Trawler& t) {
    auto db = Database::instance().handle();
    const char* sql = "INSERT INTO TRAWLERS(name,displacement,built_date) VALUES(?,?,?);";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt,1,t.name.c_str(),-1,nullptr);
    sqlite3_bind_int(stmt,2,t.displacement);
    sqlite3_bind_text(stmt,3,toSqlDate(t.builtDate).c_str(),-1,nullptr);
    bool ok = sqlite3_step(stmt)==SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool FleetService::updateCrewRole(int crewId, const std::string& newRole) {
    auto db = Database::instance().handle();
    const char* sql = "UPDATE CREW_MEMBERS SET role=? WHERE crew_id=?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt,1,newRole.c_str(),-1,nullptr);
    sqlite3_bind_int(stmt,2,crewId);
    bool ok = sqlite3_step(stmt)==SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

bool FleetService::deleteZeroCatch() {
    auto db = Database::instance().handle();
    const char* sql = "DELETE FROM CATCHES WHERE quantity_kg=0;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    bool ok = sqlite3_step(stmt)==SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}

// 5. Начисление премий за внеплановый улов
bool FleetService::awardBonuses(const std::tm& start, const std::tm& end,
                                double plannedKg, double avgPrice)
{
    auto db = Database::instance().handle();
    // Временная таблица plan_summary создаётся один раз
    const char* sql1 =
      "CREATE TEMP TABLE IF NOT EXISTS plan_summary AS "
      "SELECT c.voyage_id, SUM(c.quantity_kg) AS actual_sum "
      "FROM CATCHES c JOIN VOYAGES v ON c.voyage_id=v.voyage_id "
      "WHERE v.departure_date BETWEEN ? AND ? "
      "GROUP BY c.voyage_id;";
    sqlite3_stmt* s1;
    sqlite3_prepare_v2(db, sql1, -1, &s1, nullptr);
    sqlite3_bind_text(s1,1,toSqlDate(start).c_str(),-1,nullptr);
    sqlite3_bind_text(s1,2,toSqlDate(end).c_str(),-1,nullptr);
    sqlite3_step(s1); sqlite3_finalize(s1);

    const char* sql2 =
      "INSERT INTO BONUSES(crew_id,period_start,period_end,amount) "
      "SELECT cm.crew_id, ?, ?, (ps.actual_sum - ?) * ? "
      "FROM CREW_MEMBERS cm "
      "JOIN VOYAGES v ON cm.crew_id=v.trawler_id "  /* Эта связь требует отдельной таблицы, упростим для примера */
      "JOIN plan_summary ps ON v.voyage_id=ps.voyage_id "
      "WHERE ps.actual_sum > ?;";
    sqlite3_stmt* s2;
    sqlite3_prepare_v2(db, sql2, -1, &s2, nullptr);
    sqlite3_bind_text(s2,1,toSqlDate(start).c_str(),-1,nullptr);
    sqlite3_bind_text(s2,2,toSqlDate(end).c_str(),-1,nullptr);
    sqlite3_bind_double(s2,3,plannedKg);
    sqlite3_bind_double(s2,4,avgPrice);
    sqlite3_bind_double(s2,5,plannedKg);
    bool ok = sqlite3_step(s2)==SQLITE_DONE;
    sqlite3_finalize(s2);
    return ok;
}

// 6. Начисление премии конкретному члену
bool FleetService::awardBonusToMember(int crewId, const std::tm& start,
                                      const std::tm& end, double plannedKg, double avgPrice)
{
    auto db = Database::instance().handle();
    const char* sql =
      "INSERT INTO BONUSES(crew_id,period_start,period_end,amount) "
      "SELECT ?, ?, ?, (SUM(c.quantity_kg) - ?) * ? "
      "FROM CATCHES c JOIN VOYAGES v ON c.voyage_id=v.voyage_id "
      "WHERE v.departure_date BETWEEN ? AND ?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt,1,crewId);
    sqlite3_bind_text(stmt,2,toSqlDate(start).c_str(),-1,nullptr);
    sqlite3_bind_text(stmt,3,toSqlDate(end).c_str(),-1,nullptr);
    sqlite3_bind_double(stmt,4,plannedKg);
    sqlite3_bind_double(stmt,5,avgPrice);
    sqlite3_bind_text(stmt,6,toSqlDate(start).c_str(),-1,nullptr);
    sqlite3_bind_text(stmt,7,toSqlDate(end).c_str(),-1,nullptr);
    bool ok = sqlite3_step(stmt)==SQLITE_DONE;
    sqlite3_finalize(stmt);
    return ok;
}
