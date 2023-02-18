#pragma once

#include <string>
#include <vector>

int	  str_to_int(const std::string& s);
float str_to_float(const std::string& s);
bool  str_to_bool(const std::string& s);

std::string to_str(int8_t i);
std::string to_str(int16_t i);
std::string to_str(int32_t i);
std::string to_str(int64_t i);
std::string to_str(uint8_t i);
std::string to_str(uint16_t i);
std::string to_str(uint32_t i);
std::string to_str(uint64_t i);
std::string to_str(float f);
std::string to_str(double d);

bool		has_suffix(std::string a, std::string suffix);
void		replace_str(std::string& str, std::string oldstr, std::string newstr);
std::string sprintf_to_stdstr(const char* fmt, ...);

/*
 * 将字符串分割为多个参数
 * a 'b c\\' "c\""  =>
 *		a
 *		b c\
 *		c"
 */
std::vector<std::string> split_to_args(std::string s);

template <typename T>
const char* get_type_name();
template <>
const char* get_type_name<int8_t>();
template <>
const char* get_type_name<int16_t>();
template <>
const char* get_type_name<int32_t>();
template <>
const char* get_type_name<int64_t>();
template <>
const char* get_type_name<uint8_t>();
template <>
const char* get_type_name<uint16_t>();
template <>
const char* get_type_name<uint32_t>();
template <>
const char* get_type_name<uint64_t>();
template <>
const char* get_type_name<float>();
template <>
const char* get_type_name<double>();
template <>
const char* get_type_name<bool>();
