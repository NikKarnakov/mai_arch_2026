#include "auth.hpp"
#include "models.hpp"
#include <unordered_map>
#include <cstdlib>

static std::unordered_map<std::string, std::string> tokenStore;

namespace Auth {

std::string Login(const std::string& login, const std::string& password) {
    User* u = UserStore::Instance().FindByLogin(login);
    if (u && u->password == password) {
        std::string token = login + "_tok_" + std::to_string(rand() % 1000);
        tokenStore[token] = login;
        return token;
    }
    return "";
}

bool CheckToken(const std::string& token) {
    if (token.rfind("Bearer ", 0) != 0) return false;
    std::string actual = token.substr(7);
    return tokenStore.find(actual) != tokenStore.end();
}
}
