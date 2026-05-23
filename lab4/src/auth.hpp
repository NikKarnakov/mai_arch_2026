#pragma once

#include <optional>
#include <string>

namespace hotel::auth {

std::string HashPassword(const std::string& password);
bool CheckPassword(const std::string& password, const std::string& password_hash);

std::string MakeToken(long user_id, const std::string& login);
std::optional<long> ExtractUserId(const std::string& authorization_header);

} 
