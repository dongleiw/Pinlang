#include "utils.h"
#include <cstdarg>
#include <llvm-12/llvm/Support/raw_ostream.h>

int str_to_int(const std::string& s) {
	return std::stoi(s.c_str());
}
float str_to_float(const std::string& s) {
	return std::stof(s.c_str());
}
bool str_to_bool(const std::string& s) {
	return s == "true";
}
std::string to_str(int8_t i) {
	char buf[16];
	snprintf(buf, sizeof(buf), "%d", (int32_t)i);
	return std::string(buf);
}
std::string to_str(int16_t i) {
	char buf[16];
	snprintf(buf, sizeof(buf), "%d", (int32_t)i);
	return std::string(buf);
}
std::string to_str(int32_t i) {
	char buf[16];
	snprintf(buf, sizeof(buf), "%d", (int32_t)i);
	return std::string(buf);
}
std::string to_str(int64_t i) {
	char buf[16];
	snprintf(buf, sizeof(buf), "%ld", (int64_t)i);
	return std::string(buf);
}
std::string to_str(uint8_t i) {
	char buf[16];
	snprintf(buf, sizeof(buf), "%u", (uint32_t)i);
	return std::string(buf);
}
std::string to_str(uint16_t i) {
	char buf[16];
	snprintf(buf, sizeof(buf), "%u", (uint32_t)i);
	return std::string(buf);
}
std::string to_str(uint32_t i) {
	char buf[16];
	snprintf(buf, sizeof(buf), "%u", (uint32_t)i);
	return std::string(buf);
}
std::string to_str(uint64_t i) {
	char buf[16];
	snprintf(buf, sizeof(buf), "%lu", (uint64_t)i);
	return std::string(buf);
}
std::string to_str(float f) {
	char buf[32];
	snprintf(buf, sizeof(buf), "%f", (double)f);
	return std::string(buf);
}
std::string to_str(double d) {
	char buf[32];
	snprintf(buf, sizeof(buf), "%f", (double)d);
	return std::string(buf);
}
void replace_str(std::string& str, std::string oldstr, std::string newstr) {
	size_t cur_pos	= 0;
	size_t find_pos = std::string::npos;
	while ((find_pos = str.find(oldstr, cur_pos)) != std::string::npos) {
		str.replace(find_pos, oldstr.size(), newstr);
		cur_pos = find_pos;
	}
}
bool has_suffix(std::string a, std::string suffix) {
	if (a.size() < suffix.size()) {
		return false;
	}
	const char* p1 = a.c_str() + (a.size() - suffix.size());
	const char* p2 = suffix.c_str();
	for (size_t i = 0; i < suffix.size(); i++) {
		if (*p1 != *p2) {
			return false;
		}
		++p1;
		++p2;
	}
	return true;
}
std::vector<std::string> split_to_args(std::string s) {
	bool					 escape = false;
	std::vector<std::string> args;
	std::string				 cur;

	for (size_t i = 0; i < s.size(); i++) {
		char ch = s.at(i);
		switch (ch) {
		case '\\':
			if (escape) {
				cur.push_back(ch);
				escape = false;
			} else {
				escape = true;
			}
			break;
		case '"':
			break;
		case '\'':
			break;
		default:
			break;
		}
	}
}
std::string sprintf_to_stdstr(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	char buf[256];
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	return std::string(buf);
}
template <>
const char* get_type_name<int8_t>() {
	return "int8_t";
}
template <>
const char* get_type_name<int16_t>() {
	return "int16_t";
}
template <>
const char* get_type_name<int32_t>() {
	return "int32_t";
}
template <>
const char* get_type_name<int64_t>() {
	return "int64_t";
}
template <>
const char* get_type_name<uint8_t>() {
	return "uint8_t";
}
template <>
const char* get_type_name<uint16_t>() {
	return "uint16_t";
}
template <>
const char* get_type_name<uint32_t>() {
	return "uint32_t";
}
template <>
const char* get_type_name<uint64_t>() {
	return "uint64_t";
}
template <>
const char* get_type_name<float>() {
	return "float";
}
template <>
const char* get_type_name<double>() {
	return "double";
}
template <>
const char* get_type_name<bool>() {
	return "bool";
}

std::string get_string_of_llvm_ir_type(llvm::Type* ty) {
	std::string				 ret;
	llvm::raw_string_ostream os = llvm::raw_string_ostream(ret);
	ty->print(os);
	return ret;
}
