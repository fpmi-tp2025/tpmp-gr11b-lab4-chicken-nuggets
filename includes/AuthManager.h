//
// Created by pravl on 30.04.2025.
//

#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include "Models.h"
#include <optional>

class AuthManager {
public:
    bool login(const std::string& username, const std::string& password);
    std::optional<CrewMember> currentUser();
    bool isManager() const;
private:
    CrewMember user_;
};

#endif //AUTHMANAGER_H
