//
// Created by pravl on 30.04.2025.
//

#include "Database.h"
#include <stdexcept>

Database::Database(): db_(nullptr) {}
Database::~Database() { close(); }

Database& Database::instance() {
    static Database inst;
    return inst;
}

bool Database::open(const std::string& filename) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (sqlite3_open(filename.c_str(), &db_) != SQLITE_OK) {
        return false;
    }
    return true;
}

void Database::close() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (db_) { sqlite3_close(db_); db_ = nullptr; }
}

sqlite3* Database::handle() const {
    return db_;
}
