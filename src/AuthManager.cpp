//
// Created by pravl on 30.04.2025.
//

#include "AuthManager.h"
#include "Database.h"
#include <sqlite3.h>
#include <iostream>

bool AuthManager::login(const std::string& username, const std::string& password) {
    // Здесь простой пример: ищем в таблице USERS
    sqlite3_stmt* stmt;
    const char* sql =
            "SELECT crew_id, last_name, role, hire_date, birth_year, isManager, password "
            "FROM CREW_MEMBERS WHERE last_name = ? AND password = ?;";

    sqlite3_prepare_v2(Database::instance().handle(), sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, nullptr);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, nullptr);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user_.id         = sqlite3_column_int(stmt, 0);
        user_.lastName   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user_.role       = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        // Парсинг dates пропущен для краткости...
        user_.birthYear  = sqlite3_column_int(stmt, 4);
        user_.isManager  = sqlite3_column_int(stmt, 5) != 0;
        user_.password   = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        sqlite3_finalize(stmt);
        return true;
    }
    sqlite3_finalize(stmt);
    return false;
}

std::optional<CrewMember> AuthManager::currentUser() {
    if (user_.id != 0) return user_;
    return std::nullopt;
}

bool AuthManager::isManager() const {
    return user_.isManager;
}
