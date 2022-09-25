#pragma once

#include <string>

int str_to_int(const std::string &s);
float str_to_float(const std::string &s);
bool str_to_bool(const std::string &s);

std::string int_to_str(int i);

void replace_str(std::string& str, std::string oldstr, std::string newstr);
