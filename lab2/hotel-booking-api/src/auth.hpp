#pragma once
#include <string>

namespace Auth {
    std::string Login(const std::string& login, const std::string& password);
    bool CheckToken(const std::string& token);
}
