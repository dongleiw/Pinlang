#pragma once

#include <string>

int str_to_int(const std::string &s);
float str_to_float(const std::string &s);
bool str_to_bool(const std::string &s);

void replace_str(std::string& str, std::string oldstr, std::string newstr);
