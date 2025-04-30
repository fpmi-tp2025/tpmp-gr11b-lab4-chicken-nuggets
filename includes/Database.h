//
// Created by pravl on 30.04.2025.
//

#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <mutex>
#include <string>

class Database {
public:
    static Database& instance();
    bool open(const std::string& filename);
    void close();
    sqlite3* handle() const;

private:
    Database();
    ~Database();
    sqlite3* db_;
    std::mutex mtx_;
};

#endif //DATABASE_H
