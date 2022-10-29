#pragma once

#include <string>

int str_to_int(const std::string &s);
float str_to_float(const std::string &s);
bool str_to_bool(const std::string &s);

std::string int_to_str(int8_t i);
std::string int_to_str(int16_t i);
std::string int_to_str(int32_t i);
std::string int_to_str(int64_t i);
std::string int_to_str(uint8_t i);
std::string int_to_str(uint16_t i);
std::string int_to_str(uint32_t i);
std::string int_to_str(uint64_t i);

void replace_str(std::string& str, std::string oldstr, std::string newstr);
